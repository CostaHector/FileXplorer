#ifndef TORRENTSMANAGERWIDGET_H
#define TORRENTSMANAGERWIDGET_H

#include <QMainWindow>

#include <QToolBar>
#include <QWidget>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QSqlTableModel>

#include <QAction>
#include <QMenu>
#include <QMenuBar>

#include <QLineEdit>
#include "CustomTableView.h"
#include "Tools/FileDescriptor/TorrDb.h"

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

  void closeEvent(QCloseEvent* event);
  void updateWindowsSize();

  TorrDb mDb;
  QLineEdit* m_searchLE;
  CustomTableView* m_torrentsListView;
  QWidget* m_torrentsCentralWidget;
  QSqlTableModel* m_torrentsDBModel;
};

#endif  // TORRENTSMANAGERWIDGET_H
