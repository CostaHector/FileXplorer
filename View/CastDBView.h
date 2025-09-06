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
  Q_OBJECT
public:
  explicit CastDBView(CastDbModel* castDbModel_,
                      CastDatabaseSearchToolBar* castDbSearchBar_,
                      CastBaseDb& castDb_,
                      QWidget* parent = nullptr);
  void subscribe();
signals:
  void currentRecordChanged(const QSqlRecord& newRecord, const QString imageHostPath);

private:
  void onInitATable();
  int onAppendCasts();
  int onDeleteRecords();

  bool onDropDeleteTable(const DbManager::DROP_OR_DELETE dropOrDelete);
  bool DropSqlDatabase();

  bool onSubmit();
  bool onRevert();

  int onLoadFromFileSystemStructure();
  int onLoadFromPsonDirectory();

  int onSyncAllImgsFieldFromImageHost();
  int onSyncImgsFieldFromImageHost();

  int onDumpAllIntoPsonFile();
  int onDumpIntoPsonFile();

  int onForceRefreshAllRecordsVids();
  int onForceRefreshRecordsVids();

  void emitCastCurrentRowSelectionChanged(const QModelIndex &current, const QModelIndex &/*previous*/);
  int onMigrateCastTo();

private:
  void RefreshCurrentRowHtmlContents();

  CastDatabaseSearchToolBar* _castDbSearchBar{nullptr};
  CastDbModel* _castModel{nullptr};

  const QString mImageHost;
  CastBaseDb& _castDb;
};

#endif  // CASTDBVIEW_H
