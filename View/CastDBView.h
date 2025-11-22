#ifndef CASTDBVIEW_H
#define CASTDBVIEW_H

#include "CastBaseDb.h"
#include "CastDbModel.h"
#include "CustomTableView.h"
#include "DatabaseSearchToolBar.h"
#include "QuickWhereClauseDialog.h"
#include <QToolBar>
#include <QLineEdit>

#ifdef RUNNING_UNIT_TESTS
namespace CastDbViewMocker {
inline std::pair<bool, QString>& MockMultiLineInput() {
  static std::pair<bool, QString> accept2MultiLine;
  return accept2MultiLine;
}
inline bool& MockDeleteRecord() {
  static bool acceptDelete = false;
  return acceptDelete;
}
inline bool& MockDropDeleteTable() {
  static bool acceptDropDelete = false;
  return acceptDropDelete;
}
inline bool& MockLoadFromPsonDirectory() {
  static bool acceptLoadPson = false;
  return acceptLoadPson;
}
inline bool& MockRefreshVidsField() {
  static bool acceptRefreshVidsField = false;
  return acceptRefreshVidsField;
}
inline std::pair<QString, QString>& MockMovieDbAbsFilePath2ConnName() {
  static std::pair<QString, QString> movieDatabasePath2ConnName;
  return movieDatabasePath2ConnName;
}
inline QString& MockMigrateToPath() {
  static QString migrateToPath;
  return migrateToPath;
}
}  // namespace CastDbViewMocker
#endif

struct IndexRecoverHelper {
public:
  void stash(const QModelIndex& oldIndex) { mNeedRecover = oldIndex.isValid(); mOldIndex = oldIndex;}
  void stashPop(QAbstractItemView& itemView, const QModelIndex& currentIndex);

private:
  QModelIndex mOldIndex;
  bool mNeedRecover{false};
};

class FileFolderPreviewer;

class CastDBView : public CustomTableView {
  Q_OBJECT
 public:
  explicit CastDBView(CastDbModel* castDbModel_, CastDatabaseSearchToolBar* castDbSearchBar_, CastBaseDb& castDb_, QWidget* parent = nullptr);
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
  bool onModelSubmitAll();

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

  void EmitCurrentCastRecordChanged(const QModelIndex& current, const QModelIndex& /*previous*/);

  static void setQueryConfirmIfRowSelectedCountAbove(int newValue);
 private:
  void RefreshCurrentRowHtmlContents();

  CastDatabaseSearchToolBar* _castDbSearchBar{nullptr};
  CastDbModel* _castModel{nullptr};

  const QString mImageHost;
  CastBaseDb& _castDb;
  static int QUERY_CONFIRM_IF_ROW_SELECTED_COUNT_ABOVE;

  IndexRecoverHelper mIndexRecover;
};

#endif  // CASTDBVIEW_H
