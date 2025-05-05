#include "MovieDBView.h"

#include "Actions/MovieDBActions.h"
#include "Actions/FileBasicOperationsActions.h"

#include "Component/QuickWhereClause.h"
#include "Component/Notificator.h"

#include "Tools/FileDescriptor/MountHelper.h"

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
  connect(inst.INIT_A_DATABASE, &QAction::triggered, this, &MovieDBView::onInitDataBase);
  connect(inst.INIT_A_TABLE, &QAction::triggered, this, &MovieDBView::onCreateATable);
  connect(inst.DROP_A_TABLE, &QAction::triggered, this, &MovieDBView::onDropATable);
  connect(inst.INSERT_A_PATH, &QAction::triggered, this, &MovieDBView::onInsertIntoTable);
  connect(inst.DELETE_FROM_TABLE, &QAction::triggered, this, &MovieDBView::onDeleteFromTable);
  connect(inst.UNION_TABLE, &QAction::triggered, this, &MovieDBView::onUnionTables);
  connect(inst.AUDIT_A_TABLE, &QAction::triggered, this, &MovieDBView::onAuditATable);
  // extra function actions
  connect(inst.SET_DURATION_BY_VIDEO, &QAction::triggered, this, &MovieDBView::onSetDurationByVideo);
  connect(inst.EXPORT_DURATION_STUDIO_CAST_TAGS_TO_JSON, &QAction::triggered, this, &MovieDBView::onExportToJson);
  connect(inst.UPDATE_STUDIO_CAST_TAGS_BY_JSON, &QAction::triggered, this, &MovieDBView::onUpdateByJson);
  // assistance actions
  connect(inst.QUICK_WHERE_CLAUSE, &QAction::triggered, this, &MovieDBView::onQuickWhereClause);
  // common function actions
  connect(inst._COUNT, &QAction::triggered, this, &MovieDBView::onCountRow);

  addAction(g_fileBasicOperationsActions().COPY_FULL_PATH);
  addAction(g_fileBasicOperationsActions().COPY_NAME);
  addAction(g_fileBasicOperationsActions().COPY_RECORDS);
}

bool MovieDBView::onSearchDataBase() {
  const QString& searchPattern = _searchWhereLineEdit->text();
  _dbModel->setFilter(searchPattern);
  _dbModel->select();
  if (_dbModel->lastError().isValid()) {
    Notificator::badNews(QString("FAIL set new searchPattern[%1]").arg(searchPattern), _dbModel->lastError().text());
    return false;
  }
  Notificator::goodNews("Succeed set new searchPattern", searchPattern);
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
    Notificator::warning(QString{"Table[%1] Peer Path[%2] not exist(Disk Maybe Offline)"}.arg(curTblName).arg(tblPeerPath), "Skip now");
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
    Notificator::warning("User cancel insert, path is not directory", selectPath);
    return false;
  }
  if (!selectPath.startsWith(tblPeerPath)) {
    Notificator::warning("Path user selected is not under table PeerPath", selectPath + '\n' + tblPeerPath);
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
    Notificator::badNews("Open failed", con.lastError().text());
    return false;
  }

  const QString& curTblName = GetMovieTableName();
  if (!con.tables().contains(curTblName)) {
    Notificator::badNews(curTblName, "Table NOT exist. ABORT insert");
    return false;
  }

  QString selectPath;
  if (!GetAPathFromUserSelect("and load videos into", selectPath)) {
    return false;
  }

  const QString msg{QString{"%1/* ----->---- Table: %2"}.arg(selectPath).arg(_tablesDropDownList->currentText())};
  if (QMessageBox::question(this, "CONFIRM INSERT INTO?", msg) != QMessageBox::StandardButton::Yes) {
    Notificator::information("User cancel insert", selectPath);
    return true;
  }

  int retCnt = mDb.ReadADirectory(curTblName, selectPath);
  if (retCnt < 0) {
    Notificator::badNews(QString("Read videos from path failed, code:%1").arg(retCnt), selectPath);
    return false;
  }

  Notificator::goodNews(QString("Read videos from path succeed, count:%1").arg(retCnt), selectPath);
  QMessageBox::information(this, "Read videos from path succeed", QString("Count=%1").arg(retCnt));
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
    Notificator::goodNews("Table not dirty, Skip", _tablesDropDownList->currentText());
    return true;
  }
  if (!_dbModel->submitAll()) {
    Notificator::badNews("Submit failed. see details in logs", _tablesDropDownList->currentText());
    return false;
  }

  Notificator::goodNews("Submit succeed", _tablesDropDownList->currentText());
  return true;
}

bool MovieDBView::onInitDataBase() {
  const bool crtResult = mDb.CreateDatabase();
  if (!crtResult) {
    Notificator::badNews("Init database failed, reason:", mDb.GetCfgDebug());
  }
  Notificator::goodNews("Init database succeed", "...");
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
    qWarning("User cancel[%d]", isInputOk);
    Notificator::warning("Create table failed", "User cancel.");
    return;
  }
  const QString& newTbl = MountHelper::ChoppedDisplayName(crtTbl);
  if (newTbl.isEmpty() || tables.contains(newTbl)) {
    Notificator::warning("Table name " + newTbl, "empty or already occupied");
    return;
  }

  if (newTbl.contains(JSON_RENAME_REGEX::INVALID_TABLE_NAME_LETTER)) {
    Notificator::badNews("Table name " + newTbl, "contains invalid letter.");
    return;
  }

  if (!mDb.CreateTable(newTbl, FdBasedDb::CREATE_TABLE_TEMPLATE)) {
    Notificator::badNews(newTbl + " Table name created failed", "see detail in log");
    return;
  }

  _tablesDropDownList->AddItem(newTbl, MountHelper::GetDisplayNameByGuidTableName(newTbl));
  _tablesDropDownList->setCurrentIndex(_tablesDropDownList->count() - 1);
  Notificator::goodNews("Table name " + newTbl, "created succeed");
}

bool MovieDBView::onDropATable() {
  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    Notificator::badNews("Open db failed, reason:", con.lastError().text());
    return false;
  }
  const QStringList& tables = con.tables();
  if (tables.isEmpty()) {
    Notificator::goodNews("No need drop", "No table exist at all");
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
    Notificator::goodNews("User cancel drop table", "cancel");
    return true;
  }
  const QString& deleteTbl = MountHelper::ChoppedDisplayName(drpTbl);
  if (deleteTbl.isEmpty()) {
    Notificator::badNews("Table name[%s] is empty, cannot drop", deleteTbl);
    return true;
  }

  if (!mDb.DropTable(deleteTbl)) {
    Notificator::badNews(QString("Table[%s] drop failed").arg(deleteTbl), con.lastError().text());
    return false;
  }
  InitMoviesTables();
  Notificator::goodNews("Table has been dropped", deleteTbl);
  return true;
}

bool MovieDBView::onDeleteFromTable() {
  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    qWarning("Open db failed, reason:%s", qPrintable(con.lastError().text()));
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
    qDebug("User Cancel delete row");
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
    Notificator::goodNews("User Cancel delete records", "Skip");
    return true;
  }

  const int affectedRows = mDb.DeleteByWhereClause(tbl, whereClause);
  if (affectedRows < 0) {
    Notificator::badNews("Delete failed", deleteCmd);
    return false;
  }
  Notificator::goodNews(deleteCmd + " succceed", QString("%1 record(s) affected").arg(affectedRows));
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
    qWarning("Open db failed:%s", qPrintable(con.lastError().text()));
    return false;
  }

  const QStringList& tbs = con.tables();
  if (!tbs.contains(DB_TABLE::MOVIES)) {
    QMessageBox::warning(this, "Destination table not exist. Create it at first", DB_TABLE::MOVIES);
    return false;
  }

  const int SRC_TABLE_CNT = tbs.size() - 1;
  if (SRC_TABLE_CNT <= 1) {
    Notificator::information("No need union", "Only one table find(except destination table)");
    return true;
  }

  const QString& confirmUnionHintMsg = QString("All %1 tables into Table[%2]").arg(SRC_TABLE_CNT).arg(DB_TABLE::MOVIES);
  if (QMessageBox::question(this, "Confirm Union?", confirmUnionHintMsg, QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) {
    Notificator::goodNews(QString("User cancel union %1 tables").arg(SRC_TABLE_CNT), "Skip");
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
    Notificator::goodNews(title, msg);
    QMessageBox::warning(this, title, msg);
    con.rollback();
    return false;
  }
  unionTableQry.finish();
  QMessageBox::information(this, DB_TABLE::MOVIES, "Union into succeed");
  return true;
}

bool MovieDBView::onAuditATable() {
  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    qWarning("Open db failed:%s", qPrintable(con.lastError().text()));
    return false;
  }
  const QString& curTblName = GetMovieTableName();
  const QStringList& tbs = con.tables();
  if (!tbs.contains(curTblName)) {
    Notificator::warning(QString{"Table[%1] not exist in %1 table(s)"}.arg(curTblName).arg(tbs.size()), "Skip audit now");
    return false;
  }

  QString selectPath;
  if (GetAPathFromUserSelect("used to Audit", selectPath)) {
    return false;
  }

  VolumeUpdateResult adtResult{0};
  auto ret = mDb.Adt(curTblName, selectPath, &adtResult);
  if (ret != FD_OK) {
    Notificator::badNews(QString{"Audit table[%1] by selectPath[%2] FAILED"}.arg(curTblName).arg(selectPath),  //
                         QString{"Error code[%3], see in log details"}.arg(ret));
    return false;
  }
  _dbModel->select();
  Notificator::goodNews(QString{"Audit table[%1] by selectPath[%2] succeed"}.arg(curTblName).arg(selectPath),  //
                        QString{"Insert: %1/Delete: %2/Update: %3"}.arg(adtResult.insertCnt).arg(adtResult.deleteCnt).arg(adtResult.updateCnt));

  return true;
}

bool MovieDBView::onSetDurationByVideo() {
  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    qWarning("Open db failed:%s", qPrintable(con.lastError().text()));
    return false;
  }
  const QString& curTblName = GetMovieTableName();
  const QString& tblPeerPath = GetMovieTableRootPath();
  if (!QFileInfo{tblPeerPath}.isDir()) {
    Notificator::warning(QString{"Table[%1] Peer Path[%2] not exist(Disk Maybe Offline)"}.arg(curTblName).arg(tblPeerPath), "Skip now");
    return false;
  }
  const int retCnt = mDb.SetDuration(curTblName);
  if (retCnt < 0) {
    Notificator::badNews("SetDuration failed", QString{"errorCode:%1, see details in logs"}.arg(retCnt));
    return false;
  }
  _dbModel->select();
  Notificator::goodNews("SetDuration succeed", QString{"%1 record(s) duration field get update"}.arg(retCnt));
  return true;
}

bool MovieDBView::onExportToJson() {
  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    qWarning("Open db failed:%s", qPrintable(con.lastError().text()));
    return false;
  }
  const QString& curTblName = GetMovieTableName();
  const QStringList& tbs = con.tables();
  if (!tbs.contains(curTblName)) {
    Notificator::warning(QString{"Table[%1] not exist in %1 table(s)"}.arg(curTblName).arg(tbs.size()), "Skip export valid field(s) to local json file now");
    return false;
  }
  const QString& tblPeerPath = GetMovieTableRootPath();
  if (!QFileInfo{tblPeerPath}.isDir()) {
    Notificator::warning(QString{"Table[%1] Peer Path[%2] not exist(Disk Maybe Offline)"}.arg(curTblName).arg(tblPeerPath), "Skip now");
    return false;
  }
  const int retCnt = mDb.ExportDurationStudioCastTagsToJson(curTblName);
  if (retCnt < 0) {
    Notificator::badNews("Export changed duration/Studio/Cast/Tags fields to json failed", QString{"errorCode:%1, see details in logs"}.arg(retCnt));
    return false;
  }
  _dbModel->select();
  Notificator::goodNews("Export changed duration/Studio/Cast/Tags fields to json succeed", QString{"%1 json file(s) get update"}.arg(retCnt));
  return true;
}

bool MovieDBView::onUpdateByJson() {
  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    qWarning("Open db failed:%s", qPrintable(con.lastError().text()));
    return false;
  }
  const QString& curTblName = GetMovieTableName();
  const QStringList& tbs = con.tables();
  if (!tbs.contains(curTblName)) {
    Notificator::warning(QString{"Table[%1] not exist in %1 table(s)"}.arg(curTblName).arg(tbs.size()), "Skip update field(s) by local json file now");
    return false;
  }

  QString selectPath;
  if (!GetAPathFromUserSelect("read field(s) from json file to update", selectPath)) {
    return false;
  }

  const int retCnt = mDb.UpdateStudioCastTagsByJson(curTblName, selectPath);
  if (retCnt < 0) {
    Notificator::badNews("Update Studio/Cast/Tags/Field(s) by local json file failed", QString{"errorCode:%1, see details in logs"}.arg(retCnt));
    return false;
  }
  _dbModel->select();
  Notificator::goodNews("Update Studio/Cast/Tags/Field(s) by local json file json succeed", QString{"%1 record(s) get update"}.arg(retCnt));
  return true;
}

int MovieDBView::onCountRow() {
  const QString& tableName = GetMovieTableName();
  const QString& whereClause = _searchWhereLineEdit->text();
  int succeedCnt = mDb.CountRow(tableName, whereClause);
  if (succeedCnt < 0) {
    Notificator::badNews(QString("Get rows count failed, code: %1").arg(succeedCnt), "See details in logs");
    return succeedCnt;
  }
  Notificator::goodNews(QString("Get rows count failed, code: %1").arg(succeedCnt), "See details in logs");
  QMessageBox::information(this, "Get rows count succeed", QString("Count=%1").arg(succeedCnt));
  return succeedCnt;
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
