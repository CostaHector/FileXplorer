#ifndef TORRENTSMANAGERWIDGET_H
#define TORRENTSMANAGERWIDGET_H

#include <QMainWindow>
#include <QSqlTableModel>
#include <QLineEdit>
#include "CustomTableView.h"
#include "TorrDb.h"
#include <QToolBar>

class TorrentsManagerWidget : public QMainWindow {
 public:
  explicit TorrentsManagerWidget(QWidget* parent = nullptr);

  void subscribe();
  bool onInitDataBase();
  void onInitATable();
  bool onInsertIntoTable();
  bool onDropATable();
  bool onDeleteFromTable();

  bool onSubmit();

  void showEvent(QShowEvent *event) override;
  void closeEvent(QCloseEvent* event) override;
  void updateWindowsSize();

private:
  TorrDb mDb;
  QLineEdit* m_searchLE {nullptr};
  QToolBar* m_searchToobar {nullptr};
  CustomTableView* m_torrentsListView {nullptr};
  QSqlTableModel* m_torrentsDBModel {nullptr};
};

#endif  // TORRENTSMANAGERWIDGET_H
