#include "MovieDBView.h"

#include "Actions/DataBaseActions.h"
#include "Actions/FileBasicOperationsActions.h"

#include "Component/QuickWhereClause.h"
#include "Component/Notificator.h"

#include "Tools/PlayVideo.h"

#include "public/PublicMacro.h"
#include "public/PublicVariable.h"
#include "public/MemoryKey.h"

#include <QDesktopServices>
#include <QHeaderView>

#include <QSqlError>
#include <QSqlQuery>

#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>

MovieDBView::MovieDBView(MyQSqlTableModel* model_,             //
                         DatabaseSearchToolBar* _dbSearchBar,  //
                         MovieBaseDb& movieDb_,
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

  setEditTriggers(QAbstractItemView::NoEditTriggers);  // only F2 works. QAbstractItemView.NoEditTriggers

  InitMoviesTables();
  const QString defaultTableName = PreferenceSettings().value(MemoryKey::VIDS_LAST_TABLE_NAME.name, MemoryKey::VIDS_LAST_TABLE_NAME.v).toString();
  int defaultTableIndex = _tablesDropDownList->findText(defaultTableName, Qt::MatchExactly);
  if (defaultTableIndex != -1) {
    _tablesDropDownList->setCurrentIndex(defaultTableIndex);
  }
  setCurrentMovieTable(_tablesDropDownList->currentText());

  subscribe();
  InitTableView();
}

void MovieDBView::subscribe() {
  connect(horizontalHeader(), &QHeaderView::sectionResized, this, [this]() { PreferenceSettings().setValue("DATABASE_TABLEVIEW_HERDER_GEOMETRY", horizontalHeader()->saveState()); });

  connect(_searchWhereLineEdit, &QLineEdit::returnPressed, this, [this]() {
    const QString& searchPattern = _searchWhereLineEdit->text();
    onSearchDataBase(searchPattern);
  });

  connect(_tablesDropDownList, &QComboBox::currentTextChanged, this, &MovieDBView::setCurrentMovieTable);

  {
    const QList<QAction*>& DB_CONTROL_ACTIONS = g_dbAct().DB_CONTROL_ACTIONS->actions();

    QAction* INSERT_A_PATH = DB_CONTROL_ACTIONS[0];
    QAction* DELETE_FROM_TABLE = DB_CONTROL_ACTIONS[1];
    QAction* INIT_A_DATABASE = DB_CONTROL_ACTIONS[2];
    QAction* INIT_A_TABLE = DB_CONTROL_ACTIONS[3];
    QAction* DROP_A_DATABASE = DB_CONTROL_ACTIONS[4];
    QAction* DROP_A_TABLE = DB_CONTROL_ACTIONS[5];
    QAction* UNION_TABLE = DB_CONTROL_ACTIONS[6];

    connect(INIT_A_DATABASE, &QAction::triggered, this, &MovieDBView::onInitDataBase);
    connect(DROP_A_DATABASE, &QAction::triggered, this, [this]() { QMessageBox::warning(this, "Too danger", "Cancel drop database"); });

    connect(INIT_A_TABLE, &QAction::triggered, this, &MovieDBView::onCreateATable);
    connect(DROP_A_TABLE, &QAction::triggered, this, &MovieDBView::onDropATable);
    connect(INSERT_A_PATH, &QAction::triggered, this, &MovieDBView::onInsertIntoTable);
    connect(DELETE_FROM_TABLE, &QAction::triggered, this, [this]() { this->onDeleteFromTable(); });

    connect(UNION_TABLE, &QAction::triggered, this, &MovieDBView::onUnionTables);
  }

  {
    connect(g_dbAct().DELETE_BY_DRIVER, &QAction::triggered, this, &MovieDBView::on_DeleteByDrive);
    connect(g_dbAct().DELETE_BY_PREPATH, &QAction::triggered, this, &MovieDBView::on_DeleteByPrepath);
  }

  { connect(g_dbAct().QUICK_WHERE_CLAUSE, &QAction::triggered, this, &MovieDBView::onQuickWhereClause); }

  { connect(g_dbAct()._COUNT, &QAction::triggered, this, &MovieDBView::onCountRow); }

  addAction(g_fileBasicOperationsActions().COPY_FULL_PATH);
  addAction(g_fileBasicOperationsActions().COPY_NAME);
  addAction(g_fileBasicOperationsActions().COPY_RECORDS);
}

bool MovieDBView::on_PlayVideo() const {
  // select an item or select nothing
  const int selectedCnt = selectedIndexes().size();
  QString playPath;
  if (selectedCnt == 0) {
    if (_dbModel->rootDirectory().isRoot()) {
      qDebug("root path is so large range. skip");
      return true;
    }
    playPath = _dbModel->rootPath();
  } else if (selectedCnt == 1) {
    QModelIndex curIndex = selectionModel()->currentIndex();
    QFileInfo selectedFi = _dbModel->fileInfo(curIndex);
    if (selectedFi.isDir()) {
      if (QDir(selectedFi.absoluteFilePath()).isRoot()) {
        qDebug("root path is so large range. skip");
        return true;
      }
    }
    playPath = selectedFi.absoluteFilePath();
  } else {
    qDebug("Select nothing MyQFileSystemModel or JUST Select 1");
    return true;
  }

  auto ret = on_ShiftEnterPlayVideo(playPath);
  qDebug(ret ? "Playing ..." : "Nothing to play");
  return ret;
}

bool MovieDBView::InitMoviesTables() {
  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    qWarning("opened failed:%s", qPrintable(con.lastError().text()));
    return false;
  }
  CHECK_NULLPTR_RETURN_FALSE(_tablesDropDownList);
  const QStringList& tbls = con.tables();
  _tablesDropDownList->clear();
  _tablesDropDownList->addItems(tbls);
  qDebug("tables count:%d", tbls.size());
  return true;
}

bool MovieDBView::setCurrentMovieTable(const QString& movieTableName) {
  qDebug("Set Current table to[%s]", qPrintable(movieTableName));
  PreferenceSettings().setValue(MemoryKey::VIDS_LAST_TABLE_NAME.name, movieTableName);
  _tablesDropDownList->setCurrentText(movieTableName);
  _dbModel->setTable(movieTableName);
  //  m_dbModel->select();
  _dbModel->submitAll();
  InitTableView();
  return true;
}

bool MovieDBView::onUnionTables() {
  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    qDebug("Open failed:%s", qPrintable(con.lastError().text()));
    return false;
  }

  const QStringList& tbs = con.tables();
  if (!con.tables().contains(DB_TABLE::MOVIES)) {
    QMessageBox::warning(this, DB_TABLE::MOVIES, "Destination table not exist. Create it at first");
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
  QMessageBox::information(this, DB_TABLE::MOVIES, "Union into Finish");
  return true;
}

bool MovieDBView::onInitDataBase() {
  if (!mDb.CreateDatabase()) {
    Notificator::badNews("Init database failed", mDb.GetCfgDebug());
  }
  Notificator::goodNews("Init database succeed", mDb.GetCfgDebug());
  return true;
}

void MovieDBView::onCreateATable() {
  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    qDebug("Open failed:%s", qPrintable(con.lastError().text()));
    return;
  }

  bool tablesNameOk{false};
  const QStringList& tables = con.tables();
  const QString& tablesLabel = tables.join('\n');
  const QString& tableName = QInputDialog::getText(this, "Input an unique table name",  //
                                                   tablesLabel,                         //
                                                   QLineEdit::Normal, "", &tablesNameOk);
  if (!tablesNameOk || tableName.isEmpty() || tables.contains(tableName)) {
    qWarning("User cancel[%d], Table[%s] name empty, or already occupied",  //
             tablesNameOk, qPrintable(tableName));
    Notificator::warning("Create table failed", "User cancel or table name empty or occupied.");
    return;
  }

  if (tableName.contains(JSON_RENAME_REGEX::INVALID_TABLE_NAME_LETTER)) {
    qDebug("Table name[%s] contains invalid letter", qPrintable(tableName));
    Notificator::warning(tableName, QString("Table name contains invalid letter, create failed."));
    return;
  }

  if (!mDb.CreateTable(DB_TABLE::MOVIES, MovieBaseDb::CREATE_MOVIE_TABLE_TEMPLATE)) {
    Notificator::badNews(tableName + " Table name created failed", "see detail in log");
    return;
  }

  _tablesDropDownList->addItem(tableName);
  _tablesDropDownList->setCurrentText(tableName);
  Notificator::goodNews(tableName, "Table name created succeed");
}

bool MovieDBView::onDropATable() {
  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    qWarning("Open failed:%s", qPrintable(con.lastError().text()));
    return false;
  }
  const QStringList& tables = con.tables();
  if (tables.isEmpty()) {
    Notificator::goodNews("no need drop a table", "No tables at all");
    return true;
  }

  bool okUserSelect = false;
  const QString& tableName = QInputDialog::getItem(this, "CONFIRM DROP?",                               //
                                                   "Choose a table to drop (NOT RECOVERABLE)",          //
                                                   tables,                                              //
                                                   tables.indexOf(_tablesDropDownList->currentText()),  //
                                                   false,                                               //
                                                   &okUserSelect);
  if (!okUserSelect || tableName.isEmpty()) {
    Notificator::goodNews("User cancel drop table", tableName);
    return true;
  }

  if (!mDb.DropTable('^' + tableName + '|')) {
    Notificator::badNews("Table drop failed", tableName);
    return false;
  }
  InitMoviesTables();
  Notificator::goodNews("Table has been dropped", tableName);
  return true;
}

bool MovieDBView::onDeleteFromTable(const QString& clause) {
  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    qWarning("Open failed:%s", qPrintable(con.lastError().text()));
    return false;
  }

  const QString& deleteFromTable = _tablesDropDownList->currentText();
  QString whereClause = clause;
  if (clause.isEmpty()) {
    static const QStringList deleteDriverChoicePool{QString("\"%1\"=\"\"").arg(DB_HEADER_KEY::Name), QString("\"%1\"=\"\"").arg(DB_HEADER_KEY::Size), QString("\"%1\"=\"\"").arg(DB_HEADER_KEY::Driver),
                                                    QString("\"%1\"=\"\"").arg(DB_HEADER_KEY::Prepath)};

    bool okClicked = false;
    whereClause = QInputDialog::getItem(this, "Delete where", QString("DELETE FROM \"%1\" WHERE").arg(deleteFromTable), deleteDriverChoicePool, 0, true, &okClicked);
    if (!okClicked || whereClause.isEmpty()) {
      qDebug("User Cancel delete row");
      return true;
    }
  }

  if (QMessageBox::question(this, "Confirm Delete(NOT recoverable)?", whereClause, QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) {
    Notificator::goodNews("skip", "User Cancel delete records");
    return true;
  }

  if (!mDb.DeleteByWhereClause(deleteFromTable, whereClause)) {
    Notificator::badNews("Delete record(s) failed", QString("DELETE FROM %1 WHERE %2").arg(deleteFromTable).arg(whereClause));
    return false;
  }
  Notificator::goodNews("Delete record(s) succceed", QString("DELETE FROM %1 WHERE %2").arg(deleteFromTable).arg(whereClause));
  _dbModel->submitAll();
  return true;
}

bool MovieDBView::on_DeleteByDrive() {
  QSet<QString> driversSet;
  for (const auto rowIndex : selectionModel()->selectedRows()) {
    const QString& curDriver = _dbModel->driver(rowIndex);  // TODO:driver
    if (driversSet.contains(curDriver)) {
      continue;
    }
    driversSet.insert(curDriver);
    const QString& whereClause = QString(R"(`%1`="%2")").arg(DB_HEADER_KEY::Driver, curDriver);
    if (!onDeleteFromTable(whereClause)) {
      qWarning("delete by drive failed:%s", qPrintable(whereClause));
      return false;
    }
  }
  return true;
}

bool MovieDBView::on_DeleteByPrepath() {
  QSet<QString> prepathSet;
  for (const auto& rowIndex : selectionModel()->selectedRows()) {
    const QString& prepath = _dbModel->absolutePath(rowIndex);
    if (prepathSet.contains(prepath)) {
      continue;
    }
    prepathSet.insert(prepath);
    const QString& whereClause = QString(R"(`%1`="%2")").arg(DB_HEADER_KEY::Prepath, prepath);
    if (!onDeleteFromTable(whereClause)) {
      qWarning("delete by prepath failed:%s", qPrintable(whereClause));
      return false;
    }
  }
  return true;
}

bool MovieDBView::onInsertIntoTable() {
  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    Notificator::badNews("Open failed", con.lastError().text());
    return false;
  }
  const QString& operateTable = _tablesDropDownList->currentText();
  if (!con.tables().contains(operateTable)) {
    Notificator::badNews(operateTable, "Table NOT exist. ABORT insert");
    return false;
  }

  QString lastPath = PreferenceSettings().value(MemoryKey::PATH_DB_INSERT_VIDS_FROM.name, MemoryKey::PATH_DB_INSERT_VIDS_FROM.v).toString();
  if (!QFileInfo(lastPath).isDir()) {  // fallback
    lastPath = MemoryKey::PATH_DB_INSERT_VIDS_FROM.v.toString();
  }
  const QString& selectPath = QFileDialog::getExistingDirectory(this,                                         //
                                                                "Choose a path into table: " + operateTable,  //
                                                                lastPath, QFileDialog::ShowDirsOnly);
  if (selectPath.isEmpty()) {
    Notificator::warning("User cancel insert, path is not directory", selectPath);
    return false;
  }

  PreferenceSettings().setValue(MemoryKey::PATH_DB_INSERT_VIDS_FROM.name, selectPath);
  if (QMessageBox::question(this, "CONFIRM INSERT?", selectPath + "/* ----->---- Table:" + operateTable) != QMessageBox::StandardButton::Yes) {
    Notificator::information("User cancel insert", selectPath);
    return true;
  }

  int retCnt = mDb.ReadVideosFromAPath(selectPath, operateTable);
  if (retCnt < 0) {
    Notificator::badNews(QString("Read videos from path failed, code:%1").arg(retCnt), selectPath);
    return false;
  }

  Notificator::goodNews(QString("Read videos from path succeed, count:%1").arg(retCnt), selectPath);
  QMessageBox::information(this, "Read videos from path succeed", QString("Count=%1").arg(retCnt));
  _dbModel->submitAll();
  return true;
}

void MovieDBView::onQuickWhereClause() {
  auto retCode = m_quickWhereClause->exec();
  if (retCode != QDialog::DialogCode::Accepted) {
    return;
  }
  const QString& where = m_quickWhereClause->GetWhereString();
  qDebug("Quick where clause: [%s]", qPrintable(where));
  _searchWhereLineEdit->setText(where);
  emit _searchWhereLineEdit->returnPressed();
}

int MovieDBView::onCountRow() {
  const QString& tableName = _tablesDropDownList->currentText();
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

// #include <QMainWindow>
// class MoviesDatabase : public QMainWindow {
//  public:
//   DatabaseToolBar* m_databaseTB;
//   DatabaseTableView* m_dbPanel;
//   explicit MoviesDatabase(QWidget* parent = nullptr)
//       : QMainWindow(parent), m_databaseTB(new DatabaseToolBar("Movies Database Toolbar", this)), m_dbPanel(new DatabaseTableView) {
//     this->addToolBar(Qt::ToolBarArea::TopToolBarArea, m_databaseTB);
//     this->setCentralWidget(m_dbPanel);
//     this->setWindowTitle("QTableView Example");
//     this->setWindowIcon(QIcon(":img/SHOW_DATABASE"));
//   }
//   QSize sizeHint() const override { return QSize(1400, 768); }
// };

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>
#include "Tools/SubscribeDatabase.h"

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
