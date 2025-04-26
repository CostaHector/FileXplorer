#ifndef PERFORMERSWIDGET_H
#define PERFORMERSWIDGET_H

#include "Tools/FileDescriptor/PerfBaseDb.h"
#include "Component/PerformersPreviewTextBrowser.h"
#include "View/PerformersTableView.h"
#include <QSqlTableModel>
#include <QMainWindow>
#include <QToolBar>
#include <QLineEdit>

class PerformersWidget : public QMainWindow {
  Q_OBJECT
 public:
  explicit PerformersWidget(QWidget* parent = nullptr);
  void closeEvent(QCloseEvent* event) override;

  void readSettings();
  void subscribe();

 private:
  void onInitATable();
  bool onInsertIntoTable();
  int onDeleteRecords();

  bool onDropDeleteTable(const DbManager::DROP_OR_DELETE dropOrDelete);
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

 private:
  QLineEdit* m_perfSearch{nullptr};
  QToolBar* m_perfToolbar{nullptr};
  PerformersTableView* m_perfTv{nullptr};
  PerformersPreviewTextBrowser* m_introTE{nullptr};
  QDockWidget* m_perfPrevDock{nullptr};

  QSqlTableModel* m_perfDbMdl{nullptr};

  QString m_imageHostPath;
  int m_performerImageHeight;

  PerfBaseDb mDb;
};

#endif  // PERFORMERSWIDGET_H
