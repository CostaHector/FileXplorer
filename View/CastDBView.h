#ifndef CASTDBVIEW_H
#define CASTDBVIEW_H

#include "PerfBaseDb.h"
#include "CustomTableView.h"
#include <QSqlTableModel>
#include <QToolBar>
#include <QLineEdit>
class FloatingPreview;

class CastDBView : public CustomTableView {
 public:
  explicit CastDBView(QLineEdit* perfSearchLE, FloatingPreview* floatingPreview, QWidget* parent = nullptr);
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
  QSqlTableModel* m_perfDbMdl{nullptr};
  FloatingPreview* _floatingPreview{nullptr};
  QString m_imageHostPath;
  int m_performerImageHeight;

  PerfBaseDb mDb;
};

#endif  // CASTDBVIEW_H
