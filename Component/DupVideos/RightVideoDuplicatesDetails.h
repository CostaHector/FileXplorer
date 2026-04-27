#ifndef RIGHTVIDEODUPLICATESDETAILS_H
#define RIGHTVIDEODUPLICATESDETAILS_H

#include "CustomTableView.h"
#include "RightVideoDuplicatesModel.h"
#include "DuplicateVideosHelper.h"
#include <QSortFilterProxyModel>

class RightVideoDuplicatesDetails : public CustomTableView {
 public:
  friend class DuplicateVideosFinder;
  explicit RightVideoDuplicatesDetails(QWidget* parent = nullptr);
  bool on_effectiveNameCopiedForEverything(const QModelIndex& ind) const;
  bool on_cellDoubleClicked(const QModelIndex& ind) const;
  bool setSharedMember(GroupedDupVidListArr* pGroupedVidsList, DuplicateVideoDetectionCriteria::DVCriteriaE* pCurDifferType);
  bool onRecycleSelection();
  int onLeftVideoGroupsTableSelectionChanged(int newRow);

 private:
  void subscribe();
  RightVideoDuplicatesModel* m_detailsModel{nullptr};
  QSortFilterProxyModel* m_rightSortProxy{nullptr};
};

#endif  // RIGHTVIDEODUPLICATESDETAILS_H
