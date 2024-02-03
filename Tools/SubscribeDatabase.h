#ifndef SUBSCRIBEDATABASE_H
#define SUBSCRIBEDATABASE_H
#include <QAction>
#include <QObject>
#include <QWidget>
#include <QDebug>

#include <QFileSystemModel>
#include <QLineEdit>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QTableView>

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QStorageInfo>

#include "PublicVariable.h"

auto InitDataBase() -> bool;

class SubscribeDatabase : public QObject {
 public:
  static auto GetSelectionByDriveClause(const QList<QAction*>& selectByDriveActs) -> QString;
  explicit SubscribeDatabase(QTableView* view_,
                             T_SwitchStackWidget switchStackWidget_ = T_SwitchStackWidget());

  auto subscribe() -> void;
  inline auto onShowOrCloseDatabase(const bool isVisible) -> void;
  inline auto onShowOrHidePerformerManger(const bool isVisible) -> void;
  inline auto onShowOrHideTorrentsManager(const bool isVisible) -> void;

  QTableView* view;
  T_SwitchStackWidget switchStackWidget;
  QWidget* performerManager;
  QWidget* torrentsManager;
};

#endif  // SUBSCRIBEDATABASE_H
