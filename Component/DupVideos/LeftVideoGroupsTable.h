#ifndef LEFTVIDEOGROUPSTABLE_H
#define LEFTVIDEOGROUPSTABLE_H

#include "CustomTableView.h"
#include "DuplicateVideosHelper.h"
#include "LeftVideoGroupsModel.h"
#include <QSortFilterProxyModel>

class LeftVideoGroupsTable : public CustomTableView {
  Q_OBJECT
 public:
  friend class DuplicateVideosFinder;
  explicit LeftVideoGroupsTable(QWidget* parent = nullptr);
  const QString GetCurrentDupVideoGroupInfo() const;

  void setDifferType(const DuplicateVideoDetectionCriteria::DVCriteriaE& newDifferType);
  void setDeviationDuration(int newDuration);
  void setDeviationSize(qint64 newSize);
  int GetCurSrcIndexRow(const QItemSelection& selected) const;

 signals:
  // curRow >=0: valid, otherwise invalid
  void leftSelectionChanged(int curRow = INVALID_LEFT_SELECTED_ROW);

 public slots:
  int onDuplicateVideosListChanged(const DupVidMetaInfoList& needAnalyzeVidLst);

 private:
  void subscribe();
  LeftVideoGroupsModel* m_leftGrpModel{nullptr};
  QSortFilterProxyModel* m_leftSortProxy{nullptr};
};

#endif  // LEFTVIDEOGROUPSTABLE_H
