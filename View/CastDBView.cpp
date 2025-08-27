#include "CastDBView.h"

#include "CastDBActions.h"
#include "FdBasedDb.h"
#include "FileFolderPreviewer.h"
#include "JsonHelper.h"
#include "MemoryKey.h"
#include "NotificatorMacro.h"
#include "PathTool.h"
#include "PerformerJsonFileHelper.h"
#include "PublicMacro.h"
#include "PublicVariable.h"
#include "StringTool.h"
#include "TableFields.h"
#include "QuickWhereClauseHelper.h"

#include <QDesktopServices>
#include <QDirIterator>
#include <QDockWidget>
#include <QHeaderView>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QFileDialog>

CastDBView::CastDBView(CastDbModel* castDbModel_,
                       CastDatabaseSearchToolBar* castDbSearchBar_,
                       FileFolderPreviewer* floatingPreview_,
                       CastBaseDb& castDb_,
                       QWidget* parent)
  : CustomTableView{"PERFORMERS_TABLE", parent},  //
  _castDbSearchBar{castDbSearchBar_},
  _castModel{castDbModel_},
  _floatingPreview{floatingPreview_},
  _castDb{castDb_},
  mImageHost{castDbModel_->rootPath()}
{
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
  setWindowTitle("Cast Manager Widget");
  setWindowIcon(QIcon(":img/CAST_VIEW"));
}

bool CastDBView::onOpenRecordInFileSystem() const {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("Nothing was selected.", "Select a row to open pson folder");
    return false;
  }
  const QModelIndex ind{currentIndex()};
  // when column in detail. open pson file. Otherwise open folder contains pson
  QString revealPath{currentIndex().column() == PERFORMER_DB_HEADER_KEY::Detail ? _castModel->psonFilePath(ind): _castModel->filePath(ind)};
  if (!QFile::exists(revealPath)) {
    LOG_WARN_NP("Path not exists", revealPath);
    return false;
  }
  return QDesktopServices::openUrl(QUrl::fromLocalFile(revealPath));
}


void CastDBView::subscribe() {
  connect(_castDbSearchBar, &CastDatabaseSearchToolBar::whereClauseChanged, _castModel, &QSqlTableModel::setFilter);

  auto& castInst = g_castAct();
  connect(castInst.SUBMIT, &QAction::triggered, this, &CastDBView::onSubmit);
  connect(castInst.APPEND_FROM_MULTILINES_INPUT, &QAction::triggered, this, &CastDBView::onAppendCasts);
  connect(castInst.DELETE_RECORDS, &QAction::triggered, this, &CastDBView::onDeleteRecords);
  connect(castInst.INIT_DATABASE, &QAction::triggered, &_castDb, &DbManager::CreateDatabase);
  connect(castInst.INIT_TABLE, &QAction::triggered, this, &CastDBView::onInitATable);
  connect(castInst.DROP_TABLE, &QAction::triggered, this, [this]() { onDropDeleteTable(DbManager::DROP_OR_DELETE::DROP); });
  connect(castInst.DELETE_TABLE, &QAction::triggered, this, [this]() { onDropDeleteTable(DbManager::DROP_OR_DELETE::DELETE); });

  connect(castInst.SYNC_SELECTED_RECORDS_IMGS_FROM_DISK, &QAction::triggered, this, &CastDBView::onSyncImgsFieldFromImageHost);
  connect(castInst.SYNC_ALL_RECORDS_IMGS_FROM_DISK, &QAction::triggered, this, &CastDBView::onSyncAllImgsFieldFromImageHost);
  connect(castInst.SYNC_SELECTED_RECORDS_VIDS_FROM_DB, &QAction::triggered, this, &CastDBView::onForceRefreshRecordsVids);
  connect(castInst.SYNC_ALL_RECORDS_VIDS_FROM_DB, &QAction::triggered, this, &CastDBView::onForceRefreshAllRecordsVids);

  connect(castInst.OPEN_DB_WITH_LOCAL_APP, &QAction::triggered, &_castDb, &DbManager::ShowInFileSystemView);
  connect(castInst.OPEN_RECORD_IN_FILE_SYSTEM, &QAction::triggered, this, &CastDBView::onOpenRecordInFileSystem);

  connect(castInst.APPEND_FROM_FILE_SYSTEM_STRUCTURE, &QAction::triggered, this, &CastDBView::onLoadFromFileSystemStructure);
  connect(castInst.APPEND_FROM_PSON_FILES, &QAction::triggered, this, &CastDBView::onLoadFromPsonDirectory);

  connect(castInst.DUMP_ALL_RECORDS_INTO_PSON_FILE, &QAction::triggered, this, &CastDBView::onDumpAllIntoPsonFile);
  connect(castInst.DUMP_SELECTED_RECORDS_INTO_PSON_FILE, &QAction::triggered, this, &CastDBView::onDumpIntoPsonFile);

  connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &CastDBView::on_selectionChanged);
}

QString CastDBView::filePath(const QModelIndex& index) const {
  if (_castModel == nullptr || !index.isValid()){
    return "";
  }
  const auto& record = _castModel->record(currentIndex().row());
  return CastBaseDb::GetCastFilePath(record, mImageHost);
}

void CastDBView::onInitATable() {
  // UTF-8 each char takes 1 to 4 byte, 256 chars means 256~1024 bytes
  if (!_castDb.CreateTable(DB_TABLE::PERFORMERS, CastBaseDb::CREATE_PERF_TABLE_TEMPLATE)) {
    qWarning("Table[%s] create failed.", qPrintable(DB_TABLE::PERFORMERS));
    return;
  }
  _castModel->setTable(DB_TABLE::PERFORMERS);
  _castModel->submitAll();
  qDebug("Table[%s] create succeed", qPrintable(DB_TABLE::PERFORMERS));
}

int CastDBView::onAppendCasts() {
  if (_castModel->isDirty()) {
    LOG_BAD_NP("Table dirty", "submit before load from file-system structure");
    return false;
  }
  bool ok = false;
  const QString& perfsText =                             //
      QInputDialog::getMultiLineText(this,               //
                                     "Input 'Casts, aka'",  //
                                     "Example:\n Guardiola, Pep\nHuge Jackman, Wolverine", "", &ok);
  if (!ok) {
    LOG_INFO_NP("[skip] User cancel", "return");
    return 0;
  }
  int succeedCnt = _castDb.AppendCastFromMultiLineInput(perfsText);
  if (succeedCnt < 0) {
    LOG_WARN_P("Load perfs from text failed", "perfsText:%s, errorCode: %d", qPrintable(perfsText), succeedCnt);
    return 0;
  }
  _castModel->submitAll();
  LOG_GOOD_P("[Ok] load performer(s)", "perfsText:%s, count: %d", qPrintable(perfsText), succeedCnt);
  return succeedCnt;
}

bool CastDBView::onDropDeleteTable(const DbManager::DROP_OR_DELETE dropOrDelete) {
  auto retBtn = QMessageBox::warning(this,                                                                          //
                                     QString("Confirm %1?").arg((int)dropOrDelete),                                 //
                                     "Drop(0)/Delete(1) [" + DB_TABLE::PERFORMERS + "] operation not recoverable",  //
                                     QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
  if (retBtn != QMessageBox::StandardButton::Yes) {
    qDebug("User cancel drop/delete table[%s]", qPrintable(DB_TABLE::PERFORMERS));
    return true;
  }
  int rmvedTableCnt = _castDb.RmvTable(DB_TABLE::PERFORMERS, dropOrDelete);
  if (rmvedTableCnt < 0) {
    LOG_BAD_P("Drop/Delete failed", "errorCode: %d", rmvedTableCnt);
    return false;
  }
  _castModel->submitAll();
  const QString title = QString("Operation: %1 on [%2]").arg((int)dropOrDelete).arg(DB_TABLE::PERFORMERS);
  const QString msg = QString("Drop(0)/Delete(1). %1 table removed").arg(rmvedTableCnt);
  LOG_GOOD_NP(title, msg);
  return rmvedTableCnt >= 0;
}

int CastDBView::onLoadFromFileSystemStructure() {
  if (_castModel->isDirty()) {
    LOG_BAD_NP("[Abort] Table dirty", "submit before load from file-system structure");
    return false;
  }
  int succeedCnt = _castDb.ReadFromImageHost(mImageHost);
  if (succeedCnt < 0) {
    LOG_BAD_P("[Failed] Load perfs", "errorCode: %d", succeedCnt)
    return 0;
  }
  _castModel->submitAll();
  LOG_GOOD_P("Load perf(s) succeed", "count: %d", succeedCnt)
  return succeedCnt;
}

bool CastDBView::onSubmit() {
  CHECK_NULLPTR_RETURN_FALSE(_castModel)

  if (!_castModel->isDirty()) {
    LOG_GOOD_NP("[Skip submit] Table not dirty", DB_TABLE::PERFORMERS);
    return true;
  }
  if (!_castModel->submitAll()) {
    LOG_WARN_NP("Submit failed", _castModel->lastError().text());
    return false;
  }
  LOG_GOOD_NP("Submit succeed. Following .db has been saved", DB_TABLE::PERFORMERS);
  return true;
}

bool CastDBView::onRevert() {
  if (!_castModel->isDirty()) {
    LOG_GOOD_NP("Table not dirty.", "Skip revert");
    return true;
  }
  _castModel->revertAll();
  LOG_GOOD_NP("Revert succeed", "All changes revert");
  return true;
}

bool CastDBView::on_selectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/) {
  if (!currentIndex().isValid()) {
    return true;
  }
  CHECK_NULLPTR_RETURN_FALSE(_floatingPreview);
  const auto& record = _castModel->record(currentIndex().row());
  _floatingPreview->operator()(record, mImageHost);
  return true;
}

int CastDBView::onLoadFromPsonDirectory() {
  if (_castModel->isDirty()) {
    LOG_BAD_NP("Table dirty", "submit before load pson");
    return 0;
  }
  int succeedCnt = _castDb.LoadFromPsonFile(mImageHost);
  if (succeedCnt < 0) {
    LOG_WARN_P("[Failed] Load perfs from pJson", "[%s/*.pson] errorCode:%d", qPrintable(mImageHost), succeedCnt);
    return succeedCnt;
  }
  _castModel->submitAll();
  LOG_GOOD_P("[Ok] pson file(s) load succeed", "count: %d", succeedCnt);
  return succeedCnt;
}

int CastDBView::onSyncAllImgsFieldFromImageHost() {
  const int totalCnt{_castModel->rowCount()};
  if (totalCnt == 0) {
    LOG_INFO_NP("[skip] No records at all skip", "No need sync");
    return 0;
  }
  int succeedCnt = 0;
  for (int r = 0; r < _castModel->rowCount(); ++r) {
    QSqlRecord sqlRecord = _castModel->record(r);
    succeedCnt += CastBaseDb::UpdateRecordImgsField(sqlRecord, mImageHost);
    _castModel->setRecord(r, sqlRecord);
  }
  RefreshHtmlContents();
  QString msgTitle{QString("All %1 record(s) imgs field been sync").arg(totalCnt)};
  QString msgDetail{QString("%1/%2 succeed").arg(succeedCnt).arg(totalCnt)};
  if (totalCnt != succeedCnt) {
    LOG_WARN_NP(msgTitle, msgDetail);
  } else {
    LOG_GOOD_NP(msgTitle, msgDetail);
  }
  return succeedCnt;
}

int CastDBView::onSyncImgsFieldFromImageHost() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("Nothing was selected", "Select some row to sync imgs fields");
    return 0;
  }
  currentIndex();

  const int totalCnt{selectionModel()->selectedRows().size()};
  int succeedCnt = 0;
  for (const auto& indr : selectionModel()->selectedRows()) {
    const int r = indr.row();
    QSqlRecord sqlRecord = _castModel->record(r);
    succeedCnt += CastBaseDb::UpdateRecordImgsField(sqlRecord, mImageHost);
    _castModel->setRecord(r, sqlRecord);
  }
  RefreshHtmlContents();
  QString msgTitle{QString("%1 record(s) selected imgs field been sync").arg(totalCnt)};
  QString msgDetail{QString("%1/%2 succeed").arg(succeedCnt).arg(totalCnt)};
  if (totalCnt != succeedCnt) {
    LOG_WARN_NP(msgTitle, msgDetail);
  } else {
    LOG_GOOD_NP(msgTitle, msgDetail);
  }
  return succeedCnt;
}

int CastDBView::onDumpAllIntoPsonFile() {
  const int totalCnt{_castModel->rowCount()};
  if (totalCnt == 0) {
    LOG_INFO_NP("[Skip] No records at all skip", "No need dump");
    return 0;
  }
  int succeedCnt = 0;
  for (int r = 0; r < _castModel->rowCount(); ++r) {
    const auto& pson = PerformerJsonFileHelper::PerformerJsonJoiner(_castModel->record(r));
    const QString& psonPath = PerformerJsonFileHelper::PsonPath(mImageHost, pson);
    succeedCnt += JsonHelper::DumpJsonDict(pson, psonPath);
  }
  QString msgTitle{QString("All %1 record(s) dumped result").arg(totalCnt)};
  QString msgDetail{QString("%1/%2 succeed").arg(succeedCnt).arg(totalCnt)};
  if (totalCnt != succeedCnt) {
    LOG_WARN_NP(msgTitle, msgDetail);
  } else {
    LOG_GOOD_NP(msgTitle, msgDetail);
  }
  return succeedCnt;
}

int CastDBView::onDumpIntoPsonFile() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("Nothing was selected", "Select some row to dump");
    return 0;
  }

  QDir imageHostDir{mImageHost};
  int totalCnt {selectionModel()->selectedRows().size()};
  int succeedCnt = 0;
  for (const auto& indr : selectionModel()->selectedRows()) {
    const int r = indr.row();
    const auto& record = _castModel->record(r);
    const QString ori {record.value(PERFORMER_DB_HEADER_KEY::Ori).toString()};
    const QString castName {record.value(PERFORMER_DB_HEADER_KEY::Name).toString()};
    const QString prepath {ori + '/' + castName};
    if (!imageHostDir.exists(prepath) && !imageHostDir.mkpath(prepath)) {
      qWarning("Create folder [%s] under [%s] failed", qPrintable(prepath), qPrintable(mImageHost));
      continue;
    }
    const QString psonPath {PerformerJsonFileHelper::PsonPath(mImageHost, ori, castName)};
    const QVariantHash pson = PerformerJsonFileHelper::PerformerJsonJoiner(record);
    succeedCnt += JsonHelper::DumpJsonDict(pson, psonPath);
  }

  QString msgTitle{QString("Selected %1 record(s) dumped result").arg(totalCnt)};
  QString msgDetail{QString("%1/%2 succeed").arg(succeedCnt).arg(totalCnt)};
  if (totalCnt != succeedCnt) {
    LOG_WARN_NP(msgTitle, msgDetail);
  } else {
    LOG_GOOD_NP(msgTitle, msgDetail);
  }
  return succeedCnt;
}

int CastDBView::onForceRefreshAllRecordsVids() {
  selectAll();
  LOG_INFO_NP("Refresh all records may lag", "Click Refresh Selected action if you are sure");
  return 0;
}

int CastDBView::onForceRefreshRecordsVids() {
  if (!selectionModel()->hasSelection()) {
    LOG_BAD_NP("Nothing was selected", "Select some row to refresh");
    return 0;
  }

  FdBasedDb movieDb{SystemPath::VIDS_DATABASE, "SEARCH_MOVIE_BY_PERFORMER"};
  QSqlDatabase con = movieDb.GetDb();  // videos table
  if (!movieDb.CheckValidAndOpen(con)) {
    qWarning("Open failed:%s", qPrintable(con.lastError().text()));
    return -1;
  }
  QSqlQuery qur{con};
  int recordsCnt = 0;
  int vidsCnt = 0;
  for (const auto& indr : selectionModel()->selectedRows()) {
    const int r = indr.row();
    QSqlRecord record = _castModel->record(r);
    const QString& perfs {record.field(PERFORMER_DB_HEADER_KEY::Name).value().toString()};
    const QString& akas {record.field(PERFORMER_DB_HEADER_KEY::AKA).value().toString()};
    const QString& selectStr {QuickWhereClauseHelper::GetSelectMovieByCastStatement(perfs, akas, DB_TABLE::MOVIES)};

    if (!qur.exec(selectStr)) {
      qWarning("Query[%s] failed: %s", qPrintable(qur.executedQuery()), qPrintable(qur.lastError().text()));
      return -1;
    }

    int curCastVidCnt{0};
    QString vidPaths;
    while (qur.next()) {
      vidPaths += QuickWhereClauseHelper::GetMovieFullPathFromSqlQry(qur);
      vidPaths += StringTool::PERFS_VIDS_IMGS_SPLIT_CHAR;
      ++curCastVidCnt;
    }
    if (!vidPaths.isEmpty()) { // remove suffix \n
      vidPaths.chop(1);
    }
    qDebug("cast[%s] %d records finded", qPrintable(perfs), curCastVidCnt);

    record.setValue(PERFORMER_DB_HEADER_KEY::Vids, vidPaths);
    _castModel->setRecord(r, record);  // update back

    vidsCnt += curCastVidCnt;
    ++recordsCnt;
  }
  RefreshHtmlContents();
  LOG_GOOD_P("[ok]Videos(s) updated", "%d records selection, total %d videos", recordsCnt, vidsCnt);
  return recordsCnt;
}

int CastDBView::onDeleteRecords() {
  if (!selectionModel()->hasSelection()) {
    LOG_BAD_NP("Nothing was selected", "Select some row(s) to delete");
    return 0;
  }
  int deleteCnt = 0;
  int succeedCnt = 0;
  const auto& itemSelection = selectionModel()->selection();
  for (auto it = itemSelection.crbegin(); it != itemSelection.crend(); ++it) {
    int startRow = it->top();  // [top, bottom]
    int size = it->bottom() - startRow + 1;
    bool ret = _castModel->removeRows(startRow, size);
    qDebug("drop[%d] records [%d, %d]", ret, startRow, it->bottom());
    deleteCnt += size;
    succeedCnt += ((int)ret * size);
  }
  LOG_GOOD_P("[Ok]Delete records(need submit)", "%d/%d succeed", succeedCnt, deleteCnt);
  return succeedCnt;
}

void CastDBView::RefreshHtmlContents() {
  if (_floatingPreview == nullptr || !selectionModel()->hasSelection()) {
    return;
  }
  const auto& record = _castModel->record(currentIndex().row());
  _floatingPreview->operator()(record, mImageHost);
}


// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>
int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  CastDBView perfManaWid;
  perfManaWid.show();
  a.exec();
  return 0;
}
#endif
