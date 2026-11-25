#ifndef CASTDBTABLEVIEW_H
#define CASTDBTABLEVIEW_H

#include "CustomTableView.h"

class QSqlRecord;
class CastDbModel;
class CastDBTableView : public CustomTableView {
  Q_OBJECT
public:
  explicit CastDBTableView(CastDbModel* castDbModel_, QItemSelectionModel* selectionModel_, QWidget* parent = nullptr);
  bool onRevertAll();
  void RefreshCurrentRowHtmlContents();
  bool EmitCurrentCastRecordChangedCore(const QSqlRecord& record);

signals:
  void currentRecordChanged(const QSqlRecord& newRecord, const QString imageHostPath);

private:
  void subscribe();

  int onInsertARowBeforeCurrentIndex();
  int onDeleteRecords();

  bool onSubmitAll();

  int onRefreshAllImgsField();
  int onRefreshImgsField();
  int onRefreshImgsFieldCore(const QModelIndexList& selectedRowsIndexes);
  static int onRefreshImgsFieldCoreStatic(CastDbModel& model, const QModelIndexList& selectedRowsIndexes);

  int onRefreshAllVidsField();
  int onRefreshVidsField();
  int onRefreshVidsFieldCore(const QModelIndexList& selectedRowsIndexes);
  static int onRefreshVidsFieldCoreStatic(CastDbModel& model, const QModelIndexList& selectedRowsIndexes);

  int onDumpAllIntoPsonFile();
  int onDumpIntoPsonFile();
  int onDumpIntoCore(const QModelIndexList& selectedRowsIndexes);

  bool EmitCurrentCastRecordChanged(const QModelIndex& current, const QModelIndex& /*previous*/);

  CastDbModel* _castModel{nullptr};

  static int QUERY_CONFIRM_IF_ROW_SELECTED_COUNT_ABOVE;
};

#endif  // CASTDBTABLEVIEW_H
