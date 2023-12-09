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
  T_SwitchStackWidget switchStackWidget;
  QWidget* performerManger;

  explicit SubscribeDatabase(QTableView* view_, T_SwitchStackWidget switchStackWidget_ = T_SwitchStackWidget(), QWidget* performerManger_ = nullptr)
      : QObject(), view(view_), switchStackWidget(switchStackWidget_), performerManger(performerManger_) {
    this->subscribe();
  }

  auto subscribe() -> void;
  inline auto onShowOrCloseDatabase(const bool isVisible) -> void;
  inline auto onShowOrHidePerformerManger(const bool isVisible) -> void;
};

#endif  // SUBSCRIBEDATABASE_H
