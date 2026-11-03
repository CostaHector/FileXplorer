#ifndef DUPLICATEVIDEOGROUPSTABLE_H
#define DUPLICATEVIDEOGROUPSTABLE_H

#include "CustomTableView.h"
#include "DuplicateVideosHelper.h"
#include "DuplicateVideoGroupsModel.h"
#include <QSortFilterProxyModel>

class DuplicateVideoGroupsTable : public CustomTableView {
  Q_OBJECT
 public:
  friend class DuplicateVideosFinder;
  explicit DuplicateVideoGroupsTable(QWidget* parent = nullptr);
  const QString GetCurrentDupVideoGroupInfo() const;

  void initGroupMember(DuplicateVideoDetectionCriteria::DVCriteriaE newDifferType, qint64 newSize, int newDuration);
  void setDifferType(const DuplicateVideoDetectionCriteria::DVCriteriaE& newDifferType);
  void setDeviationSize(qint64 newSize);
  void setDeviationDuration(int newDuration);
  int GetCurSrcIndexRow(const QItemSelection& selected) const;

 signals:
  // curRow >=0: valid, otherwise invalid
  void leftSelectionChanged(int curRow = INVALID_LEFT_SELECTED_ROW);

 public slots:
  int onDuplicateVideosListChanged(const DupVidMetaInfoList& needAnalyzeVidLst);

 private:
  void subscribe();
  DuplicateVideoGroupsModel* m_leftGrpModel{nullptr};
  QSortFilterProxyModel* m_leftSortProxy{nullptr};
};

#endif  // DUPLICATEVIDEOGROUPSTABLE_H
