#ifndef TORRENTSMANAGERWIDGET_H
#define TORRENTSMANAGERWIDGET_H

#include <QMainWindow>
#include <QSqlTableModel>
#include <QLineEdit>
#include "CustomTableView.h"
#include "TorrDb.h"

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

  TorrDb mDb;
  QLineEdit* m_searchLE;
  CustomTableView* m_torrentsListView;
  QWidget* m_torrentsCentralWidget;
  QSqlTableModel* m_torrentsDBModel;
};

#endif  // TORRENTSMANAGERWIDGET_H
