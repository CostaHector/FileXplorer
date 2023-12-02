#include "SubscribeDatabase.h"
#include "Actions/DataBaseActions.h"
#include "Actions/PerformersManagerActions.h"
#include "PublicTool.h"
#include "PublicVariable.h"

#include <QAbstractItemView>
#include <QMainWindow>
#include <QSqlError>
#include <QSqlQuery>

auto SubscribeDatabase::GetSelectionByDriveClause(const QList<QAction*>& selectByDriveActs) -> QString {
  QStringList clauseLst;
  for (QAction* act : selectByDriveActs) {
    if (act->isChecked()) {
      clauseLst.append(QString("Driver=\"%1\"").arg(act->text()));
    }
  }
  if (clauseLst.isEmpty()) {  // select none driver
    return "0";
  }
  if (clauseLst.size() == selectByDriveActs.size()) {  // select all driver
    return "";
  }
  return clauseLst.join(" OR ");
}

void SubscribeDatabase::subscribe() {
  connect(g_dbAct().DRIVE_BATCH_SELECTION_AG, &QActionGroup::triggered, this, &SubscribeDatabase::onSelectBatch);
  connect(g_dbAct().DRIVE_SEPERATE_SELECTION_AG, &QActionGroup::triggered, this, &SubscribeDatabase::onSelectSingleDriver);

  connect(this->sqlSearchLE, &QLineEdit::returnPressed, this, [this]() {
    const QString& searchPattern = this->sqlSearchLE->text();
    this->onSearchDataBase(searchPattern);
  });

  connect(g_dbAct().DB_VIEW_CLOSE_SHOW, &QAction::triggered, this, &SubscribeDatabase::onShowOrCloseDatabase);
  connect(g_performersManagerActions().SHOW_PERFORMER_MANAGER, &QAction::triggered, this, &SubscribeDatabase::onShowOrHidePerformerManger);
  if (g_performersManagerActions().SHOW_PERFORMER_MANAGER->isChecked()) {
    onShowOrHidePerformerManger(true);
  }

  {
    const QList<QAction*>& DB_CONTROL_ACTIONS = g_dbAct().DB_CONTROL_ACTIONS->actions();

    QAction* INSERT_A_PATH = DB_CONTROL_ACTIONS[0];
    QAction* DELETE_FROM_TABLE = DB_CONTROL_ACTIONS[1];
    QAction* INIT_A_DATABASE = DB_CONTROL_ACTIONS[2];
    QAction* INIT_A_TABLE = DB_CONTROL_ACTIONS[3];
    QAction* DROP_A_DATABASE = DB_CONTROL_ACTIONS[4];
    QAction* DROP_A_TABLE = DB_CONTROL_ACTIONS[5];

    connect(INIT_A_DATABASE, &QAction::triggered, this, &SubscribeDatabase::onInitDataBase);
    connect(DROP_A_DATABASE, &QAction::triggered, this, [this]() { QMessageBox::warning(this->view, "Too danger", "Cancel drop database"); });

    connect(INIT_A_TABLE, &QAction::triggered, this, &SubscribeDatabase::onInitATable);
    connect(DROP_A_TABLE, &QAction::triggered, this, &SubscribeDatabase::onDropATable);
    connect(INSERT_A_PATH, &QAction::triggered, this, &SubscribeDatabase::onInsertIntoTable);
    connect(DELETE_FROM_TABLE, &QAction::triggered, this, [this]() { this->onDeleteFromTable(); });
  }

  {
    const QList<QAction*>& DB_FUNCTIONS_ACTIONS = g_dbAct().DB_FUNCTIONS->actions();
    QAction* COUNT = DB_FUNCTIONS_ACTIONS[0];
    QAction* AVG = DB_FUNCTIONS_ACTIONS[1];
    QAction* SUM = DB_FUNCTIONS_ACTIONS[2];
    connect(COUNT, &QAction::triggered, this, &SubscribeDatabase::onCountRow);
  }
  {
    const QList<QAction*>& DB_RIGHT_CLICK_MENU_AG = g_dbAct().DB_RIGHT_CLICK_MENU_AG->actions();
    auto* DELETE_BY_DRIVER = DB_RIGHT_CLICK_MENU_AG[3];
    auto* DELETE_BY_PREPATH = DB_RIGHT_CLICK_MENU_AG[4];
    connect(DELETE_BY_DRIVER, &QAction::triggered, this, &SubscribeDatabase::on_DeleteByDrive);
    connect(DELETE_BY_PREPATH, &QAction::triggered, this, &SubscribeDatabase::on_DeleteByPrepath);
  }
}

int SubscribeDatabase::onCountRow() {
  auto con = GetSqlDB();
  if (not con.isOpen()) {
    qDebug("Cannot open connection");
    return -1;
  }
  const QString& countCmd = QString("SELECT COUNT(%1) FROM %2;").arg(DB_HEADER_KEY::Name).arg(TABLE_NAME);
  QSqlQuery queryCount(con);
  queryCount.exec(countCmd);
  queryCount.next();
  const int rowCnt = queryCount.value(0).toInt();
  QMessageBox::information(view, countCmd, QString("%1").arg(rowCnt));
  return rowCnt;
}

bool SubscribeDatabase::onInitDataBase() {
  QSqlDatabase con = GetSqlDB();
  if (not con.isOpen()) {
    qDebug("con cannot open");
    return false;
  }
  qDebug("Database create succeed");
  return true;
}

void SubscribeDatabase::onInitATable() {
  QSqlDatabase con = GetSqlDB();
  if (not con.isOpen()) {
    qDebug("con cannot open");
    return;
  }

  if (con.tables().contains(TABLE_NAME)) {
    qDebug("Table[%s] already exists in database[%s]", TABLE_NAME.toStdString().c_str(), con.databaseName().toStdString().c_str());
    return;
  }

  // UTF-8 each char takes 1 to 4 byte, 256 chars means 256~1024 bytes
  const QString& createTableSQL = QString(
                                      "CREATE TABLE IF NOT EXISTS `%1`("
                                      "   `%2` TEXT NOT NULL,"
                                      "   `%3` INT,"
                                      "   `%4` TEXT NOT NULL,"
                                      "   `%5` TEXT,"
                                      "   `%6` TEXT,"
                                      "   `%7` TEXT,"
                                      "   `%8` INT,"
                                      "   `%9` TEXT,"
                                      "   `%10` TEXT NOT NULL,"
                                      "   `%11` TEXT,"
                                      "    PRIMARY KEY (%9, %10, "
                                      "                %2, %4,"
                                      "                %6, %7)"
                                      "    );")
                                      .arg(TABLE_NAME)
                                      .arg(DB_HEADER_KEY::Name)
                                      .arg(DB_HEADER_KEY::Size)
                                      .arg(DB_HEADER_KEY::Type)
                                      .arg(DB_HEADER_KEY::DateModified)
                                      .arg(DB_HEADER_KEY::Performers)
                                      .arg(DB_HEADER_KEY::Tags)
                                      .arg(DB_HEADER_KEY::Rate)
                                      .arg(DB_HEADER_KEY::Driver)
                                      .arg(DB_HEADER_KEY::Prepath)
                                      .arg(DB_HEADER_KEY::Extra);
  QSqlQuery createTableQuery(con);
  const auto ret = createTableQuery.exec(createTableSQL);
  if (not ret) {
    qDebug("Create table[%s] failed.", TABLE_NAME.toStdString().c_str());
    return;
  }
  this->dbModel->setTable(TABLE_NAME);
  this->dbModel->submitAll();
  qDebug("Table create succeed");
}

bool SubscribeDatabase::onDropATable() {
  QSqlDatabase con = GetSqlDB();
  if (not con.isOpen()) {
    qDebug("con cannot open");
    return false;
  }

  if (not con.tables().contains(TABLE_NAME)) {
    qDebug("Table[%s] not exists", TABLE_NAME.toStdString().c_str());
    return true;
  }

  QSqlQuery dropQry(con);
  const auto dropTableRet = dropQry.exec(QString("DROP TABLE `%1`;").arg(TABLE_NAME));
  if (not dropTableRet) {
    qDebug("Drop Table[%s] failed. %s", TABLE_NAME.toStdString().c_str(), con.lastError().databaseText().toStdString().c_str());
  }
  dropQry.finish();
  this->dbModel->submitAll();
  return dropTableRet;
}

bool SubscribeDatabase::onDeleteFromTable(const QString& clause) {
  QSqlDatabase con = GetSqlDB();
  if (not con.isOpen()) {
    qDebug("con cannot open");
    return false;
  }

  QStringList deleteDriverChoicePool;
  for (const QAction* act : g_dbAct().DRIVE_SEPERATE_SELECTION_AG->actions()) {
    deleteDriverChoicePool.append(QString("\"%1\"=\"%2\"").arg(DB_HEADER_KEY::Driver, act->text()));
  }
  deleteDriverChoicePool.append(QString("\"%1\"=\"\"").arg(DB_HEADER_KEY::Prepath));

  if (deleteDriverChoicePool.isEmpty()) {
    qDebug("Cancel [No need to delete rows from drive]");
    return true;
  }
  QString whereClause = clause;
  if (clause.isEmpty()) {
    bool okClicked = false;
    whereClause =
        QInputDialog::getItem(this->view, "Where clause", "DELETE FROM \"%1\" WHERE \t\t\t\t\t\t\t\t", deleteDriverChoicePool, 0, true, &okClicked);
    if (not okClicked or whereClause.isEmpty()) {
      qDebug("Cancel");
      return false;
    }
  }
  const QString& deleteCmd = QString("DELETE FROM \"%1\" WHERE %2").arg(TABLE_NAME, whereClause);

  QSqlQuery seleteQry(con);
  const auto deleteRes = seleteQry.exec(deleteCmd);
  qDebug("delete from result %d", deleteRes);
  this->dbModel->submitAll();
  return deleteRes;
}

bool SubscribeDatabase::on_DeleteByDrive() {
  const QList<QAction*>& DB_CONTROL_ACTIONS = g_dbAct().DB_CONTROL_ACTIONS->actions();
  QAction* DELETE_FROM_TABLE = DB_CONTROL_ACTIONS[1];

  QSet<QString> driversSet;
  for (const auto rowIndex : view->selectionModel()->selectedRows()) {
    const QString& curDriver = dbModel->driver(rowIndex);
    if (driversSet.contains(curDriver)) {
      continue;
    }
    driversSet.insert(curDriver);

    const QString& whereClause = QString("\"%1\"=\"%2\"").arg(DB_HEADER_KEY::Driver, curDriver);
    const auto ret = onDeleteFromTable(whereClause);
    if (not ret) {
      qDebug("Error when %s", whereClause.toStdString().c_str());
      return false;
    }
  }
  return true;
}

bool SubscribeDatabase::on_DeleteByPrepath() {
  const QList<QAction*>& DB_CONTROL_ACTIONS = g_dbAct().DB_CONTROL_ACTIONS->actions();
  QAction* DELETE_FROM_TABLE = DB_CONTROL_ACTIONS[1];

  QSet<QString> prepathSet;
  for (const auto rowIndex : view->selectionModel()->selectedRows()) {
    const QString& prepath = dbModel->absolutePath(rowIndex);
    if (prepathSet.contains(prepath)) {
      continue;
    }
    prepathSet.insert(prepath);
    const QString& whereClause = QString("\"%1\"=\"%2\"").arg(DB_HEADER_KEY::Prepath, prepath);
    const auto ret = onDeleteFromTable(whereClause);
    if (not ret) {
      qDebug("Error when %s", whereClause.toStdString().c_str());
      return false;
    }
  }
  return true;
}

bool SubscribeDatabase::onInsertIntoTable() {
  QSqlDatabase con = GetSqlDB();
  if (not con.isOpen()) {
    qDebug("con cannot open");
    return false;
  }
  if (not con.tables().contains(TABLE_NAME)) {
    const QString& tablesNotExistsMsg = QString("Cannot insert, table[%1] not exist.").arg(TABLE_NAME);
    qDebug("%s", tablesNotExistsMsg.toStdString().c_str());
    QMessageBox::warning(view, "Insert abort", tablesNotExistsMsg);
    return false;
  }

  const QString& selectPath = QFileDialog::getExistingDirectory(view, "Choose A Path");
  if (selectPath.isEmpty()) {
    qDebug("Path[%s] is not directory", selectPath.toStdString().c_str());
    return false;
  }

  auto* msgBox = new QMessageBox;
  msgBox->setWindowTitle(QString("INSERT INTO %1 () VALUES ();").arg(TABLE_NAME));
  msgBox->setText(QString("items in %1 will be added to").arg(selectPath));
  msgBox->setInformativeText("Do you sure to add them?");
  msgBox->setStandardButtons(QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel);
  msgBox->setDefaultButton(QMessageBox::StandardButton::Cancel);
  const auto ret = msgBox->exec();
  if (ret != QMessageBox::StandardButton::Ok) {
    qDebug("Cancel [%s]", msgBox->text().toStdString().c_str());
    return true;
  }
  const QString& insertTemplate = QString("INSERT INTO `%1` (%2) VALUES").arg(TABLE_NAME).arg(DB_HEADER_KEY::DB_HEADER.join(',')) +
                                  QString("(\"%1\", %2, \"%3\", \"%4\", \"%5\", \"%6\", %7, \"%8\", \"%9\", \"%10\");");

  int totalItemCnt = 0;
  int succeedItemCnt = 0;
  QDirIterator it(selectPath, TYPE_FILTER::VIDEO_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  const QString CURRENT_DRIVE_LETTER = QStorageInfo(selectPath).rootPath();

  QSqlQuery insertTableQuery(con);
  while (it.hasNext()) {
    it.next();
    const QFileInfo& fi = it.fileInfo();
    const QString& currentInsert = insertTemplate.arg(fi.fileName())
                                       .arg(fi.size())
                                       .arg(fi.suffix())
                                       .arg(fi.lastModified().toString("yyyy-MM-dd HH:mm:ss.zzz"))
                                       .arg("")
                                       .arg("")
                                       .arg(0)
                                       .arg(CURRENT_DRIVE_LETTER)
                                       .arg(fi.absolutePath())
                                       .arg("");
    const bool insertResult = insertTableQuery.exec(currentInsert);
    succeedItemCnt += int(insertResult);
    if (not insertResult) {
      qDebug("Insert Error: %s", insertTableQuery.lastError().databaseText().toStdString().c_str());
    }
    ++totalItemCnt;
  }
  const QString& msg = QString("%1/%2 item(s) add succeed.").arg(succeedItemCnt).arg(totalItemCnt);
  qDebug("%s", msg.toStdString().c_str());
  insertTableQuery.finish();
  QMessageBox::information(view, "insert result", msg.toStdString().c_str());
  this->dbModel->submitAll();
  return true;
}

bool SubscribeDatabase::onSearchDataBase(const QString& searchText) {
  const QString& driverWhereClause = this->GetSelectionByDriveClause(g_dbAct().DRIVE_SEPERATE_SELECTION_AG->actions());

  if (driverWhereClause == "0") {
    dbModel->setFilter("0");
    return true;
  }
  QStringList conditionGroup;

  if (not driverWhereClause.isEmpty()) {
    conditionGroup << driverWhereClause;
  }
  if (not searchText.isEmpty()) {
    conditionGroup << searchText;
  }
  if (conditionGroup.isEmpty()) {
    dbModel->setFilter("");
    return true;
  }
  const QString& whereClause = conditionGroup.join(" AND ");
  dbModel->setFilter(whereClause);
  return true;
}

void SubscribeDatabase::onSelectBatch(const QAction* act) {
  bool changed = false;
  if (act->text() == "all") {
    for (auto* driveAct : g_dbAct().DRIVE_SEPERATE_SELECTION_AG->actions()) {
      if (not driveAct->isChecked()) {
        changed = true;
        driveAct->setChecked(true);
      }
    }
  } else if (act->text() == "none") {
    for (auto* driveAct : g_dbAct().DRIVE_SEPERATE_SELECTION_AG->actions()) {
      if (driveAct->isChecked()) {
        changed = true;
        driveAct->setChecked(false);
      }
    }
  } else if (act->text() == "online") {
    for (auto* driveAct : g_dbAct().DRIVE_SEPERATE_SELECTION_AG->actions()) {
      const bool exiDir = QFile::exists(act->text());
      if (driveAct->isChecked() != exiDir) {
        changed = true;
        driveAct->setChecked(exiDir);
      }
    }
  }
  if (not changed) {
    qDebug("[Normal] No need to change model filter(batch selection checked same).");
    return;
  }
  this->onSearchDataBase(this->sqlSearchLE->text());
}
