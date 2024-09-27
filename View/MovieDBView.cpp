#include "MovieDBView.h"

#include "Actions/DataBaseActions.h"
#include "Actions/FileBasicOperationsActions.h"
#include "Component/QuickWhereClause.h"
#include "Tools/PlayVideo.h"

#include "PublicTool.h"
#include "PublicVariable.h"

#include <QDesktopServices>
#include <QDirIterator>
#include <QHeaderView>
#include <QProcess>
#include <QStorageInfo>

#include <QSqlError>
#include <QSqlQuery>

MovieDBView::MovieDBView(DatabaseSearchToolBar* _dbSearchBar, MyQSqlTableModel* dbModel, QWidget* parent)
    : CustomTableView("MOVIE_TABLE", parent),
      _dbModel(dbModel),
      m_movieMenu{new MovieDatabaseMenu("Movie Right click menu", this)},
      _dbSearchBar{_dbSearchBar},
      _tables{_dbSearchBar->m_tables},
      _searchLE{_dbSearchBar->m_searchLE},
      _searchCB{_dbSearchBar->m_searchCB},
      m_quickWhereClause{new QuickWhereClause(this)} {
  BindMenu(m_movieMenu);

  setModel(_dbModel);

  setEditTriggers(QAbstractItemView::NoEditTriggers);  // only F2 works. QAbstractItemView.NoEditTriggers

  InitMoviesTables();
  const QString defaultTableName = PreferenceSettings().value(MemoryKey::VIDS_LAST_TABLE_NAME.name, MemoryKey::VIDS_LAST_TABLE_NAME.v).toString();
  int defaultTableIndex = _tables->findText(defaultTableName, Qt::MatchExactly);
  if (defaultTableIndex != -1) {
    _tables->setCurrentIndex(defaultTableIndex);
  }
  setCurrentMovieTable(_tables->currentText());

  subscribe();
  InitTableView();
}

void MovieDBView::subscribe() {
  connect(horizontalHeader(), &QHeaderView::sectionResized, this,
          [this]() { PreferenceSettings().setValue("DATABASE_TABLEVIEW_HERDER_GEOMETRY", horizontalHeader()->saveState()); });

  connect(_searchLE, &QLineEdit::returnPressed, this, [this]() {
    const QString& searchPattern = _searchLE->text();
    onSearchDataBase(searchPattern);
  });

  connect(_tables, &QComboBox::currentTextChanged, this, &MovieDBView::setCurrentMovieTable);

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

  {
    const QList<QAction*>& DB_FUNCTIONS_ACTIONS = g_dbAct().DB_FUNCTIONS->actions();
    QAction* COUNT = DB_FUNCTIONS_ACTIONS[0];
    QAction* SUM = DB_FUNCTIONS_ACTIONS[1];
    connect(COUNT, &QAction::triggered, this, &MovieDBView::onCountRow);
  }

  addAction(g_fileBasicOperationsActions().COPY_FULL_PATH);
  addAction(g_fileBasicOperationsActions().COPY_NAME);
  addAction(g_fileBasicOperationsActions().COPY_RECORDS);
}

auto MovieDBView::on_PlayVideo() const -> bool {
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
  QSqlDatabase con = GetSqlVidsDB();
  if (!con.isOpen()) {
    qDebug("db[%s] not opened[%s]", qPrintable(SystemPath::VIDS_DATABASE), qPrintable(con.lastError().text()));
    return false;
  }
  if (_tables == nullptr) {
    qDebug("m_tables is nullptr");
    return false;
  }
  _tables->clear();
  _tables->addItems(con.tables());
  return true;
}

bool MovieDBView::setCurrentMovieTable(const QString& movieTableName) {
  m_movieTableName = movieTableName;
  _tables->setCurrentText(movieTableName);
  _dbModel->setTable(movieTableName);
  _dbModel->submitAll();
  InitTableView();
  PreferenceSettings().setValue(MemoryKey::VIDS_LAST_TABLE_NAME.name, movieTableName);
  return true;
}

bool MovieDBView::onUnionTables() {
  QSqlDatabase con = GetSqlVidsDB();
  if (not con.isOpen() and not con.open()) {
    qDebug("Con cannot open");
    return false;
  }

  const QStringList& tbs = con.tables();
  if (not con.tables().contains(DB_TABLE::MOVIES)) {
    QMessageBox::warning(this, DB_TABLE::MOVIES, "Destination table not exist. Create it at first");
    return false;
  }
  const int SRC_TABLE_CNT = tbs.size() - 1;
  if (SRC_TABLE_CNT < 1) {
    qDebug("nothing to union");
    return true;
  }
  QStringList unionSrcTbs;
  for (const QString& srcTable : tbs) {
    if (srcTable == DB_TABLE::MOVIES) {
      continue;
    }
    unionSrcTbs << QString("SELECT * FROM `%1`").arg(srcTable);
  }
  // REPLACE INTO `MOVIES` SELECT * FROM `A568` UNION SELECT * FROM `AASAD`;
  const QString& unionStr = unionSrcTbs.join(" UNION ");
  const QString& unionCmd = QString("REPLACE INTO `%1` %2").arg(DB_TABLE::MOVIES).arg(unionStr);
  QSqlQuery unionTableQry(con);
  const auto ret = unionTableQry.exec(unionCmd);
  if (not ret) {
    qDebug("%s. \nUnion %d table(s) into [%s] failed. \n%s", qPrintable(unionCmd), SRC_TABLE_CNT, qPrintable(DB_TABLE::MOVIES),
           qPrintable(unionTableQry.lastError().text()));
    QMessageBox::warning(this, DB_TABLE::MOVIES,
                         unionCmd + QString("\nUnion %1 table(s) failed.\n").arg(SRC_TABLE_CNT) + unionTableQry.lastError().text());
    con.rollback();
    return false;
  }
  unionTableQry.finish();
  QMessageBox::information(this, DB_TABLE::MOVIES, "Union into Finish");
  return true;
}

bool MovieDBView::onInitDataBase() {
  QSqlDatabase con = GetSqlVidsDB();
  if (not con.isOpen()) {
    qDebug("con cannot open");
    return false;
  }
  qDebug("Database create succeed");
  return true;
}

void MovieDBView::onCreateATable() {
  QSqlDatabase con = GetSqlVidsDB();
  if (not con.isOpen() and not con.open()) {
    qDebug("Con cannot open");
    return;
  }

  QStringList tbs = con.tables();
  if (not tbs.contains(DB_TABLE::MOVIES)) {
    tbs << DB_TABLE::MOVIES;
  }

  const QString& inputTableName = QInputDialog::getItem(this, "Input an unique table name", "current tables", tbs, tbs.size() - 1, true);
  if (inputTableName.isEmpty() or con.tables().contains(inputTableName)) {
    qDebug("Table name[%s] is empty or already occupied", qPrintable(inputTableName));
    QMessageBox::warning(this, inputTableName, "Table name is empty or already occupied");
    return;
  }

  if (inputTableName.contains(JSON_RENAME_REGEX::INVALID_TABLE_NAME_LETTER)) {
    qDebug("Table name[%s] contains invalid letter", qPrintable(inputTableName));
    QMessageBox::warning(this, inputTableName, "Table name contains invalid letter");
    return;
  }

  // UTF-8 each char takes 1 to 4 byte, 256 chars means 256~1024 bytes
  const QString& createTableSQL = QString(
                                      "CREATE TABLE IF NOT EXISTS `%1`("
                                      "   `%2` NCHAR(256) NOT NULL,"   // Name
                                      "   `%3` INT DEFAULT 0,"         // Size
                                      "   `%4` CHAR(10),"              // Type
                                      "   `%5` CHAR(20),"              // DateModified
                                      "   `%6` NCHAR(40),"             // Performers
                                      "   `%7` CHAR(40),"              // Tags
                                      "   `%8` INT DEFAULT 0,"         // Rate
                                      "   `%9` CHAR(20),"              // Driver
                                      "   `%10` NCHAR(256) NOT NULL,"  // Prepath
                                      "   `%11` CHAR(128),"            // Extra
                                      "   `%12` NCHAR(512) NOT NULL,"  // ForSearch
                                      "    PRIMARY KEY (%2, %10, %12)"
                                      "    );")
                                      .arg(inputTableName)
                                      .arg(DB_HEADER_KEY::Name)
                                      .arg(DB_HEADER_KEY::Size)
                                      .arg(DB_HEADER_KEY::Type)
                                      .arg(DB_HEADER_KEY::DateModified)
                                      .arg(DB_HEADER_KEY::Performers)
                                      .arg(DB_HEADER_KEY::Tags)
                                      .arg(DB_HEADER_KEY::Rate)
                                      .arg(DB_HEADER_KEY::Driver)
                                      .arg(DB_HEADER_KEY::Prepath)
                                      .arg(DB_HEADER_KEY::Extra)
                                      .arg(DB_HEADER_KEY::ForSearch);
  QSqlQuery createTableQuery(con);
  const auto ret = createTableQuery.exec(createTableSQL);
  if (not ret) {
    qDebug("Create table[%s] failed.", qPrintable(inputTableName));
    QMessageBox::warning(this, inputTableName, "Create table[%s] failed");
    return;
  }
  _tables->addItem(inputTableName);
  _tables->setCurrentText(inputTableName);
  qDebug("Table create succeed");
}

bool MovieDBView::onDropATable() {
  QSqlDatabase con = GetSqlVidsDB();
  if (not con.isOpen() and not con.open()) {
    qDebug("con cannot open");
    return false;
  }
  const QStringList& tables = con.tables();
  if (tables.isEmpty()) {
    qDebug("No table find now");
    return true;
  }

  bool okUserSelect = false;
  const QString& dropTableName = QInputDialog::getItem(this, "CONFIRM DROP?", "Choose a table to drop (NOT RECOVERABLE)", tables,
                                                       tables.indexOf(_tables->currentText()), false, &okUserSelect);
  if (not okUserSelect) {
    qDebug("User cancel drop table");
    return true;
  }

  const QString& sqlCmd = QString("DROP TABLE `%1`;").arg(dropTableName);
  QSqlQuery dropQry(con);
  if (not dropQry.exec(sqlCmd)) {
    qDebug("Drop Table[%s] failed. %s", qPrintable(dropTableName), qPrintable(dropQry.lastError().text()));
    QMessageBox::information(this, dropTableName, "Table drop failed");
    return false;
  }
  InitMoviesTables();
  QMessageBox::information(this, dropTableName, "Table has been Dropped");
  return true;
}

bool MovieDBView::onDeleteFromTable(const QString& clause) {
  QSqlDatabase con = GetSqlVidsDB();
  if (not con.isOpen() and not con.open()) {
    qDebug("con cannot open");
    return false;
  }

  const QString& deleteFromTable = _tables->currentText();
  QString whereClause = clause;
  if (clause.isEmpty()) {
    static const QStringList deleteDriverChoicePool{QString("\"%1\"=\"\"").arg(DB_HEADER_KEY::Name), QString("\"%1\"=\"\"").arg(DB_HEADER_KEY::Size),
                                                    QString("\"%1\"=\"\"").arg(DB_HEADER_KEY::Driver),
                                                    QString("\"%1\"=\"\"").arg(DB_HEADER_KEY::Prepath)};

    bool okClicked = false;
    whereClause = QInputDialog::getItem(this, "Delete where", QString("DELETE FROM \"%1\" WHERE").arg(deleteFromTable), deleteDriverChoicePool, 0,
                                        true, &okClicked);
    if (not okClicked or whereClause.isEmpty()) {
      qDebug("User Cancel delete row");
      return true;
    }
  }

  const QString& deleteCmd = QString("DELETE FROM \"%1\" WHERE %2").arg(deleteFromTable, whereClause);
  if (QMessageBox::question(this, "Confirm Delete(NOT recoverable)?", deleteCmd, QMessageBox::Yes | QMessageBox::No, QMessageBox::No) !=
      QMessageBox::Yes) {
    qDebug("User Cancel delete row");
    return true;
  }

  QSqlQuery seleteQry(con);
  const bool deleteRes = seleteQry.exec(deleteCmd);
  qDebug("delete from result: %d", deleteRes);
  _dbModel->submitAll();
  return deleteRes;
}

bool MovieDBView::on_DeleteByDrive() {
  QSet<QString> driversSet;

  for (const auto rowIndex : selectionModel()->selectedRows()) {
    const QString& curDriver = _dbModel->driver(rowIndex);  // TODO:driver
    if (driversSet.contains(curDriver)) {
      continue;
    }
    driversSet.insert(curDriver);

    const QString& whereClause = QString("\"%1\"=\"%2\"").arg(DB_HEADER_KEY::Driver, curDriver);
    const auto ret = onDeleteFromTable(whereClause);
    if (not ret) {
      qDebug("Error when %s", qPrintable(whereClause));
      return false;
    }
  }
  return true;
}

bool MovieDBView::on_DeleteByPrepath() {
  QSet<QString> prepathSet;
  for (const auto rowIndex : selectionModel()->selectedRows()) {
    const QString& prepath = _dbModel->absolutePath(rowIndex);
    if (prepathSet.contains(prepath)) {
      continue;
    }
    prepathSet.insert(prepath);
    const QString& whereClause = QString("\"%1\"=\"%2\"").arg(DB_HEADER_KEY::Prepath, prepath);
    const auto ret = onDeleteFromTable(whereClause);
    if (not ret) {
      qDebug("Error when %s", qPrintable(whereClause));
      return false;
    }
  }
  return true;
}

bool MovieDBView::onInsertIntoTable() {
  QSqlDatabase con = GetSqlVidsDB();
  if (not con.isOpen() and not con.open()) {
    qWarning("con cannot open");
    return false;
  }
  const QString& insertIntoTable = _tables->currentText();
  if (not con.tables().contains(insertIntoTable)) {
    qWarning("Cannot insert into inexist table[%s]", qPrintable(insertIntoTable));
    QMessageBox::warning(this, insertIntoTable, "Table NOT exist. ABORT insert");
    return false;
  }

  QString lastPath = PreferenceSettings().value(MemoryKey::PATH_DB_INSERT_VIDS_FROM.name, MemoryKey::PATH_DB_INSERT_VIDS_FROM.v).toString();
  if (not QFileInfo(lastPath).isDir()) {  // fallback
    lastPath = MemoryKey::PATH_DB_INSERT_VIDS_FROM.v.toString();
  }
  const QString& selectPath =
      QFileDialog::getExistingDirectory(this, "Choose a path into table: " + insertIntoTable, lastPath, QFileDialog::ShowDirsOnly);
  if (selectPath.isEmpty()) {
    qWarning("Path[%s] is not directory", qPrintable(selectPath));
    return false;
  }
  PreferenceSettings().setValue(MemoryKey::PATH_DB_INSERT_VIDS_FROM.name, selectPath);
  if (QMessageBox::question(this, "CONFIRM INSERT?", selectPath + "/* =>" + insertIntoTable) != QMessageBox::StandardButton::Yes) {
    qInfo("User cancel insert[%s]", qPrintable(selectPath));
    return true;
  }

  const QString CURRENT_DRIVE_LETTER = QStorageInfo(selectPath).rootPath();

  const QString& insertTemplate = QString("REPLACE INTO `%1` (%2) VALUES").arg(insertIntoTable).arg(DB_HEADER_KEY::DB_HEADER.join(',')) +
                                  QString("(\"%1\", %2, \"%3\", \"%4\", \"%5\", \"%6\", %7, \"%8\", \"%9\", \"%10\", \"%11\");");

  if (not con.transaction()) {
    qCritical() << "Failed to start transaction mode";
    return 0;
  }
  QSqlQuery insertTableQuery(con);

  int totalItemCnt = 0;
  int succeedItemCnt = 0;
  QDirIterator it(selectPath, TYPE_FILTER::VIDEO_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    const QFileInfo& fi = it.fileInfo();
    const QString& currentInsert = insertTemplate.arg(fi.fileName())
                                       .arg(fi.size())
                                       .arg(fi.suffix())
                                       .arg(fi.lastModified().toString("yyyy/MM/dd HH:mm:ss"))
                                       .arg("")
                                       .arg("")
                                       .arg(0)
                                       .arg(CURRENT_DRIVE_LETTER)
                                       .arg(fi.absolutePath())
                                       .arg("")
                                       .arg(fi.absoluteFilePath());
    const bool insertResult = insertTableQuery.exec(currentInsert);
    succeedItemCnt += int(insertResult);
    if (not insertResult) {
      qWarning("Error [%s]: %s", qPrintable(currentInsert), qPrintable(insertTableQuery.lastError().text()));
    }
    ++totalItemCnt;
  }

  insertTableQuery.finish();
  if (!con.commit()) {
    qDebug() << "Failed to commit, all will be rollback";
    con.rollback();
    succeedItemCnt = 0;
    return false;
  }
  qDebug("%d/%d item(s) add succeed. [%s]", succeedItemCnt, totalItemCnt, qPrintable(selectPath));
  QMessageBox::information(this, QString("%1/%2 succeed").arg(succeedItemCnt).arg(totalItemCnt), selectPath);
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
  _searchLE->setText(where);
  emit _searchLE->returnPressed();
}

int MovieDBView::onCountRow() {
  auto con = GetSqlVidsDB();
  if (not con.isOpen() and not con.open()) {
    qDebug("Cannot open connection");
    return -1;
  }

  QString countCmd = QString("SELECT COUNT(%1) FROM %2").arg(DB_HEADER_KEY::Name).arg(_tables->currentText());
  if (not _searchLE->text().isEmpty()) {
    countCmd += ("WHERE " + _searchLE->text());
  }

  QSqlQuery queryCount(con);
  queryCount.exec(countCmd);
  queryCount.next();

  const int rowCnt = queryCount.value(0).toInt();
  QMessageBox::information(this, countCmd, QString("Count=%1").arg(rowCnt));
  qDebug("%d: %s", rowCnt, qPrintable(countCmd));
  return rowCnt;
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
//     this->setWindowIcon(QIcon(":/themes/SHOW_DATABASE"));
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
