#ifndef PERFORMERSWIDGET_H
#define PERFORMERSWIDGET_H

#include "Component/PerformersPreviewTextBrowser.h"
#include "View/PerformersTableView.h"

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

class PerformersWidget : public QMainWindow {
  Q_OBJECT
 public:
  explicit PerformersWidget(QWidget* parent = nullptr);
  auto closeEvent(QCloseEvent* event) -> void override;

  void updateWindowsSize();
  void subscribe();

 signals:

 private:
  QSqlDatabase GetSqlDB() const;

  bool onInitDataBase();
  void onInitATable();
  bool onInsertIntoTable();
  int onDeleteRecords();

  enum class DROP_OR_DELETE {
    DROP = 0,
    DELETE = 1,
  };

  bool onDropDeleteTable(const DROP_OR_DELETE dropOrDelete);
  bool DropSqlDatabase();

  int onLoadFromPerformersList();

  bool onLocateImageHost();
  bool onChangePerformerImageHeight();

  bool onSubmit();

  bool on_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

  int onLoadFromFileSystemStructure();
  int onLoadFromPJsonDirectory();
  int onDumpAllIntoPJsonFile();
  int onDumpIntoPJsonFile();
  int onForceRefreshAllRecordsVids();
  int onForceRefreshRecordsVids();
  bool onOpenRecordInFileSystem() const;

  PerformersTableView* m_performersListView;
  PerformersPreviewTextBrowser* m_introductionTextEdit;
  QDockWidget* performerPreviewDock;
  QSqlTableModel* m_perfsDBModel;

  QString m_imageHostPath;
  int m_performerImageHeight;
};

#endif  // PERFORMERSWIDGET_H
