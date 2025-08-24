#ifndef CASTDBVIEW_H
#define CASTDBVIEW_H

#include "CastBaseDb.h"
#include "CustomTableView.h"
#include <QSqlTableModel>
#include <QToolBar>
#include <QLineEdit>
class FileFolderPreviewer;

class CastDBView : public CustomTableView {
 public:
  explicit CastDBView(QLineEdit* perfSearchLE, FileFolderPreviewer* floatingPreview, QWidget* parent = nullptr);
  void subscribe();

 private:
  void onInitATable();
  bool onInsertIntoTable();
  int onDeleteRecords();

  bool onDropDeleteTable(const DbManager::DROP_OR_DELETE dropOrDelete);
  bool DropSqlDatabase();

  int onLoadFromPerformersList();

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
  static QString GetImageHostPath();

  QLineEdit* m_perfSearch{nullptr};
  QSqlTableModel* m_castModel{nullptr};
  FileFolderPreviewer* _floatingPreview{nullptr};

  CastBaseDb mDb;
};

#endif  // CASTDBVIEW_H
