#ifndef CASTDBVIEW_H
#define CASTDBVIEW_H

#include "CastBaseDb.h"
#include "CastDbModel.h"
#include "CustomTableView.h"
#include "DatabaseSearchToolBar.h"
#include "QuickWhereClauseDialog.h"
#include <QToolBar>
#include <QLineEdit>

class FileFolderPreviewer;

class CastDBView : public CustomTableView {
public:
  explicit CastDBView(CastDbModel* castDbModel_,
                      CastDatabaseSearchToolBar* castDbSearchBar_,
                      FileFolderPreviewer* floatingPreview_,
                      CastBaseDb& castDb_,
                      QWidget* parent = nullptr);
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
  void RefreshHtmlContents();

  CastDatabaseSearchToolBar* _castDbSearchBar{nullptr};
  CastDbModel* _castModel{nullptr};
  FileFolderPreviewer* _floatingPreview{nullptr};

  const QString mImageHost;
  CastBaseDb& _castDb;
};

#endif  // CASTDBVIEW_H
