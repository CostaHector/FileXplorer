#ifndef SUBSCRIBEDATABASE_H
#define SUBSCRIBEDATABASE_H
#include <QAction>
#include <QObject>
#include <QWidget>
#include <qDebug>

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

#include "Component/PerformersManagerWidget.h"
#include "MyQSqlTableModel.h"
#include "PublicVariable.h"

auto InitDataBase() -> bool;

class SubscribeDatabase : public QObject {
 public:
  static auto GetSelectionByDriveClause(const QList<QAction*>& selectByDriveActs) -> QString;

  QTableView* view;
  QWidget* contentPane;
  MyQSqlTableModel* dbModel;
  QLineEdit* sqlSearchLE;
  QString currentSearchColumnName;
  T_SwitchStackWidget switchStackWidget;
  QWidget* performerManger;

  explicit SubscribeDatabase(QTableView* view_,
                             MyQSqlTableModel* dbModel_,
                             QLineEdit* sqlSearchLE_,
                             T_SwitchStackWidget switchStackWidget_ = T_SwitchStackWidget(),
                             QWidget* performerManger_ = nullptr)
      : QObject(),
        view(view_),
        dbModel(dbModel_),
        sqlSearchLE(sqlSearchLE_),
        currentSearchColumnName("Name"),
        switchStackWidget(switchStackWidget_),
        performerManger(performerManger_) {
    this->subscribe();
  }

  auto subscribe() -> void;

  auto onCountRow() -> int;

  static auto onInitDataBase() -> bool;

  auto onInitATable() -> void;

  auto onDropATable() -> bool;

  auto onDeleteFromTable(const QString& clause = "") -> bool;
  auto on_DeleteByDrive() -> bool;
  auto on_DeleteByPrepath() -> bool;

  auto onInsertIntoTable() -> bool;
  auto onSearchDataBase(const QString& searchText) -> bool;
  auto onSelectSingleDriver() -> void { this->onSearchDataBase(this->sqlSearchLE->text()); }
  auto onSelectBatch(const QAction* act) -> void;
  auto onShowOrCloseDatabase(const bool isVisible) -> void {
    PreferenceSettings().setValue(MemoryKey::SHOW_DATABASE.name, isVisible);
    switchStackWidget();
  }

  auto onShowOrHidePerformerManger(const bool isVisible) -> void {
    PreferenceSettings().setValue(MemoryKey::SHOW_PERFORMERS_MANGER_DATABASE.name, isVisible);
    if (!performerManger) {
      performerManger = new PerformersManagerWidget(this->view);
    }
    performerManger->setVisible(isVisible);
  }
};

#endif  // SUBSCRIBEDATABASE_H
