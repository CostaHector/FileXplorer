#ifndef EXTRAVIEWVISIBILITYCONTROL_H
#define EXTRAVIEWVISIBILITYCONTROL_H
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

auto InitDataBase() -> bool;

class ExtraViewVisibilityControl : public QObject {
 public:
  explicit ExtraViewVisibilityControl(QWidget* parent);

  auto subscribe() -> void;
  inline auto onShowOrHidePerformerManger(const bool isVisible) -> void;
  inline auto onShowOrHideTorrentsManager(const bool isVisible) -> void;

  QWidget* _parent;
  QWidget* performerManager;
  QWidget* torrentsManager;
};

#endif  // EXTRAVIEWVISIBILITYCONTROL_H
