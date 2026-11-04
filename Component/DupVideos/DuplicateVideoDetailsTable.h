#ifndef DUPLICATEVIDEODETAILSTABLE_H
#define DUPLICATEVIDEODETAILSTABLE_H

#include "CustomTableView.h"
#include "DuplicateVideoDetailsModel.h"
#include "DuplicateVideosHelper.h"
#include <QSortFilterProxyModel>

class AdvanceRenamer;
class DuplicateVideoDetailsTable : public CustomTableView {
 public:
  friend class DuplicateVideosFinder;
  explicit DuplicateVideoDetailsTable(QWidget* parent = nullptr);

  QStringList selectedFilesFullPath() const;
  bool on_effectiveNameCopiedForEverything(const QModelIndex& ind) const;
  bool on_cellDoubleClicked(const QModelIndex& ind) const;

  bool setSharedMember(GroupedDupVidListArr* pGroupedVidsList, DuplicateVideoDetectionCriteria::DVCriteriaE* pCurDifferType);
  int onLeftVideoGroupsTableSelectionChanged(int newRow);


public slots:
  bool on_RenameReplace();

 private:
  void subscribe();
  bool on_Rename(AdvanceRenamer& renameWid);
  bool on_RecycleSelection();

  DuplicateVideoDetailsModel* m_detailsModel{nullptr};
  QSortFilterProxyModel* m_rightSortProxy{nullptr};
};

#endif  // DUPLICATEVIDEODETAILSTABLE_H
