#ifndef CASTDBVIEW_H
#define CASTDBVIEW_H

#include "CastBaseDb.h"
#include "CustomTableView.h"
#include "DatabaseSearchToolBar.h"
#include <QSqlTableModel>
#include <QToolBar>
#include <QLineEdit>
class FileFolderPreviewer;

class CastDBView : public CustomTableView {
public:
  explicit CastDBView(CastDatabaseSearchToolBar* perfSearchLE, FileFolderPreviewer* floatingPreview, QWidget* parent = nullptr);
  void subscribe();
  QString filePath(const QModelIndex& index) const;

public slots:
  bool onOpenRecordInFileSystem() const;

private:
  void onInitATable();
  int onAppendCasts();
  int onDeleteRecords();

  bool onDropDeleteTable(const DbManager::DROP_OR_DELETE dropOrDelete);
  bool DropSqlDatabase();

  bool onSubmit();

  bool on_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

  int onLoadFromFileSystemStructure();
  int onLoadFromPsonDirectory();

  int onSyncAllImgsFieldFromImageHost();
  int onSyncImgsFieldFromImageHost();

  int onDumpAllIntoPsonFile();
  int onDumpIntoPsonFile();

  int onForceRefreshAllRecordsVids();
  int onForceRefreshRecordsVids();

private:
  static QString GetImageHostPath();
  void RefreshHtmlContents();


  CastDatabaseSearchToolBar* m_perfSearch{nullptr};
  QSqlTableModel* m_castModel{nullptr};
  FileFolderPreviewer* _floatingPreview{nullptr};

  CastBaseDb mDb;
};

#endif  // CASTDBVIEW_H
