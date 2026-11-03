#ifndef DUPLICATEVIDEOGROUPSMODEL_H
#define DUPLICATEVIDEOGROUPSMODEL_H

#include "QAbstractTableModelPub.h"
#include "DuplicateVideosHelper.h"

class DuplicateVideoGroupsModel : public QAbstractTableModelPub {
public:
  using QAbstractTableModelPub::QAbstractTableModelPub;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  int rowCount(const QModelIndex& /*parent*/ = {}) const override { return rowCountHelper(m_groupedVidLstArr[(int)m_currentDiffer]); }
  int columnCount(const QModelIndex& /*parent*/ = {}) const override { return sizeof(DUPLICATE_LIST_HEADER) / sizeof(DUPLICATE_LIST_HEADER[0]); }

  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  static GroupedDupVidList getFuzzyDurationDuplicateGroups(const DupVidMetaInfoList& plainList, int dev);
  static GroupedDupVidList getFuzzySizeDuplicateGroups(const DupVidMetaInfoList& plainList, qint64 dev);
  static GroupedDupVidList getFuzzySizeDurationDuplicateGroups(const DupVidMetaInfoList& plainList, qint64 sizeDev, int durationDev);

  void initGroupMember(DuplicateVideoDetectionCriteria::DVCriteriaE newDifferType, qint64 newSize, int newDuration);
  int onDuplicateVideosListChanged(DupVidMetaInfoList needAnalyzeVidLst);
  int setDifferType(DuplicateVideoDetectionCriteria::DVCriteriaE newDifferType);
  int setDeviationSize(qint64 newSize);
  int setDeviationDuration(int newDuration);

  const char* getCurDifferTypeStr() const { return DuplicateVideoDetectionCriteria::c_str(m_currentDiffer); }

  DuplicateVideoDetectionCriteria::DVCriteriaE m_currentDiffer = DuplicateVideoDetectionCriteria::DEFAULT_VD_CRITERIA_E;  // also shared with right detail view
  GroupedDupVidListArr m_groupedVidLstArr;                                                                                // shared with right detail view

private:
  static int rowCountHelper(const GroupedDupVidList& plainLst) { return plainLst.size(); }

  struct NewDataMember {
    NewDataMember(DupVidMetaInfoList* _p_needAnalyzeVidLst, DuplicateVideoDetectionCriteria::DVCriteriaE* _p_currentDiffer, qint64* _p_deviationSz, int* _p_deviationDur):
      p_needAnalyzeVidLst{_p_needAnalyzeVidLst}, p_currentDiffer{_p_currentDiffer}, p_deviationSz{_p_deviationSz}, p_deviationDur{_p_deviationDur} {}
    NewDataMember() = default;
    DupVidMetaInfoList* p_needAnalyzeVidLst{nullptr};
    DuplicateVideoDetectionCriteria::DVCriteriaE* p_currentDiffer{nullptr};
    qint64* p_deviationSz{nullptr};
    int* p_deviationDur{nullptr};
  };
  int SwapMemberCore(NewDataMember newDataMember);

  DupVidMetaInfoList m_needAnalyzeVidLst;
  qint64 m_deviationSz = 2 * 1024;  // bytes
  int m_deviationDur = 2 * 1000;    // ms, 995~1004 => 1000, (v+dev/2)//dev*dev

  static constexpr const char* DUPLICATE_LIST_HEADER[]{"Count", "Value"};

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
#endif  // DUPLICATEVIDEOGROUPSMODEL_H
