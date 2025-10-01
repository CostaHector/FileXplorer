#include "CastDBView.h"

#include "CastDBActions.h"
#include "FdBasedDb.h"
#include "JsonHelper.h"
#include "NotificatorMacro.h"
#include "CastPsonFileHelper.h"
#include "PublicMacro.h"
#include "PublicVariable.h"
#include "StringTool.h"
#include "TableFields.h"
#include "QuickWhereClauseHelper.h"

#include <QDesktopServices>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QFileDialog>

int CastDBView::QUERY_CONFIRM_IF_ROW_SELECTED_COUNT_ABOVE = 100;
void CastDBView::setQueryConfirmIfRowSelectedCountAbove(int newValue) {
  QUERY_CONFIRM_IF_ROW_SELECTED_COUNT_ABOVE = newValue;
}

CastDBView::CastDBView(CastDbModel* castDbModel_, CastDatabaseSearchToolBar* castDbSearchBar_, CastBaseDb& castDb_, QWidget* parent)
    : CustomTableView{"PERFORMERS_TABLE", parent},  //
      _castDbSearchBar{castDbSearchBar_},
      _castModel{castDbModel_},
      _castDb{castDb_},
      mImageHost{castDbModel_->rootPath()} {
  if (!QFileInfo{mImageHost}.isDir()) {
    QString titleMsg{QString{"ImageHostPath[%1] not exist"}.arg(mImageHost)};
    LOG_CRIT_NP(titleMsg, mImageHost);
    QMessageBox::critical(this, titleMsg, "Path not exist. Fix it in .ini file at first");
    return;
  }

  CHECK_NULLPTR_RETURN_VOID(_castDbSearchBar);
  CHECK_NULLPTR_RETURN_VOID(_castModel);

  BindMenu(g_castAct().GetRightClickMenu(this));
  setModel(_castModel);
  InitTableView();

  subscribe();
  setWindowTitle("Cast View");
  setWindowIcon(QIcon(":img/CAST_VIEW"));
}

void CastDBView::subscribe() {
  connect(_castDbSearchBar, &CastDatabaseSearchToolBar::whereClauseChanged, _castModel, &QSqlTableModel::setFilter);

  static auto& castInst = g_castAct();
  connect(castInst._MODEL_SUBMIT_ALL, &QAction::triggered, this, &CastDBView::onModelSubmitAll);
  connect(castInst._MODEL_REPOPULATE, &QAction::triggered, this, &CastDBView::onModelRepopulate);
  connect(castInst.APPEND_FROM_MULTILINES_INPUT, &QAction::triggered, this, &CastDBView::onAppendCasts);
  connect(castInst.DELETE_RECORDS, &QAction::triggered, this, &CastDBView::onDeleteRecords);
  connect(castInst.INIT_DATABASE, &QAction::triggered, &_castDb, &DbManager::CreateDatabase);
  connect(castInst.INIT_TABLE, &QAction::triggered, this, &CastDBView::onInitATable);
  connect(castInst.DROP_TABLE, &QAction::triggered, this, [this]() { onDropDeleteTable(DbManagerHelper::DropOrDeleteE::DROP); });
  connect(castInst.DELETE_TABLE, &QAction::triggered, this, [this]() { onDropDeleteTable(DbManagerHelper::DropOrDeleteE::DELETE); });

  connect(castInst.SYNC_SELECTED_RECORDS_IMGS_FROM_DISK, &QAction::triggered, this, &CastDBView::onSyncImgsFieldFromImageHost);
  connect(castInst.SYNC_ALL_RECORDS_IMGS_FROM_DISK, &QAction::triggered, this, &CastDBView::onSyncAllImgsFieldFromImageHost);
  connect(castInst.SYNC_SELECTED_RECORDS_VIDS_FROM_DB, &QAction::triggered, this, &CastDBView::onRefreshVidsField);
  connect(castInst.SYNC_ALL_RECORDS_VIDS_FROM_DB, &QAction::triggered, this, &CastDBView::onRefreshAllVidsField);

  connect(castInst.OPEN_DB_WITH_LOCAL_APP, &QAction::triggered, &_castDb, &DbManager::onShowInFileSystemView);
  connect(castInst.MIGRATE_CAST_TO, &QAction::triggered, this, &CastDBView::onMigrateCastTo);

  connect(castInst.APPEND_FROM_FILE_SYSTEM_STRUCTURE, &QAction::triggered, this, &CastDBView::onLoadFromFileSystemStructure);
  connect(castInst.APPEND_FROM_PSON_FILES, &QAction::triggered, this, &CastDBView::onLoadFromPsonDirectory);

  connect(castInst.DUMP_ALL_RECORDS_INTO_PSON_FILE, &QAction::triggered, this, &CastDBView::onDumpAllIntoPsonFile);
  connect(castInst.DUMP_SELECTED_RECORDS_INTO_PSON_FILE, &QAction::triggered, this, &CastDBView::onDumpIntoPsonFile);

  connect(selectionModel(), &QItemSelectionModel::currentRowChanged, this, &CastDBView::EmitCurrentCastRecordChanged);
}

void CastDBView::onInitATable() {
  // UTF-8 each char takes 1 to 4 byte, 256 chars means 256~1024 bytes
  if (!_castDb.CreateTable(DB_TABLE::PERFORMERS, CastBaseDb::CREATE_PERF_TABLE_TEMPLATE)) {
    LOG_W("Table[%s] create failed.", qPrintable(DB_TABLE::PERFORMERS));
    return;
  }
  _castModel->setTable(DB_TABLE::PERFORMERS);
  _castModel->select();
  LOG_D("Table[%s] create succeed", qPrintable(DB_TABLE::PERFORMERS));
}

int CastDBView::onAppendCasts() {
  if (_castModel->isDirty()) {
    LOG_ERR_NP("Table dirty", "submit before load from file-system structure");
    return false;
  }
  const QString exampleText = "Example:\n Guardiola, Pep\nHuge Jackman, Wolverine";
  bool ok = false;
  QString perfsText;
#ifdef RUNNING_UNIT_TESTS
  std::tie(ok, perfsText) = CastDbViewMocker::MockMultiLineInput();
#else
  perfsText = QInputDialog::getMultiLineText(this, "Input 'Casts, aka'", exampleText, "", &ok);
#endif
  if (!ok) {
    LOG_INFO_NP("[skip] User cancel", "return");
    return 0;
  }
  int succeedCnt = _castDb.AppendCastFromMultiLineInput(perfsText);
  if (succeedCnt < 0) {
    LOG_WARN_P("Load perfs from text failed", "perfsText:%s, errorCode: %d", qPrintable(perfsText), succeedCnt);
    return 0;
  }
  onModelRepopulate();
  LOG_OK_P("[Ok] load performer(s)", "perfsText:%s, count: %d", qPrintable(perfsText), succeedCnt);
  return succeedCnt;
}

int CastDBView::onDeleteRecords() {
  const auto& itemSelection = selectionModel()->selection();
  if (itemSelection.isEmpty()) {
    LOG_INFO_NP("Nothing was selected", "Select some row(s) to delete");
    return 0;
  }
  const QString cfmTitleText{"Confirm Delete selection rows? (OPERATION NOT RECOVERABLE)"};
  const QString hintText = QString::asprintf("Risk: %d ranges are about to removed! (Attention: Not recoverable)", itemSelection.size());
  QMessageBox::StandardButton stdCfmDeleteBtn = QMessageBox::StandardButton::No;
#ifdef RUNNING_UNIT_TESTS
  stdCfmDeleteBtn = CastDbViewMocker::MockDeleteRecord() ? QMessageBox::Yes : QMessageBox::No;
#else
  stdCfmDeleteBtn = QMessageBox::question(this, cfmTitleText, hintText, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
#endif
  if (stdCfmDeleteBtn != QMessageBox::Yes) {
    LOG_INFO_NP("[Skip] User Cancel delete records", "return");
    return 0;
  }
  int succeedCnt = _castModel->DeleteSelectionRange(itemSelection);
  LOG_OE_NP(succeedCnt >= 0, "Delete records:", QString::number(succeedCnt));
  return succeedCnt;
}

bool CastDBView::onDropDeleteTable(const DbManagerHelper::DropOrDeleteE dropOrDelete) {
  QMessageBox::StandardButton stdCfmDeleteBtn = QMessageBox::StandardButton::No;
  const QString cfmTitleText{QString::asprintf("Confirm %s?", DbManagerHelper::c_str(dropOrDelete))};
  const QString hintText{QString::asprintf("Operation[%s] on Table[%s] is not recoverable",  //
                                           DbManagerHelper::c_str(dropOrDelete), qPrintable(DB_TABLE::PERFORMERS))};
#ifdef RUNNING_UNIT_TESTS
  stdCfmDeleteBtn = CastDbViewMocker::MockDropDeleteTable() ? QMessageBox::StandardButton::Yes : QMessageBox::StandardButton::No;
#else
  stdCfmDeleteBtn = QMessageBox::warning(this,                                                                          //
                                         cfmTitleText,                                                                  //
                                         "Drop(0)/Delete(1) [" + DB_TABLE::PERFORMERS + "] operation not recoverable",  //
                                         QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
#endif
  if (stdCfmDeleteBtn != QMessageBox::StandardButton::Yes) {
    LOG_D("User cancel drop/delete table[%s]", qPrintable(DB_TABLE::PERFORMERS));
    return true;
  }
  int rmvedTableCnt = _castDb.RmvTable(DB_TABLE::PERFORMERS, dropOrDelete);
  LOG_OE_P(rmvedTableCnt >= 0, "Drop/Delete", "Table[%s] %s count:%d", qPrintable(DB_TABLE::PERFORMERS), DbManagerHelper::c_str(dropOrDelete),
           rmvedTableCnt);
  onModelRepopulate();
  return rmvedTableCnt >= 0;
}

int CastDBView::onLoadFromFileSystemStructure() {
  if (_castModel->isDirty()) {
    LOG_ERR_NP("[Abort] Table dirty", "submit before load from file-system structure");
    return 0;
  }
  int succeedCnt = _castDb.ReadFromImageHost(mImageHost);
  LOG_OE_P(succeedCnt >= 0, "Load Performers", "count: %d", succeedCnt);
  if (succeedCnt >= 0) {
    onModelRepopulate();
  }
  return succeedCnt;
}

bool CastDBView::onModelRepopulate() {
  const QModelIndex oldIndex = currentIndex();
  bool bRepopulateRet = _castModel->repopulate();
  if (oldIndex.isValid() && currentIndex() != oldIndex) {
    setCurrentIndex(oldIndex);
  }
  LOG_OE_P(bRepopulateRet, "Repopulate", "Table: %s", qPrintable(DB_TABLE::PERFORMERS));
  return bRepopulateRet;
}

bool CastDBView::onModelSubmitAll() {
  bool submitRet = _castModel->submitSaveAllChanges();
  LOG_OE_P(submitRet, "Model submit", "Table: %s", qPrintable(DB_TABLE::PERFORMERS));
  return submitRet;
}

int CastDBView::onLoadFromPsonDirectory() {
  if (_castModel->isDirty()) {
    LOG_ERR_NP("Table dirty", "submit before load pson");
    return 0;
  }

  QMessageBox::StandardButton stdCfmLoadFromPson = QMessageBox::StandardButton::No;
  const QString cfmTitleText{"CONFIRM Load from pson? (OVERRIDE NOT RECOVERABLE)"};
  const QString hintText{"Risk: records in database will be override if differs from local pson file."};
#ifdef RUNNING_UNIT_TESTS
  stdCfmLoadFromPson = CastDbViewMocker::MockLoadFromPsonDirectory() ? QMessageBox::StandardButton::Yes : QMessageBox::StandardButton::No;
#else
  stdCfmLoadFromPson = QMessageBox::question(this, cfmTitleText, hintText, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
#endif
  if (stdCfmLoadFromPson != QMessageBox::Yes) {
    LOG_INFO_NP("[Skip] User cancel load records from pson", "return");
    return 0;
  }
  int succeedCnt = _castDb.LoadFromPsonFile(mImageHost);
  LOG_OE_P(succeedCnt >= 0, "Load perfs from pJson", "[%s/*.pson] count:%d", qPrintable(mImageHost), succeedCnt);
  onModelRepopulate();
  return succeedCnt;
}

int CastDBView::onSyncAllImgsFieldFromImageHost() {
  QModelIndexList allIndexes = _castModel->GetAllRowsIndexes();
  return onSyncImgsFieldCore(allIndexes);
}

int CastDBView::onSyncImgsFieldFromImageHost() {
  const QModelIndexList& selectedRowsIndexes = selectionModel()->selectedRows();
  return onSyncImgsFieldCore(selectedRowsIndexes);
}

int CastDBView::onSyncImgsFieldCore(const QModelIndexList& selectedRowsIndexes) {
  const int totalCnt{selectedRowsIndexes.size()};
  if (totalCnt == 0) {
    LOG_INFO_NP("No record need Sync", "Empty table or nothing row was selected");
    return 0;
  }
  const QModelIndex oldIndex = currentIndex();
  int succeedCnt = _castModel->SyncImageFieldsFromImageHost(selectedRowsIndexes);
  LOG_OE_P(succeedCnt >= 0, "Image fields sync", "%d/%d updated", succeedCnt, totalCnt);
  if (oldIndex.isValid()) {
    currentIndex() != oldIndex ? setCurrentIndex(oldIndex) : RefreshCurrentRowHtmlContents();
  }
  return succeedCnt;
}

int CastDBView::onDumpAllIntoPsonFile() {
  QModelIndexList allIndexes = _castModel->GetAllRowsIndexes();
  return onDumpIntoCore(allIndexes);
}

int CastDBView::onDumpIntoPsonFile() {
  const QModelIndexList& selectedRowsIndexes = selectionModel()->selectedRows();
  return onDumpIntoCore(selectedRowsIndexes);
}

int CastDBView::onDumpIntoCore(const QModelIndexList& selectedRowsIndexes) {
  const int totalCnt{selectedRowsIndexes.size()};
  if (totalCnt == 0) {
    LOG_INFO_NP("No record need dump", "Empty table or nothing row was selected");
    return 0;
  }
  int succeedCnt = _castModel->DumpRecordsIntoPsonFile(selectedRowsIndexes);
  LOG_OE_P(succeedCnt >= 0, "Dump records into pson", "%d succeed", succeedCnt);
  return succeedCnt;
}

int CastDBView::onRefreshAllVidsField() {
  QModelIndexList allIndexes = _castModel->GetAllRowsIndexes();
  return onRefreshVidsFieldCore(allIndexes);
}

int CastDBView::onRefreshVidsField() {
  const QModelIndexList& selectedRowsIndexes = selectionModel()->selectedRows();
  return onRefreshVidsFieldCore(selectedRowsIndexes);
}

int CastDBView::onRefreshVidsFieldCore(const QModelIndexList& selectedRowsIndexes) {
  const int totalCnt{selectedRowsIndexes.size()};
  if (totalCnt == 0) {
    LOG_INFO_NP("No record need refresh", "Empty table or nothing row was selected");
    return 0;
  }

  if (totalCnt > QUERY_CONFIRM_IF_ROW_SELECTED_COUNT_ABOVE) {
    const QString cfmTitleText{"CONFIRM refresh `Vids` Field of the selected %d rows?"};
    const QString hintText{QString::asprintf("%d rows selected(>%d). May lag", totalCnt, QUERY_CONFIRM_IF_ROW_SELECTED_COUNT_ABOVE)};
    QMessageBox::StandardButton stdCfmRefreshVidFields = QMessageBox::StandardButton::No;
#ifdef RUNNING_UNIT_TESTS
    stdCfmRefreshVidFields = CastDbViewMocker::MockRefreshVidsField() ? QMessageBox::StandardButton::Yes : QMessageBox::StandardButton::No;
#else
    stdCfmRefreshVidFields = QMessageBox::question(this, cfmTitleText, hintText, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
#endif
    if (stdCfmRefreshVidFields != QMessageBox::StandardButton::Yes) {
      LOG_INFO_NP("[Skip] User cancel load records from pson", "return");
      return 0;
    }
  }

  QString movieDbFileAbsPath = SystemPath::VIDS_DATABASE();
  QString movieDbConnName = "SEARCH_MOVIE_BY_PERFORMER";
#ifdef RUNNING_UNIT_TESTS
  std::tie(movieDbFileAbsPath, movieDbConnName) = CastDbViewMocker::MockMovieDbAbsFilePath2ConnName();
#endif
  FdBasedDb movieDb{movieDbFileAbsPath, movieDbConnName};

  const QModelIndex oldIndex = currentIndex();
  int succeedCnt = _castModel->RefreshVidsForRecords(selectedRowsIndexes, movieDb.GetDb());
  LOG_OE_P(succeedCnt >= 0, "Force refresh `VidName` Field", "%d/%d succeed", succeedCnt, totalCnt);
  if (oldIndex.isValid()) {
    currentIndex() != oldIndex ? setCurrentIndex(oldIndex) : RefreshCurrentRowHtmlContents();
  }
  return succeedCnt;
}

void CastDBView::EmitCurrentCastRecordChanged(const QModelIndex& current, const QModelIndex& /*previous*/) {
  if (!current.isValid()) {
    return;
  }
  const auto& record = _castModel->record(current.row());
  emit currentRecordChanged(record, mImageHost);
}

int CastDBView::onMigrateCastTo() {
  const QModelIndexList& selectedRowIndexes = selectionModel()->selectedRows();
  if (selectedRowIndexes.isEmpty()) {
    LOG_INFO_NP("Nothing was selected.", "Select at least one row before migrate");
    return 0;
  }
  const QString cfmTitleText{"Migrate to (folder under[" + mImageHost + "])"};
  QString destPath;
#ifdef RUNNING_UNIT_TESTS:
  destPath = CastDbViewMocker::MockMigrateToPath();
#else
  destPath = QFileDialog::getExistingDirectory(this, cfmTitleText, mImageHost);
#endif
  if (destPath.isEmpty()) {
    LOG_OK_NP("[Skip] User cancel migrate", "return");
    return 0;
  }
  int migrateCastCnt = _castModel->MigrateCastsTo(selectedRowIndexes, destPath);
  LOG_OE_P(migrateCastCnt >= 0, "Migrate cast", "%d casts from to %s", selectedRowIndexes.size(), qPrintable(destPath));
  return migrateCastCnt;
}

void CastDBView::RefreshCurrentRowHtmlContents() {
  auto current = currentIndex();
  if (!current.isValid()) {
    return;
  }
  const auto& record = _castModel->record(current.row());
  emit currentRecordChanged(record, mImageHost);
}
