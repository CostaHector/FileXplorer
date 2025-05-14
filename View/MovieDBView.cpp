#include "MovieDBView.h"

#include "Actions/MovieDBActions.h"
#include "Actions/FileBasicOperationsActions.h"

#include "Component/QuickWhereClause.h"
#include "Component/Notificator.h"

#include "Tools/FileDescriptor/MountHelper.h"
#include "Tools/StudiosManager.h"
#include "public/PublicMacro.h"
#include "public/PublicVariable.h"
#include "public/MemoryKey.h"

#include <QHeaderView>

#include <QSqlError>
#include <QSqlQuery>

#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>

MovieDBView::MovieDBView(FdBasedDbModel* model_,               //
                         DatabaseSearchToolBar* _dbSearchBar,  //
                         FdBasedDb& movieDb_,
                         QWidget* parent)      //
    : CustomTableView("MOVIE_TABLE", parent),  //
      mDb{movieDb_} {
  CHECK_NULLPTR_RETURN_VOID(model_);
  _dbModel = model_;
  CHECK_NULLPTR_RETURN_VOID(_dbSearchBar);
  _tablesDropDownList = _dbSearchBar->m_tables;
  CHECK_NULLPTR_RETURN_VOID(_tablesDropDownList);
  _searchWhereLineEdit = _dbSearchBar->m_searchLE;
  CHECK_NULLPTR_RETURN_VOID(_searchWhereLineEdit);

  m_movieMenu = new MovieDatabaseMenu{"Movie Right click menu", this};
  m_quickWhereClause = new QuickWhereClause{this};
  BindMenu(m_movieMenu);

  setModel(_dbModel);

  setEditTriggers(QAbstractItemView::EditKeyPressed);  // only F2 works.

  InitMoviesTables();
  const QString defaultTableName = PreferenceSettings().value(MemoryKey::VIDS_LAST_TABLE_NAME.name, MemoryKey::VIDS_LAST_TABLE_NAME.v).toString();
  const int defaultDisplayIndex = _tablesDropDownList->findText(defaultTableName, Qt::MatchStartsWith);
  if (defaultDisplayIndex != -1) {
    _tablesDropDownList->setCurrentIndex(defaultDisplayIndex);
  }
  setCurrentMovieTable(_tablesDropDownList->currentText());

  subscribe();
  InitTableView();
}

void MovieDBView::subscribe() {
  connect(horizontalHeader(), &QHeaderView::sectionResized, this, [this]() {                               //
    PreferenceSettings().setValue("DATABASE_TABLEVIEW_HERDER_GEOMETRY", horizontalHeader()->saveState());  //
  });
  connect(_searchWhereLineEdit, &QLineEdit::returnPressed, this, &MovieDBView::onSearchDataBase);
  connect(_tablesDropDownList, &QComboBox::currentTextChanged, this, &MovieDBView::setCurrentMovieTable);

  auto& inst = g_dbAct();
  // control actions
  connect(inst.SUBMIT, &QAction::triggered, this, &MovieDBView::onSubmit);
  connect(inst.REVERT, &QAction::triggered, this, &MovieDBView::onRevert);
  connect(inst.INIT_A_DATABASE, &QAction::triggered, this, &MovieDBView::onInitDataBase);
  connect(inst.INIT_A_TABLE, &QAction::triggered, this, &MovieDBView::onCreateATable);
  connect(inst.DROP_A_TABLE, &QAction::triggered, this, &MovieDBView::onDropATable);
  connect(inst.INSERT_A_PATH, &QAction::triggered, this, &MovieDBView::onInsertIntoTable);
  connect(inst.DELETE_FROM_TABLE, &QAction::triggered, this, &MovieDBView::onDeleteFromTable);
  connect(inst.UNION_TABLE, &QAction::triggered, this, &MovieDBView::onUnionTables);
  connect(inst.AUDIT_A_TABLE, &QAction::triggered, this, &MovieDBView::onAuditATable);
  // extra function actions
  connect(inst.READ_DURATION_BY_VIDEO, &QAction::triggered, this, &MovieDBView::onSetDurationByVideo);
  connect(inst.EXPORT_DURATION_STUDIO_CAST_TAGS_TO_JSON, &QAction::triggered, this, &MovieDBView::onExportToJson);
  connect(inst.UPDATE_STUDIO_CAST_TAGS_BY_JSON, &QAction::triggered, this, &MovieDBView::onUpdateByJson);
  // assistance actions
  connect(inst.QUICK_WHERE_CLAUSE, &QAction::triggered, this, &MovieDBView::onQuickWhereClause);
  // common function actions
  connect(inst._COUNT, &QAction::triggered, this, &MovieDBView::onCountRow);
  connect(inst._OPEN_DB_WITH_LOCAL_APP, &QAction::triggered, &mDb, &DbManager::ShowInFileSystemView);
  // record studio/cast/tags manual batch edit
  connect(inst.SET_STUDIO, &QAction::triggered, this, &MovieDBView::onSetStudio);
  connect(inst.SET_CAST, &QAction::triggered, this, [this]() { onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::SET); });
  connect(inst.APPEND_CAST, &QAction::triggered, this, [this]() { onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::APPEND); });
  connect(inst.REMOVE_CAST, &QAction::triggered, this, [this]() { onSetCastOrTags(FIELD_OP_TYPE::CAST, FIELD_OP_MODE::REMOVE); });
  connect(inst.SET_TAGS, &QAction::triggered, this, [this]() { onSetCastOrTags(FIELD_OP_TYPE::TAGS, FIELD_OP_MODE::SET); });
  connect(inst.APPEND_TAGS, &QAction::triggered, this, [this]() { onSetCastOrTags(FIELD_OP_TYPE::TAGS, FIELD_OP_MODE::APPEND); });
  connect(inst.REMOVE_TAGS, &QAction::triggered, this, [this]() { onSetCastOrTags(FIELD_OP_TYPE::TAGS, FIELD_OP_MODE::REMOVE); });

  addAction(g_fileBasicOperationsActions().COPY_FULL_PATH);
  addAction(g_fileBasicOperationsActions().COPY_NAME);
  addAction(g_fileBasicOperationsActions().COPY_RECORDS);
}

bool MovieDBView::onSearchDataBase() {
  const QString& searchPattern = _searchWhereLineEdit->text();
  _dbModel->setFilter(searchPattern);
  _dbModel->select();
  if (_dbModel->lastError().isValid()) {
    LOG_BAD("FAIL set new searchPattern", _dbModel->lastError().text());
    return false;
  }
  LOG_GOOD("Succeed set new searchPattern", searchPattern);
  return true;
}

bool MovieDBView::InitMoviesTables() {
  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    qWarning("Opened db failed:%s", qPrintable(con.lastError().text()));
    return false;
  }
  CHECK_NULLPTR_RETURN_FALSE(_tablesDropDownList);
  _tablesDropDownList->clear();
  const QStringList& tbls = con.tables();
  const auto& guidTblName2Disp = MountHelper::GetGuidTableName2DisplayName();
  for (const QString& tableName : tbls) {  // in underscore
    _tablesDropDownList->AddItem(tableName, guidTblName2Disp.value(tableName, "displace name NOT FOUND"));
  }
  qDebug("Tables count:%d", tbls.size());
  return true;
}

bool MovieDBView::setCurrentMovieTable(const QString& guidJoinRootPath) {
  qDebug("Set Current GuidJoinRooPath[%s]", qPrintable(guidJoinRootPath));
  _tablesDropDownList->setCurrentText(guidJoinRootPath);
  const QString& movieTableName = GetMovieTableName();
  _dbModel->setTable(movieTableName);
  PreferenceSettings().setValue(MemoryKey::VIDS_LAST_TABLE_NAME.name, movieTableName);
  //  m_dbModel->select();
  _dbModel->submitAll();
  InitTableView();
  return true;
}

bool MovieDBView::GetAPathFromUserSelect(const QString& usageMsg, QString& userSelected) {
  const QString& curTblName = GetMovieTableName();
  const QString& tblPeerPath = GetMovieTableRootPath();
  if (!QFileInfo{tblPeerPath}.isDir()) {
    const QString offLineMsg = QString{"[Skip] Table[%1] Peer Path[%2] not exist(Disk Maybe Offline)"}.arg(curTblName).arg(tblPeerPath);
    return false;
  }
  QString lastPath = PreferenceSettings().value(MemoryKey::PATH_DB_INSERT_VIDS_FROM.name, MemoryKey::PATH_DB_INSERT_VIDS_FROM.v).toString();
  if (!QFileInfo(lastPath).isDir()) {  // fallback
    lastPath = tblPeerPath;
  }
  const QString& caption{QString{"Choose a path[under %1] %2 table[%3]"}.arg(tblPeerPath).arg(usageMsg).arg(curTblName)};
  QString selectPath = QFileDialog::getExistingDirectory(this,      //
                                                         caption,   //
                                                         lastPath,  //
                                                         QFileDialog::ShowDirsOnly);
  if (selectPath.isEmpty()) {
    LOG_WARN("User cancel insert, path is not directory", selectPath);
    return false;
  }
  if (!selectPath.startsWith(tblPeerPath)) {
    const QString pthRelationMsg{selectPath + '\n' + tblPeerPath};
    LOG_WARN("Path user selected is not under table PeerPath", pthRelationMsg);
    return false;
  }

  PreferenceSettings().setValue(MemoryKey::PATH_DB_INSERT_VIDS_FROM.name, selectPath);
  userSelected.swap(selectPath);
  qDebug("[%s] User selectPath[%s] under PeerPath[%s]", qPrintable(usageMsg), qPrintable(userSelected), qPrintable(tblPeerPath));
  return true;
}

bool MovieDBView::onInsertIntoTable() {
  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    LOG_BAD("Open failed", con.lastError().text());
    return false;
  }

  const QString& curTblName = GetMovieTableName();
  if (!con.tables().contains(curTblName)) {
    LOG_BAD("[ABORT] Table NOT exist.", curTblName);
    return false;
  }

  QString selectPath;
  if (!GetAPathFromUserSelect("and load videos into", selectPath)) {
    return false;
  }

  const QString msg{QString{"%1/* ----->---- Table: %2"}.arg(selectPath).arg(_tablesDropDownList->currentText())};
  if (QMessageBox::question(this, "CONFIRM INSERT INTO?", msg) != QMessageBox::StandardButton::Yes) {
    LOG_GOOD("User cancel insert", selectPath);
    return true;
  }

  int retCnt = mDb.ReadADirectory(curTblName, selectPath);
  if (retCnt < 0) {
    LOG_BAD("Read videos from path failed, see details", selectPath);
    return false;
  }

  const QString cntMsg{QString("Count=%1").arg(retCnt)};
  LOG_GOOD("Read videos from path succeed", cntMsg);
  QMessageBox::information(this, "Read videos from path succeed", cntMsg);
  _dbModel->submitAll();
  return true;
}

bool MovieDBView::onSubmit() {
  if (_dbModel == nullptr) {
    qCritical("_dbModel is nullptr");
    return false;
  }
  if (_tablesDropDownList == nullptr) {
    qCritical("_tablesDropDownList is nullptr");
    return false;
  }
  if (!_dbModel->isDirty()) {
    LOG_GOOD("Table not dirty. Skip submit", _tablesDropDownList->currentText());
    return true;
  }
  if (!_dbModel->submitAll()) {
    LOG_BAD("Submit failed. see details in logs", _tablesDropDownList->currentText());
    return false;
  }
  LOG_GOOD("Submit succeed", _tablesDropDownList->currentText());
  return true;
}

bool MovieDBView::onRevert() {
  if (_dbModel == nullptr) {
    qCritical("_dbModel is nullptr");
    return false;
  }
  if (_tablesDropDownList == nullptr) {
    qCritical("_tablesDropDownList is nullptr");
    return false;
  }
  if (!_dbModel->isDirty()) {
    LOG_GOOD("Table not dirty. Skip revert", _tablesDropDownList->currentText());
    return true;
  }
  _dbModel->revertAll();
  LOG_GOOD("Revert succeed", _tablesDropDownList->currentText());
  return true;
}

bool MovieDBView::onInitDataBase() {
  const bool crtResult = mDb.CreateDatabase();
  if (!crtResult) {
    LOG_BAD("Init database failed, reason:", mDb.GetCfgDebug());
  }
  LOG_GOOD("Init database succeed", "...");
  return true;
}

void MovieDBView::onCreateATable() {
  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    qWarning("Open failed:%s", qPrintable(con.lastError().text()));
    return;
  }

  bool isInputOk{false};
  const QStringList& tables = con.tables();
  const QStringList& candidates = MountHelper::GetGuidJoinDisplayName();
  const QString& msgs = QString("current %1 table names occupied as following:\n%2").arg(candidates.size()).arg(candidates.join('\n'));
  const QString& crtTbl = QInputDialog::getItem(this, "Input an table name", msgs, candidates, 0,  //
                                                true, &isInputOk);
  if (!isInputOk) {
    LOG_GOOD("[skip] User cancel create table", "")
    return;
  }
  const QString& newTbl = MountHelper::ChoppedDisplayName(crtTbl);
  if (newTbl.isEmpty() || tables.contains(newTbl)) {
    LOG_WARN("[Abort] Table name empty or already occupied", newTbl)
    return;
  }

  if (newTbl.contains(JSON_RENAME_REGEX::INVALID_TABLE_NAME_LETTER)) {
    LOG_BAD("[Abort] Table name contains invalid letter.", newTbl)
    return;
  }

  if (!mDb.CreateTable(newTbl, FdBasedDb::CREATE_TABLE_TEMPLATE)) {
    LOG_BAD("[Abort] Table name created failed. See detail in log", newTbl)
    return;
  }

  _tablesDropDownList->AddItem(newTbl, MountHelper::GetDisplayNameByGuidTableName(newTbl));
  _tablesDropDownList->setCurrentIndex(_tablesDropDownList->count() - 1);
  LOG_GOOD("[OK] Table created succeed", newTbl)
}

bool MovieDBView::onDropATable() {
  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    QString lastErrMsg{con.lastError().text()};
    LOG_BAD("[Abort] Open db failed, See detail in log", lastErrMsg)
    return false;
  }
  const QStringList& tables = con.tables();
  if (tables.isEmpty()) {
    LOG_GOOD("[Skip] No need drop", "No table exist at all");
    return true;
  }
  const QStringList& candidates = _tablesDropDownList->ToQStringList();
  const int defaultDropIndex = _tablesDropDownList->currentIndex();
  const QString& msgs = QString("There are %1 table(s) as following:\n%2").arg(candidates.size()).arg(candidates.join('\n'));

  bool okUserSelect = false;
  const QString& drpTbl = QInputDialog::getItem(this, "CONFIRM DROP? (NOT RECOVERABLE)",  //
                                                msgs,                                     //
                                                candidates,                               //
                                                defaultDropIndex,                         //
                                                false,                                    //
                                                &okUserSelect);
  if (!okUserSelect) {
    LOG_GOOD("[skip] User cancel drop table", "")
    return true;
  }
  const QString& deleteTbl = MountHelper::ChoppedDisplayName(drpTbl);
  if (deleteTbl.isEmpty()) {
    LOG_BAD("[Abort] Table name is empty, cannot drop", deleteTbl)
    return true;
  }

  if (!mDb.DropTable(deleteTbl)) {
    QString lastErrMsg{con.lastError().text()};
    LOG_BAD("[Abort] Table drop failed, See detail in log", lastErrMsg)
    return false;
  }
  InitMoviesTables();
  LOG_GOOD("[OK] Table has been dropped", deleteTbl)
  return true;
}

bool MovieDBView::onDeleteFromTable() {
  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    QString lastErrMsg{con.lastError().text()};
    LOG_BAD("[Abort] Open db failed, See detail in log", lastErrMsg)
    return false;
  }

  using namespace MOVIE_TABLE;
  const QString& tbl = GetMovieTableName();
  static const QString RELATION_TEMPLATE{R"("%1" = )"};
  static const QStringList candidates{
      "",
      RELATION_TEMPLATE.arg(ENUM_TO_STRING(PrePathLeft)),   //
      RELATION_TEMPLATE.arg(ENUM_TO_STRING(PrePathRight)),  //
      RELATION_TEMPLATE.arg(ENUM_TO_STRING(Name)),          //
      RELATION_TEMPLATE.arg(ENUM_TO_STRING(Size)),          //
      RELATION_TEMPLATE.arg(ENUM_TO_STRING(Duration)),      //
      RELATION_TEMPLATE.arg(ENUM_TO_STRING(Cast)),          //
      RELATION_TEMPLATE.arg(ENUM_TO_STRING(Tags))           //
  };

  bool okClicked = false;
  const QString& whereClause = QInputDialog::getItem(this, "DELETE WHERE clause",                //
                                                     QString{"DELETE FROM %1 WHERE "}.arg(tbl),  //
                                                     candidates, 0, true, &okClicked);
  if (!okClicked) {
    LOG_GOOD("[skip] User cancel delete row", "")
    return true;
  }
  if (whereClause.isEmpty()) {
    qWarning("All record(s) in table[%s] to be deleted.", qPrintable(tbl));
  }
  qDebug("Where clause[%s]", qPrintable(whereClause));
  const QString deleteCmd{QString{"DELETE FROM %1\nWHERE [%2];"}.arg(tbl, whereClause)};
  if (QMessageBox::question(this, "CONFIRM DELETE? (OPERATION NOT RECOVERABLE)", deleteCmd,  //
                            QMessageBox::Yes | QMessageBox::No, QMessageBox::No)             //
      != QMessageBox::Yes) {
    LOG_GOOD("User Cancel delete records", "Skip");
    return true;
  }
  const int affectedRows = mDb.DeleteByWhereClause(tbl, whereClause);
  if (affectedRows < 0) {
    LOG_BAD("Delete failed", deleteCmd);
    return false;
  }
  QString affectedRowsMsg = QString{"%1 record(s) affected"}.arg(affectedRows);
  LOG_GOOD("[OK]" + deleteCmd, affectedRowsMsg);
  _dbModel->submitAll();
  return true;
}

bool MovieDBView::onQuickWhereClause() {
  auto retCode = m_quickWhereClause->exec();
  if (retCode != QDialog::DialogCode::Accepted) {
    return false;
  }
  const QString& where = m_quickWhereClause->GetWhereString();
  qDebug("Quick where clause: [%s]", qPrintable(where));
  _searchWhereLineEdit->setText(where);
  emit _searchWhereLineEdit->returnPressed();
  return true;
}

bool MovieDBView::onUnionTables() {
  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    QString lastErrMsg{con.lastError().text()};
    LOG_BAD("[Abort] Open db failed, See detail in log", lastErrMsg)
    return false;
  }

  const QStringList& tbs = con.tables();
  if (!tbs.contains(DB_TABLE::MOVIES)) {
    QMessageBox::warning(this, "Destination table not exist. Create it at first", DB_TABLE::MOVIES);
    return false;
  }

  const int SRC_TABLE_CNT = tbs.size() - 1;
  if (SRC_TABLE_CNT <= 1) {
    LOG_INFO("No need union", "Only one table find(except destination table)");
    return true;
  }

  const QString& confirmUnionHintMsg = QString("All %1 tables into Table[%2]").arg(SRC_TABLE_CNT).arg(DB_TABLE::MOVIES);
  if (QMessageBox::question(this, "Confirm Union?", confirmUnionHintMsg, QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) {
    LOG_GOOD("[Skip] User cancel union tables", "cancel");
    return true;
  }

  QStringList unionSrcTbs;
  unionSrcTbs.reserve(SRC_TABLE_CNT);
  for (const QString& srcTable : tbs) {
    if (srcTable == DB_TABLE::MOVIES) {
      continue;
    }
    unionSrcTbs << QString("SELECT * FROM `%1`").arg(srcTable);
  }

  // REPLACE INTO `MOVIES` SELECT * FROM `A568` UNION SELECT * FROM `AASAD`;
  const QString& unionStr = unionSrcTbs.join(" UNION ");
  const QString& unionCmd = QString("REPLACE INTO `%1` %2").arg(DB_TABLE::MOVIES).arg(unionStr);
  QSqlQuery unionTableQry{con};
  if (!unionTableQry.exec(unionCmd)) {
    const QString& title = QString("Union %1 table(s) into [%2]").arg(SRC_TABLE_CNT).arg(DB_TABLE::MOVIES);
    const QString& msg = QString("cmd: %1 failed: %2").arg(unionTableQry.executedQuery()).arg(unionTableQry.lastError().text());
    LOG_GOOD(title, msg);
    QMessageBox::warning(this, title, msg);
    con.rollback();
    return false;
  }
  unionTableQry.finish();
  QMessageBox::information(this, DB_TABLE::MOVIES, "Union into succeed");
  return true;
}

bool MovieDBView::onAuditATable() {
  if (_dbModel->isDirty()) {
    LOG_WARN("[Skip Audit] Table is dirty. must submit/revert first", _tablesDropDownList->currentText());
    return true;
  }

  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    QString lastErrMsg{con.lastError().text()};
    LOG_BAD("[Abort] Open db failed, See detail in log", lastErrMsg)
    return false;
  }
  const QString& curTblName = GetMovieTableName();
  const QStringList& tbs = con.tables();
  if (!tbs.contains(curTblName)) {
    LOG_WARN("Skip audit now. Table not exist", curTblName);
    return false;
  }

  QString selectPath;
  if (GetAPathFromUserSelect("used to Audit", selectPath)) {
    return false;
  }

  VolumeUpdateResult adtResult{0};
  auto ret = mDb.Adt(curTblName, selectPath, &adtResult);
  if (ret != FD_OK) {
    QString adtMsg{QString{"Audit table[%1] by selectPath[%2] FAILED, see in log details"}.arg(curTblName).arg(selectPath)};
    QString retMsg{QString{"Error code[%3]"}.arg(ret)};
    LOG_BAD(adtMsg, retMsg);
    return false;
  }
  _dbModel->select();
  QString adtMsg = QString{"Audit table[%1] by selectPath[%2] succeed"}.arg(curTblName).arg(selectPath);
  QString retMsg{QString{"Insert: %1/Delete: %2/Update: %3"}.arg(adtResult.insertCnt).arg(adtResult.deleteCnt).arg(adtResult.updateCnt)};
  LOG_GOOD(adtMsg, retMsg);

  return true;
}

bool MovieDBView::onSetDurationByVideo() {
  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    QString lastErrMsg{con.lastError().text()};
    LOG_BAD("[Abort] Open db failed, See detail in log", lastErrMsg)
    return false;
  }
  const QString& curTblName = GetMovieTableName();
  const QString& tblPeerPath = GetMovieTableRootPath();
  if (!QFileInfo{tblPeerPath}.isDir()) {
    const QString offLineMsg = QString{"[Skip] Table[%1] Peer Path[%2] not exist(Disk Maybe Offline)"}.arg(curTblName).arg(tblPeerPath);
    LOG_WARN(offLineMsg, "Skip now");
    return false;
  }
  const int retCnt = mDb.SetDuration(curTblName);
  if (retCnt < 0) {
    const QString errorCode{QString{"errorCode:%1"}.arg(retCnt)};
    LOG_BAD("[Failed] SetDuration failed. See details in logs", errorCode);
    return false;
  }
  _dbModel->select();
  const QString affectedMsg = QString{"%1 record(s) duration field get update"}.arg(retCnt);
  LOG_GOOD("[OK] SetDuration", affectedMsg);
  return true;
}

bool MovieDBView::onExportToJson() {
  if (_dbModel->isDirty()) {
    LOG_WARN("[Skip Export] Table is dirty. must submit/revert first", _tablesDropDownList->currentText());
    return true;
  }

  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    QString lastErrMsg{con.lastError().text()};
    LOG_BAD("[Abort] Open db failed, See detail in log", lastErrMsg)
    return false;
  }
  const QString& curTblName = GetMovieTableName();
  const QStringList& tbs = con.tables();
  if (!tbs.contains(curTblName)) {
    LOG_WARN("[Skip] Table not exist", curTblName);
    return false;
  }
  const QString& tblPeerPath = GetMovieTableRootPath();
  if (!QFileInfo{tblPeerPath}.isDir()) {
    const QString offLineMsg = QString{"[Skip] Table[%1] Peer Path[%2] not exist(Disk Maybe Offline)"}.arg(curTblName).arg(tblPeerPath);
    LOG_WARN(offLineMsg, "Skip now");
    return false;
  }
  const int retCnt = mDb.ExportDurationStudioCastTagsToJson(curTblName);
  if (retCnt < 0) {
    const QString errorMsg{QString{"errorCode:%1, see details in logs"}.arg(retCnt)};
    LOG_BAD("Export changed duration/Studio/Cast/Tags fields to json failed", errorMsg)
    return false;
  }
  _dbModel->select();
  QString jsonAffectedMsg = QString{"%1 json file(s) get update"}.arg(retCnt);
  LOG_GOOD("[OK] Export to json succeed", jsonAffectedMsg);
  return true;
}

bool MovieDBView::onUpdateByJson() {
  if (_dbModel->isDirty()) {
    LOG_WARN("[Skip Update] Table is dirty. must submit/revert first", _tablesDropDownList->currentText());
    return true;
  }

  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    QString lastErrMsg{con.lastError().text()};
    LOG_BAD("[Abort] Open db failed, See detail in log", lastErrMsg)
    return false;
  }
  const QString& curTblName = GetMovieTableName();
  const QStringList& tbs = con.tables();
  if (!tbs.contains(curTblName)) {
    LOG_WARN("[Skip] Table not exist", curTblName);
    return false;
  }

  QString selectPath;
  if (!GetAPathFromUserSelect("read field(s) from json file to update", selectPath)) {
    return false;
  }

  const int retCnt = mDb.UpdateStudioCastTagsByJson(curTblName, selectPath);
  if (retCnt < 0) {
    const QString errorMsg{QString{"errorCode:%1"}.arg(retCnt)};
    LOG_BAD("Update Studio/Cast/Tags/Field(s) by local json file failed. see details in logs", errorMsg);
    return false;
  }
  _dbModel->select();
  const QString affectedMsg{QString{"%1 record(s) get update"}.arg(retCnt)};
  LOG_GOOD("Update Studio/Cast/Tags field(s) by local json file json succeed", affectedMsg);
  return true;
}

int MovieDBView::onCountRow() {
  const QString& tableName = GetMovieTableName();
  const QString& whereClause = _searchWhereLineEdit->text();
  int succeedCnt = mDb.CountRow(tableName, whereClause);
  if (succeedCnt < 0) {
    const QString errorMsg{QString{"errorCode:%1"}.arg(succeedCnt)};
    LOG_BAD("Get rows count failed, See details in logs", errorMsg);
    return succeedCnt;
  }
  const QString cntMsg{QString("Count=%1").arg(succeedCnt)};
  LOG_GOOD("Get rows count succeed", cntMsg);
  QMessageBox::information(this, "Get rows count succeed", cntMsg);
  return succeedCnt;
}

int MovieDBView::onSetStudio() {
  if (!IsHasSelection("set studio")) {
    return 0;
  }
  const auto& curInd = currentIndex();
  const QString& fileName = _dbModel->fileName(curInd);

  static StudiosManager& sm = StudiosManager::getIns();
  const QString defStudio = sm(fileName);
  int defIndex = m_studioCandidates.indexOf(defStudio);
  if (defIndex == -1) {
    m_studioCandidates.push_back(defStudio);
    defIndex = m_studioCandidates.size() - 1;
  }

  bool isInputOk{false};
  QString hintMsg{"Choose or select studio from drop down list"};
  const QString& studio = QInputDialog::getItem(this, "Input an studio name", hintMsg,  //
                                                m_studioCandidates,                     //
                                                defIndex,                               //
                                                true, &isInputOk);
  if (!isInputOk) {
    LOG_GOOD("User cancel set studio", "skip")
    return 0;
  }
  if (studio.isEmpty()) {
    LOG_BAD("Studio name can not be empty", "skip")
    return 0;
  }

  if (m_studioCandidates.indexOf(studio) == -1) {
    auto insertPos = std::upper_bound(m_studioCandidates.begin(), m_studioCandidates.end(), studio);
    m_studioCandidates.insert(insertPos, studio);
  }

  const QModelIndexList& indexes = selectionModel()->selectedRows(MOVIE_TABLE::Studio);
  _dbModel->SetStudio(indexes, studio);
  const QString affectedRowsMsg{QString{"[Uncommit] %1 row(s) studio has been changed to"}.arg(indexes.size())};
  LOG_GOOD(affectedRowsMsg, studio);
  return indexes.size();
}

int MovieDBView::onSetCastOrTags(const FIELD_OP_TYPE type, const FIELD_OP_MODE mode) {
  const QString fieldOperation{"Operation:" + FIELF_OP_TYPE_ARR[(int)type] + ' ' + FIELD_OP_MODE_ARR[(int)mode]};
  if (!IsHasSelection(fieldOperation)) {
    return 0;
  }
  QStringList& candidates = m_tagsCandidates[(int)type];
  bool isInputOk{false};
  const QString hintMsg{QString{"Choose or select from drop down list[%1]"}.arg(fieldOperation)};
  const QString& tagsOrCast = QInputDialog::getItem(this, fieldOperation, hintMsg,  //
                                                    candidates,                     //
                                                    candidates.size() - 1,          //
                                                    true, &isInputOk);
  if (!isInputOk) {
    LOG_GOOD("[Skip] User cancel", fieldOperation)
    return 0;
  }
  if (tagsOrCast.isEmpty()) {
    LOG_BAD("[Abort] Input can not be empty", fieldOperation)
    return 0;
  }

  candidates.push_back(tagsOrCast);
  int fieldColumn = -1;
  switch (type) {
    case FIELD_OP_TYPE::CAST:
      fieldColumn = MOVIE_TABLE::Cast;
      break;
    case FIELD_OP_TYPE::TAGS:
      fieldColumn = MOVIE_TABLE::Tags;
      break;
    default:
      LOG_BAD("Field Type invalid", QString::number((int)type));
      return -1;
  }

  const QModelIndexList& indexes = selectionModel()->selectedRows(fieldColumn);
  switch (mode) {
    case FIELD_OP_MODE::SET:
      _dbModel->SetCastOrTags(indexes, tagsOrCast);
      break;
    case FIELD_OP_MODE::APPEND:
      _dbModel->AddCastOrTags(indexes, tagsOrCast);
      break;
    case FIELD_OP_MODE::REMOVE:
      _dbModel->RmvCastOrTags(indexes, tagsOrCast);
      break;
    default:
      LOG_BAD("Field Operation invalid", QString::number((int)mode));
      return -2;
  }
  const QString affectedRowsMsg{QString{"[Uncommit] %1 row(s) %2"}.arg(indexes.size()).arg(fieldOperation)};
  LOG_GOOD(affectedRowsMsg, tagsOrCast);
  return indexes.size();
}

bool MovieDBView::IsHasSelection(const QString& msg) const {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO("Nothing selected", QString("Skip %1").arg(msg))
    return false;
  }
  return true;
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>
#include "Tools/SubscribeDatabase.h"
#include <QMainWindow>
class MoviesDatabase : public QMainWindow {
 public:
  RibbonMovieDB* m_databaseTB;
  DatabaseTableView* m_dbPanel;
  explicit MoviesDatabase(QWidget* parent = nullptr) : QMainWindow(parent), m_databaseTB(new RibbonMovieDB("Movies Database Toolbar", this)), m_dbPanel(new DatabaseTableView) {
    this->addToolBar(Qt::ToolBarArea::TopToolBarArea, m_databaseTB);
    this->setCentralWidget(m_dbPanel);
    this->setWindowTitle("QTableView Example");
    this->setWindowIcon(QIcon(":img/SHOW_DATABASE"));
  }
  QSize sizeHint() const override { return QSize(1400, 768); }
};

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  //  QWidget widget;
  //  QMessageBox::warning(&widget, "VIDS_DATABASE path", SystemPath::VIDS_DATABASE);
  //  widget.show();
  MoviesDatabase win;
  win.show();
  auto* eventImplementer = new SubscribeDatabase(win.m_dbPanel->m_dbView, win.m_dbPanel->m_dbView->m_dbModel, win.m_dbPanel->m_searchLE);
  return a.exec();
}
#endif
