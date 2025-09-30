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

  bool onDropDeleteTable(const DbManagerHelper::DropOrDeleteE dropOrDelete);
  bool DropSqlDatabase();

  bool onModelRepopulate();
  bool onRevert();

  int onLoadFromFileSystemStructure();
  int onLoadFromPsonDirectory();

  int onSyncAllImgsFieldFromImageHost();
  int onSyncImgsFieldFromImageHost();
  int onSyncImgsFieldCore(const QModelIndexList& selectedRowsIndexes);

  int onDumpAllIntoPsonFile();
  int onDumpIntoPsonFile();
  int onDumpIntoCore(const QModelIndexList& selectedRowsIndexes);

  int onRefreshAllVidsField();
  int onRefreshVidsField();
  int onRefreshVidsFieldCore(const QModelIndexList& selectedRowsIndexes);

  void EmitCurrentCastRecordChanged(const QModelIndex &current, const QModelIndex &/*previous*/);
  int onMigrateCastTo();

private:
  void RefreshCurrentRowHtmlContents();

  CastDatabaseSearchToolBar* _castDbSearchBar{nullptr};
  CastDbModel* _castModel{nullptr};

  const QString mImageHost;
  CastBaseDb& _castDb;
  static int QUERY_CONFIRM_IF_ROW_SELECTED_COUNT_ABOVE;
};

#endif  // CASTDBVIEW_H
