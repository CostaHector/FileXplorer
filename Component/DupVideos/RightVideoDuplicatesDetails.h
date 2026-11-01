#ifndef RIGHTVIDEODUPLICATESDETAILS_H
#define RIGHTVIDEODUPLICATESDETAILS_H

#include "CustomTableView.h"
#include "RightVideoDuplicatesModel.h"
#include "DuplicateVideosHelper.h"
#include <QSortFilterProxyModel>

class AdvanceRenamer;
class RightVideoDuplicatesDetails : public CustomTableView {
 public:
  friend class DuplicateVideosFinder;
  explicit RightVideoDuplicatesDetails(QWidget* parent = nullptr);
  bool on_effectiveNameCopiedForEverything(const QModelIndex& ind) const;
  bool on_cellDoubleClicked(const QModelIndex& ind) const;
  bool setSharedMember(GroupedDupVidListArr* pGroupedVidsList, DuplicateVideoDetectionCriteria::DVCriteriaE* pCurDifferType);
  int onLeftVideoGroupsTableSelectionChanged(int newRow);

 private:
  void subscribe();
  bool on_Rename(AdvanceRenamer& renameWid);
  bool on_RecycleSelection();

  RightVideoDuplicatesModel* m_detailsModel{nullptr};
  QSortFilterProxyModel* m_rightSortProxy{nullptr};
};

#endif  // RIGHTVIDEODUPLICATESDETAILS_H
