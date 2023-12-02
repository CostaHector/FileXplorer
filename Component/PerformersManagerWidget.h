#ifndef PERFORMERSMANAGERWIDGET_H
#define PERFORMERSMANAGERWIDGET_H

#include "Component/PerformersPreviewTextBrowser.h"

#include <QMainWindow>
#include <QToolBar>
#include <QWidget>

#include <QListView>
#include <QTableView>
#include <QTextBrowser>

#include <QHBoxLayout>

#include <QSqlTableModel>

#include <QAction>
#include <QMenu>
#include <QMenuBar>

class PerformersManagerWidget : public QMainWindow {
 public:
  explicit PerformersManagerWidget(QWidget* parent = nullptr);
  auto closeEvent(QCloseEvent* event) -> void override;

  void updateWindowsSize();

  void subscribe();

 signals:

 private:
  QSqlDatabase GetSqlDB() const;

  bool onInitDataBase();
  void onInitATable();
  bool onInsertIntoTable();
  bool onHideThisColumn();
  bool onShowAllColumn();
  int onDeleteRecords();

  void onStretchLastSection(const bool checked);
  void onResizeRowToContents(const bool checked);
  void onResizeRowDefaultSectionSize();

  enum class DROP_OR_DELETE {
    DROP = 0,
    DELETE = 1,
  };

  bool onDropDeleteTable(const DROP_OR_DELETE dropOrDelete);
  bool DropSqlDatabase();

  int onLoadFromPerformersList();

  bool onLocateImageHost();
  bool onChangePerformerImageHeight();
  bool onShowHideColumn();

  bool onSubmit();

  bool on_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

  int onLoadFromFileSystemStructure();
  int onLoadFromPJsonDirectory();
  int onDumpAllIntoPJsonFile();
  int onDumpIntoPJsonFile();
  bool onOpenRecordInFileSystem() const;

  QTableView* m_performersListView;
  PerformersPreviewTextBrowser* m_introductionTextEdit;

  QWidget* m_performerCentralWidget;
  QSqlTableModel* m_perfsDBModel;

  QMenu* m_performerTableMenu;
  QMenu* m_verticalHeaderMenu;
  QMenu* m_horizontalHeaderMenu;

  QString m_imageHostPath;
  int m_performerImageHeight;
  int m_defaultTableRowCount;
  QString m_columnsShowSwitch;

 private:
  bool ShowOrHideColumnCore();
};

#endif  // PERFORMERSMANAGERWIDGET_H
