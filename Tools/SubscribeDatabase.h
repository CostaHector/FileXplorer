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

#include "PublicVariable.h"

auto InitDataBase() -> bool;

class SubscribeDatabase : public QObject {
 public:
  static auto GetSelectionByDriveClause(const QList<QAction*>& selectByDriveActs) -> QString;

  QTableView* view;
  T_SwitchStackWidget switchStackWidget;
  QWidget* performerManager;
  QWidget* torrentsManager;

  explicit SubscribeDatabase(QTableView* view_,
                             T_SwitchStackWidget switchStackWidget_ = T_SwitchStackWidget(),
                             QWidget* performerManger_ = nullptr,
                             QWidget* torrentsManager_ = nullptr)
      : QObject(), view(view_), switchStackWidget(switchStackWidget_), performerManager(performerManger_), torrentsManager(torrentsManager_) {
    this->subscribe();
  }

  auto subscribe() -> void;
  inline auto onShowOrCloseDatabase(const bool isVisible) -> void;
  inline auto onShowOrHidePerformerManger(const bool isVisible) -> void;
  inline auto onShowOrHideTorrentsManager(const bool isVisible) -> void;
};

#endif  // SUBSCRIBEDATABASE_H
