#ifndef LEFTVIDEOGROUPSMODEL_H
#define LEFTVIDEOGROUPSMODEL_H

#include "QAbstractTableModelPub.h"
#include "DuplicateVideosHelper.h"

class LeftVideoGroupsModel : public QAbstractTableModelPub {
 public:
  explicit LeftVideoGroupsModel(QObject* parent = nullptr);

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  auto rowCount(const QModelIndex& /*parent*/ = {}) const -> int override { return rowCountHelper(m_groupedVidLstArr[(int)m_currentDiffer]); }
  auto columnCount(const QModelIndex& /*parent*/ = {}) const -> int override { return DUPLICATE_LIST_HEADER.size(); }

  auto headerData(int section, Qt::Orientation orientation, int role) const -> QVariant override {
    if (role == Qt::TextAlignmentRole) {
      if (orientation == Qt::Vertical) {
        return Qt::AlignRight;
      }
    }
    if (role == Qt::DisplayRole) {
      if (orientation == Qt::Orientation::Horizontal) {
        if (section == 0) {
          return DUPLICATE_LIST_HEADER[0];
        } else if (section == 1) {
          return DuplicateVideoDetectionCriteria::c_str(m_currentDiffer);
        }
      }
      return section + 1;
    }
    return QAbstractTableModel::headerData(section, orientation, role);
  }

  int onDuplicateVideosListChanged(DupVidMetaInfoList needAnalyzeVidLst);

  static GroupedDupVidList getDurationsLst(const DupVidMetaInfoList& plainList, int dev);
  static GroupedDupVidList getSizeLst(const DupVidMetaInfoList& plainList, qint64 dev);

  int setDifferType(const DuplicateVideoDetectionCriteria::DVCriteriaE& newDifferType);
  int setDeviationDuration(int newDuration);
  int setDeviationSize(qint64 newSize);

  const char* getCurDifferTypeStr() const { return DuplicateVideoDetectionCriteria::c_str(m_currentDiffer); }

  DuplicateVideoDetectionCriteria::DVCriteriaE m_currentDiffer = DuplicateVideoDetectionCriteria::DEFAULT_VD_CRITERIA_E;  // also shared with right detail view
  GroupedDupVidListArr m_groupedVidLstArr;

 private:
  void changeDifferByTo(DuplicateVideoDetectionCriteria::DVCriteriaE newDiffBy) { m_currentDiffer = newDiffBy; }
  void changeDurTo(int newDur) { m_deviationDur = newDur; }
  void changeSzTo(int newSz) { m_deviationSz = newSz; }
  int rowCountHelper(const GroupedDupVidList& plainLst) const { return plainLst.size(); }

  DupVidMetaInfoList m_plainDupVidLst;

  int m_deviationDur = 2 * 1000;    // ms, 995~1004 => 1000, (v+dev/2)//dev*dev
  qint64 m_deviationSz = 2 * 1024;  // bytes

  static const QStringList DUPLICATE_LIST_HEADER;

  void SwapGroupedVidLstArr(DuplicateVideoDetectionCriteria::DVCriteriaE differBy, GroupedDupVidList& newList) {
    m_groupedVidLstArr[(int)differBy].swap(newList);
#ifdef RUNNING_UNIT_TESTS
    ++m_groupedVidLstArrChangedTimesInTestCase;
  }
  int m_groupedVidLstArrChangedTimesInTestCase{0};
#else
  }
#endif
};
#endif  // LEFTVIDEOGROUPSMODEL_H
