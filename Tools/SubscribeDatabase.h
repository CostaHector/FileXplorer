#ifndef SUBSCRIBEDATABASE_H
#define SUBSCRIBEDATABASE_H

#include <QAction>
#include <QObject>
#include <QWidget>
#include "Actions/DataBaseActions.h"
#include "qdebug.h"

#include <QFileSystemModel>
#include <QLineEdit>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QTableView>

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

#include <QDateTime>
#include <QStorageInfo>

typedef std::function<void()> T_HotUpdate;

inline auto GetSqlDB() -> QSqlDatabase {
  QSqlDatabase con;
  if (QSqlDatabase::connectionNames().contains("custom_connection")) {
    con = QSqlDatabase::database("custom_connection", false);
  } else {
    con = QSqlDatabase::addDatabase("QSQLITE", "custom_connection");
  }
  con.setDatabaseName(SystemPath::FILE_INFO_DATABASE);
  con.open();
  return con;
}

auto InitDataBase() -> bool;
auto InitATable() -> bool;
auto InsertIntoTable(const QString& path) -> bool;

class SubscribeDatabase : public QObject {
 public:
  static auto GetSelectionByDriveClause(const QList<QAction*>& selectByDriveActs) -> QString;

  QTableView* view;
  QWidget* contentPane;
  QSqlTableModel* dbModel;
  QLineEdit* sqlSearchLE;
  QString currentSearchColumnName;
  T_HotUpdate hotUpdate;

  explicit SubscribeDatabase(QTableView* view_, QSqlTableModel* dbModel_, QLineEdit* sqlSearchLE_, T_HotUpdate hotUpdate_ = T_HotUpdate())
      : QObject(), view(view_), dbModel(dbModel_), sqlSearchLE(sqlSearchLE_), currentSearchColumnName("Name"), hotUpdate(hotUpdate_) {
    this->subscribe();
  }

  auto subscribe() -> void {
    connect(g_dbAct().DRIVE_BATCH_SELECTION_AG, &QActionGroup::triggered, this, &SubscribeDatabase::onSelectBatch);
    connect(g_dbAct().DRIVE_SEPERATE_SELECTION_AG, &QActionGroup::triggered, this, &SubscribeDatabase::onSelectSingleDriver);
    connect(this->sqlSearchLE, &QLineEdit::returnPressed, [this]() {
      const QString& searchPattern = this->sqlSearchLE->text();
      this->onSearchDataBase(searchPattern);
    });
    connect(g_dbAct().DB_VIEW_CLOSE_SHOW, &QAction::triggered, this, &SubscribeDatabase::onShowOrCloseDatabase);

    {
      const QList<QAction*>& DB_CONTROL_ACTIONS = g_dbAct().DB_CONTROL_ACTIONS->actions();
      QAction* InsertAPath = DB_CONTROL_ACTIONS[0];
      QAction* deleteFromTableDrive = DB_CONTROL_ACTIONS[1];
      QAction* InitADatabase = DB_CONTROL_ACTIONS[2];
      QAction* InitATable = DB_CONTROL_ACTIONS[3];
      QAction* DropADatabase = DB_CONTROL_ACTIONS[4];
      QAction* DropATable = DB_CONTROL_ACTIONS[5];

      connect(InitADatabase, &QAction::triggered, this, InitDataBase);
      connect(DropADatabase, &QAction::triggered, []() { qDebug("TODO Too danger."); });

      connect(InitATable, &QAction::triggered, this, &SubscribeDatabase::onInitATable);
      connect(DropATable, &QAction::triggered, this, &SubscribeDatabase::onDropATable);
      connect(InsertAPath, &QAction::triggered, this, &SubscribeDatabase::onInsertIntoTable);
      connect(deleteFromTableDrive, &QAction::triggered, this, &SubscribeDatabase::onDeleteFromTableDrive);
    }

    {
      const QList<QAction*>& DB_FUNCTIONS_ACTIONS = g_dbAct().DB_FUNCTIONS->actions();
      QAction* COUNT = DB_FUNCTIONS_ACTIONS[0];
      QAction* AVG = DB_FUNCTIONS_ACTIONS[1];
      QAction* SUM = DB_FUNCTIONS_ACTIONS[2];
      connect(COUNT, &QAction::triggered, this, &SubscribeDatabase::onCountRow);
    }
  }

  auto onCountRow() -> int {
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

  auto onInitATable() -> void {
    InitATable();
    this->dbModel->submitAll();
    qDebug("Table create succeed");
  }
  auto onDropATable() -> bool {
    QSqlDatabase con = GetSqlDB();
    if (not con.isOpen()) {
      qDebug("con cannot open");
      return false;
    }

    QSqlQuery dropQry(con);
    qDebug() << con.tables();

    const auto dropTableRet = dropQry.exec(QString("DROP TABLE `%1`;").arg(TABLE_NAME));
    if (not dropTableRet) {
      qDebug("Drop Table[%s] failed. %s", TABLE_NAME.toStdString().c_str(), con.lastError().databaseText().toStdString().c_str());
    }
    dropQry.finish();
    this->dbModel->submitAll();
    return dropTableRet;
  }
  auto onDeleteFromTableDrive() -> bool {
    QSqlDatabase con = GetSqlDB();
    if (not con.isOpen()) {
      qDebug("con cannot open");
      return false;
    }

    const QString& mainCmd = QString("DELETE FROM \"%1\"").arg(TABLE_NAME);
    const QString& whereClause = "\"Driver\"=\"E:/\"";
    const QString& deleteCmd = QString("%1 WHERE %2;").arg(mainCmd, whereClause);

    QStringList deleteDriverChoicePool;
    for (const QAction* act : g_dbAct().DRIVE_SEPERATE_SELECTION_AG->actions()) {
      deleteDriverChoicePool.append(act->text());
    }
    if (deleteDriverChoicePool.isEmpty()) {
      qDebug("Abort [Nothing Drive need to delete from]");
      return true;
    }
    bool okClicked = false;
    const QString& driverColumnText = QInputDialog::getItem(this->view, QString("DELETE FROM \"%1\" WHERE").arg(TABLE_NAME),
                                                            "\"Driver\"=\t\t\t\t\t\t\t\t\t\t\t\t", deleteDriverChoicePool, 0, false, &okClicked);
    if (not okClicked or driverColumnText.isEmpty()) {
      qDebug("Abort [%s]", deleteCmd.toStdString().c_str());
      return false;
    }
    QSqlQuery seleteQry(con);
    const auto deleteRes = seleteQry.exec(deleteCmd);
    qDebug("delete from result %d", deleteRes);
    this->dbModel->submitAll();
    return deleteRes;
  }
  auto onInsertIntoTable() -> bool {
    const QString& selectPath = QFileDialog::getExistingDirectory(view, "Choose A Path");
    QFileInfo dstFi(selectPath);
    if (selectPath.isEmpty() or not dstFi.isDir()) {
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
      qDebug("Abort [%s]", msgBox->text().toStdString().c_str());
      return true;
    }
    const auto insertRet = InsertIntoTable(dstFi.absoluteFilePath());
    if (not insertRet){
      QMessageBox::warning(view, "insert error", "please check the debug log");
      return false;
    }
    this->dbModel->submitAll();
    return insertRet;
  }
  auto onSearchDataBase(const QString& searchText) -> bool {
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
  auto onSelectSingleDriver(const QAction* dummyAct = nullptr) -> void { this->onSearchDataBase(this->sqlSearchLE->text()); }
  auto onSelectBatch(const QAction* act) -> void {
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
  auto onShowOrCloseDatabase(const bool isVisible) -> void {
    PreferenceSettings().setValue(MemoryKey::SHOW_DATABASE.name, isVisible);
    if (not hotUpdate){
      if (view->isVisible() != isVisible){
        view->setVisible(isVisible);
      }
      return;
    }
    hotUpdate();
  }
};

#endif  // SUBSCRIBEDATABASE_H
