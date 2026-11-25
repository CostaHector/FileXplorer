#include "CastDBTableView.h"
#include "CastDbModel.h"
#include "CastDBActions.h"
#include "FdBasedDb.h"
#include "NotificatorMacro.h"
#include "PublicCheckMacro.h"
#include "PublicVariable.h"
#include "InputDialogHelper.h"
#include "SystemPath.h"
#include "CurrentIndexGuard.h"

#include <QItemSelectionModel>
#include <QSqlError>
#include <QSqlRecord>

struct RecordChangedAutoEmiter {
  explicit RecordChangedAutoEmiter(CastDBTableView& view_, CastDbModel& castDbModel_) : _view{view_}, _model{castDbModel_} {
    mCurInd = _view.currentIndex();
    if (mCurInd.isValid()) {
      m_beforeRecord = _model.record(mCurInd.row());
    }
  }

  ~RecordChangedAutoEmiter() {
    if (!mEnabled) {
      return;
    }
    if (!mCurInd.isValid()) {
      return;
    }
    QSqlRecord newRecord = _model.record(mCurInd.row());
    if (newRecord == m_beforeRecord) {
      return;
    }
    _view.EmitCurrentCastRecordChangedCore(newRecord);
  }

  void setEnabled(bool bEnabled) {
    mEnabled = bEnabled;
  }

  bool mEnabled{true};
  QSqlRecord m_beforeRecord;
  CastDBTableView& _view;
  CastDbModel& _model;
  QModelIndex mCurInd;
};


int CastDBTableView::QUERY_CONFIRM_IF_ROW_SELECTED_COUNT_ABOVE = 100;

CastDBTableView::CastDBTableView(CastDbModel* castDbModel_, QItemSelectionModel* selectionModel_, QWidget* parent)
  : CustomTableView{"ACTORS_TABLE", parent},  //
  _castModel{castDbModel_}
{
  CHECK_NULLPTR_RETURN_VOID(castDbModel_);
  CHECK_NULLPTR_RETURN_VOID(selectionModel_);
  const QString imageHostFullPath = castDbModel_->rootPath();
  if (!QFileInfo{imageHostFullPath}.isDir()) {
    LOG_WARN_P("ImageHostPath not exist", "Fix it[%s] in .ini file", qPrintable(imageHostFullPath));
    return;
  }

  {
    auto& inst = CastDBActions::GetInst();
    QList<QAction*> exclusiveActs {
        inst.SYNC_SELECTED_RECORDS_VIDS_FROM_DB,   //
        inst.SYNC_SELECTED_RECORDS_IMGS_FROM_DISK, //
        inst.DUMP_SELECTED_RECORDS_INTO_PSON_FILE, //
    };
    PushFrontExclusiveActions(exclusiveActs);
  }

  setModel(_castModel);
  setSelectionModel(selectionModel_);
  InitTableView();

  subscribe();
  setWindowTitle("Cast View");
  setWindowIcon(QIcon(":img/CAST_VIEW"));
}

void CastDBTableView::subscribe() {
  CastDBActions& castInst = CastDBActions::GetInst();
  connect(_castModel, &SqlTableModelPub::dirtyStatesChanged, castInst._MODEL_PENDING_CHANGES_ACTION, &PendingChangesAction::setDirty);
  connect(castInst._MODEL_PENDING_CHANGES_ACTION->mSubmitAct, &QAction::triggered, this, &CastDBTableView::onSubmitAll);
  connect(castInst._MODEL_PENDING_CHANGES_ACTION->mRevertAct, &QAction::triggered, this, &CastDBTableView::onRevertAll);

  connect(castInst.INSERT_A_ROW_BEFORE_CURRENT_INDEX, &QAction::triggered, this, &CastDBTableView::onInsertARowBeforeCurrentIndex);
  connect(castInst.DELETE_RECORDS, &QAction::triggered, this, &CastDBTableView::onDeleteRecords);

  connect(castInst.SYNC_SELECTED_RECORDS_IMGS_FROM_DISK, &QAction::triggered, this, &CastDBTableView::onRefreshImgsField);
  connect(castInst.SYNC_ALL_RECORDS_IMGS_FROM_DISK, &QAction::triggered, this, &CastDBTableView::onRefreshAllImgsField);
  connect(castInst.SYNC_SELECTED_RECORDS_VIDS_FROM_DB, &QAction::triggered, this, &CastDBTableView::onRefreshVidsField);
  connect(castInst.SYNC_ALL_RECORDS_VIDS_FROM_DB, &QAction::triggered, this, &CastDBTableView::onRefreshAllVidsField);

  connect(castInst.DUMP_ALL_RECORDS_INTO_PSON_FILE, &QAction::triggered, this, &CastDBTableView::onDumpAllIntoPsonFile);
  connect(castInst.DUMP_SELECTED_RECORDS_INTO_PSON_FILE, &QAction::triggered, this, &CastDBTableView::onDumpIntoPsonFile);

  connect(selectionModel(), &QItemSelectionModel::currentRowChanged, this, &CastDBTableView::EmitCurrentCastRecordChanged);
}

// returnValue: insert At
int CastDBTableView::onInsertARowBeforeCurrentIndex() {
  const QModelIndex curInd = currentIndex();
  const int currentRow = curInd.isValid() ? curInd.row() : 0; // default: insert at first line
  const bool bResult = _castModel->insertRow(currentRow);
  LOG_OE_P(bResult, "Insert a row", "before line: %d", currentRow);
  return currentRow;
}

int CastDBTableView::onDeleteRecords() {
  const auto& itemSelection = selectionModel()->selection();
  if (itemSelection.isEmpty()) {
    LOG_INFO_NP("Nothing was selected", "Select some row(s) to delete");
    return 0;
  }
  const QString cfmTitleText{"Confirm Delete selection rows? (OPERATION NOT RECOVERABLE)"};
  const QString hintText {QString::asprintf("Risk: %d ranges are about to removed! (Attention: Not recoverable)", itemSelection.size())};
  if (!InputDialogHelper::YesOrCancelBox(QMessageBox::Icon::Warning, QIcon{}, cfmTitleText, hintText, "", "")) {
    LOG_INFO_NP("[Skip] User Cancel delete records", "return");
    return 0;
  }
  int succeedCnt = _castModel->DeleteSelectionRange(itemSelection);
  LOG_OE_NP(succeedCnt >= 0, "Delete records:", QString::number(succeedCnt));
  return succeedCnt;
}

bool CastDBTableView::onRevertAll() {
  CurrentIndexGuard guard{*this};
  const bool revertRet = _castModel->revertAllChanges();
  guard.setEnabled(revertRet);
  LOG_OE_P(revertRet, "Model Revert All", "Table: %s", qPrintable(DB_TABLE::PERFORMERS));
  return revertRet;
}

bool CastDBTableView::onSubmitAll() {
  CurrentIndexGuard guard{*this};
  const bool submitRet = _castModel->submitSaveAllChanges();
  guard.setEnabled(submitRet);
  LOG_OE_P(submitRet, "Model Submit All", "Table: %s", qPrintable(DB_TABLE::PERFORMERS));
  return submitRet;
}

int CastDBTableView::onRefreshAllImgsField() {
  QModelIndexList allIndexes = _castModel->GetAllRowsIndexes();
  return onRefreshImgsFieldCore(allIndexes);
}

int CastDBTableView::onRefreshImgsField() {
  const QModelIndexList selectedRowsIndexes = selectedRowsSource();
  return onRefreshImgsFieldCore(selectedRowsIndexes);
}

int CastDBTableView::onRefreshImgsFieldCoreStatic(CastDbModel& model, const QModelIndexList& selectedRowsIndexes) {
  return model.RefreshImgsFieldValue(selectedRowsIndexes);
}

int CastDBTableView::onRefreshImgsFieldCore(const QModelIndexList& selectedRowsIndexes) {
  const int totalCnt{selectedRowsIndexes.size()};
  if (totalCnt == 0) {
    LOG_INFO_NP("No record need Sync", "Empty table or nothing row was selected");
    return 0;
  }
  RecordChangedAutoEmiter emiter{*this, *_castModel};
  const int succeedCnt = onRefreshImgsFieldCoreStatic(*_castModel, selectedRowsIndexes);
  emiter.setEnabled(succeedCnt > 0);
  LOG_OE_P(succeedCnt >= 0, "Image fields sync", "%d/%d updated", succeedCnt, totalCnt);
  return succeedCnt;
}

int CastDBTableView::onRefreshAllVidsField() {
  QModelIndexList allIndexes = _castModel->GetAllRowsIndexes();
  return onRefreshVidsFieldCore(allIndexes);
}

int CastDBTableView::onRefreshVidsField() {
  const QModelIndexList selectedRowsIndexes = selectedRowsSource();
  return onRefreshVidsFieldCore(selectedRowsIndexes);
}

int CastDBTableView::onRefreshVidsFieldCoreStatic(CastDbModel& model, const QModelIndexList& selectedRowsIndexes) {
  QString movieDbFileAbsPath{SystemPath::VIDS_DATABASE()};
  QString movieDbConnName{"SEARCH_MOVIE_BY_PERFORMER"};
  FdBasedDb movieDb{movieDbFileAbsPath, movieDbConnName};
  return model.RefreshVidsFieldValue(selectedRowsIndexes, movieDb.GetDb());
}

int CastDBTableView::onRefreshVidsFieldCore(const QModelIndexList& selectedRowsIndexes) {
  const int totalCnt{selectedRowsIndexes.size()};
  if (totalCnt == 0) {
    LOG_INFO_NP("No record need refresh", "Empty table or nothing row was selected");
    return 0;
  }

  const QString cfmTitleText{"CONFIRM refresh `Vids` Field of the selected %d rows?"};
  const QString hintText{QString::asprintf("%d rows selected(>%d). May lag", totalCnt, QUERY_CONFIRM_IF_ROW_SELECTED_COUNT_ABOVE)};
  if (totalCnt > QUERY_CONFIRM_IF_ROW_SELECTED_COUNT_ABOVE && !InputDialogHelper::YesOrCancelBox(QMessageBox::Icon::Warning, QIcon{}, cfmTitleText, hintText, "", "")) {
    LOG_INFO_NP("[Skip] User cancel refresh vids fields", "too many selection may cause lag");
    return 0;
  }

  RecordChangedAutoEmiter emiter{*this, *_castModel};
  const int succeedCnt = onRefreshVidsFieldCoreStatic(*_castModel, selectedRowsIndexes);
  emiter.setEnabled(succeedCnt > 0);
  LOG_OE_P(succeedCnt >= 0, "Force refresh `Vids` Field", "%d/%d succeed", succeedCnt, totalCnt);
  return succeedCnt;
}

int CastDBTableView::onDumpAllIntoPsonFile() {
  QModelIndexList allIndexes = _castModel->GetAllRowsIndexes();
  return onDumpIntoCore(allIndexes);
}

int CastDBTableView::onDumpIntoPsonFile() {
  const QModelIndexList selectedRowsIndexes = selectedRowsSource();
  return onDumpIntoCore(selectedRowsIndexes);
}

int CastDBTableView::onDumpIntoCore(const QModelIndexList& selectedRowsIndexes) {
  const int totalCnt{selectedRowsIndexes.size()};
  if (totalCnt == 0) {
    LOG_INFO_NP("No record need dump", "Empty table or nothing row was selected");
    return 0;
  }
  int succeedCnt = _castModel->DumpRecordsIntoPsonFile(selectedRowsIndexes);
  LOG_OE_P(succeedCnt >= 0, "Dump records into pson", "%d succeed", succeedCnt);
  return succeedCnt;
}

bool CastDBTableView::EmitCurrentCastRecordChanged(const QModelIndex& current, const QModelIndex& /*previous*/) {
  if (!current.isValid()) {
    return false;
  }
  const QSqlRecord& record = _castModel->record(current.row());
  return EmitCurrentCastRecordChangedCore(record);
}

bool CastDBTableView::EmitCurrentCastRecordChangedCore(const QSqlRecord& record) {
  QString imageHostFullPath = _castModel->rootPath();
  emit currentRecordChanged(record, imageHostFullPath);
  return true;
}

void CastDBTableView::RefreshCurrentRowHtmlContents() {
  auto current = currentIndex();
  if (!current.isValid()) {
    return;
  }
  const QSqlRecord& record = _castModel->record(current.row());
  QString imageHostFullPath = _castModel->rootPath();
  emit currentRecordChanged(record, imageHostFullPath);
}

