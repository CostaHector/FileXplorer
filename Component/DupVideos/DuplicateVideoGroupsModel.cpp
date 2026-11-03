#include "DuplicateVideoGroupsModel.h"
#include "DataFormatter.h"
#include "Logger.h"

using namespace DuplicateVideoDetectionCriteria;

constexpr const char* DuplicateVideoGroupsModel::DUPLICATE_LIST_HEADER[];

void DuplicateVideoGroupsModel::initGroupMember(DuplicateVideoDetectionCriteria::DVCriteriaE newDifferType, qint64 newSize, int newDuration) {
  m_deviationSz = newSize;
  m_deviationDur = newDuration;
  m_currentDiffer = newDifferType;
}

QVariant DuplicateVideoGroupsModel::data(const QModelIndex& index, int role) const {
  if (role != Qt::DisplayRole) {
    return {};
  }
  if (!index.isValid()) {
    return {};
  }
  const int row = index.row();
  const int column = index.column();
  const DupVidMetaInfoList& grpsList = m_groupedVidLstArr[(int)m_currentDiffer][row];
  switch (column) {
    case 0:
      return grpsList.size();
    case 1: {
      switch (m_currentDiffer) {
        case DVCriteriaE::DURATION: {
          return DataFormatter::formatDurationISO(grpsList.front().m_Duration);
        }
        case DVCriteriaE::SIZE: {
          return DataFormatter::formatFileSizeGMKB(grpsList.front().m_Size);
        }
        case DVCriteriaE::SIZE_AND_DURATION: {
          return DataFormatter::formatFileSizeGMKB(grpsList.front().m_Size) + ", " + DataFormatter::formatDurationISO(grpsList.front().m_Duration);
        }
        default: {
          return {};
        }
      }
    }
    default:
      break;
  }
  return {};
}

QVariant DuplicateVideoGroupsModel::headerData(int section, Qt::Orientation orientation, int role) const {
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

int DuplicateVideoGroupsModel::SwapMemberCore(NewDataMember newDataMember) {
  const int beforeRowCnt = rowCount();
  int afterRowCnt = 0;

  DupVidMetaInfoList& needAnalyzeVidLst{newDataMember.p_needAnalyzeVidLst != nullptr ? *newDataMember.p_needAnalyzeVidLst : m_needAnalyzeVidLst};
  DuplicateVideoDetectionCriteria::DVCriteriaE& currentDiffer{newDataMember.p_currentDiffer != nullptr ? *newDataMember.p_currentDiffer : m_currentDiffer};
  qint64& deviationSz{newDataMember.p_deviationSz != nullptr ? *newDataMember.p_deviationSz : m_deviationSz};
  int& deviationDur{newDataMember.p_deviationDur != nullptr ? *newDataMember.p_deviationDur : m_deviationDur};

  switch (currentDiffer) {
    case DVCriteriaE::SIZE: {
      auto newSizeGroupedLst = getFuzzySizeDuplicateGroups(needAnalyzeVidLst, deviationSz);
      afterRowCnt = rowCountHelper(newSizeGroupedLst);
      RowsCountBeginChange(beforeRowCnt, afterRowCnt);
      SwapGroupedVidLstArr(DVCriteriaE::SIZE, newSizeGroupedLst);
      break;
    }
    case DVCriteriaE::DURATION: {
      auto newDurationGroupedLst = getFuzzyDurationDuplicateGroups(needAnalyzeVidLst, deviationDur);
      afterRowCnt = rowCountHelper(newDurationGroupedLst);
      RowsCountBeginChange(beforeRowCnt, afterRowCnt);
      SwapGroupedVidLstArr(DVCriteriaE::DURATION, newDurationGroupedLst);
      break;
    }
    case DVCriteriaE::SIZE_AND_DURATION: {
      auto newDurationGroupedLst = getFuzzySizeDurationDuplicateGroups(needAnalyzeVidLst, deviationSz, deviationDur);
      afterRowCnt = rowCountHelper(newDurationGroupedLst);
      RowsCountBeginChange(beforeRowCnt, afterRowCnt);
      SwapGroupedVidLstArr(DVCriteriaE::SIZE_AND_DURATION, newDurationGroupedLst);
      break;
    }
    default: {
      LOG_W("Differ type[%s] invalid", getCurDifferTypeStr());
      return 0;
    }
  }
  if (newDataMember.p_needAnalyzeVidLst != nullptr) {
    m_needAnalyzeVidLst.swap(needAnalyzeVidLst);
  }
  if (newDataMember.p_currentDiffer != nullptr) {
    std::swap(m_currentDiffer, currentDiffer);
  }
  if (newDataMember.p_deviationSz != nullptr) {
    std::swap(m_deviationSz, deviationSz);
  }
  if (newDataMember.p_deviationDur != nullptr) {
    std::swap(m_deviationDur, deviationDur);
  }

  RowsCountEndChange();

  const int delta = afterRowCnt - beforeRowCnt;
  if (newDataMember.p_currentDiffer != nullptr) {
    emit headerDataChanged(Qt::Horizontal, 0, columnCount() - 1);
  }
  return delta;
}

int DuplicateVideoGroupsModel::onDuplicateVideosListChanged(DupVidMetaInfoList needAnalyzeVidLst) {
  int delta = SwapMemberCore(NewDataMember{&needAnalyzeVidLst, nullptr, nullptr, nullptr});
  LOG_D("needAnalyzeVidLst Changed leading groups count changed delta=%d", delta);
  return delta;
}

int DuplicateVideoGroupsModel::setDifferType(DVCriteriaE newDifferType) {
  if (newDifferType == m_currentDiffer) {
    LOG_D("no need to update differ type, already[%s]", getCurDifferTypeStr());
    return 0;
  }
  int delta = SwapMemberCore(NewDataMember{nullptr, &newDifferType, nullptr, nullptr});
  LOG_D("DifferType Changed to[%s] leading groups count changed delta=%d", getCurDifferTypeStr(), delta);
  return delta;
}

int DuplicateVideoGroupsModel::setDeviationSize(qint64 newSize) {
  const qint64 oldSize{m_deviationSz};
  if (newSize == oldSize) {
    LOG_D("no need to update DeviationSize, already [%d]bytes", (int)newSize);
    return 0;
  }
  if (m_currentDiffer == DuplicateVideoDetectionCriteria::DVCriteriaE::DURATION) {
    std::swap(m_deviationSz, newSize);
    return 0;
  }
  int delta = SwapMemberCore(NewDataMember{nullptr, nullptr, &newSize, nullptr});
  LOG_D("Deviation size changed leading groups count changed delta=%d", delta);
  return delta;
}

int DuplicateVideoGroupsModel::setDeviationDuration(int newDuration) {
  const int oldDuration{m_deviationDur};
  if (newDuration == oldDuration) {
    LOG_D("no need to update DeviationDuration, already [%d]ms", (int)newDuration);
    return 0;
  }
  if (m_currentDiffer == DuplicateVideoDetectionCriteria::DVCriteriaE::SIZE) {
    std::swap(m_deviationDur, newDuration);
    return 0;
  }
  int delta = SwapMemberCore(NewDataMember{nullptr, nullptr, nullptr, &newDuration});
  LOG_D("Deviation duration changed leading groups count changed delta=%d", delta);
  return delta;
}

bool sortByListSizeEleCountDescending(const DupVidMetaInfoList& lhsLst, const DupVidMetaInfoList& rhsLst) {
  return lhsLst.size() > rhsLst.size();
}

template <typename TNumber>
TNumber getFuzzyValue(const TNumber& number, const TNumber& dev) {
  return dev == 0 ? number : (number + dev / 2) / dev;
}

template int     getFuzzyValue<int>(const int&, const int&);
template qint64  getFuzzyValue<qint64>(const qint64&, const qint64&);

GroupedDupVidList DuplicateVideoGroupsModel::getFuzzyDurationDuplicateGroups(const DupVidMetaInfoList& plainList, int dev) {
  if (plainList.isEmpty()) {
    return {};
  }
  QMap<int, DupVidMetaInfoList> curMap;
  for (auto it = plainList.cbegin(); it != plainList.cend(); ++it) {
    auto fuzzyDur = getFuzzyValue(it->m_Duration, dev);
    auto durIt = curMap.find(fuzzyDur);
    if (durIt == curMap.end()) {
      curMap.insert(fuzzyDur, {*it});
      continue;
    }
    durIt.value().append(*it);
  }

  GroupedDupVidList sortByCnt2DList;
  for (auto it = curMap.cbegin(); it != curMap.cend(); ++it) {
    if (it.value().size() < 2) {
      continue;
    }
    sortByCnt2DList.append(it.value());
  }
  std::sort(sortByCnt2DList.begin(), sortByCnt2DList.end(), sortByListSizeEleCountDescending);
  return sortByCnt2DList;
}

GroupedDupVidList DuplicateVideoGroupsModel::getFuzzySizeDuplicateGroups(const DupVidMetaInfoList& plainList, qint64 dev) {
  if (plainList.isEmpty()) {
    return {};
  }
  QMap<qint64, DupVidMetaInfoList> curMap;
  for (auto it = plainList.cbegin(); it != plainList.cend(); ++it) {
    auto fuzzySz = getFuzzyValue(it->m_Size, dev);
    auto szIt = curMap.find(fuzzySz);
    if (szIt == curMap.end()) {
      curMap.insert(fuzzySz, {*it});
      continue;
    }
    szIt.value().append(*it);
  }

  GroupedDupVidList sortByCnt2DList;
  for (auto it = curMap.cbegin(); it != curMap.cend(); ++it) {
    if (it.value().size() < 2) {
      continue;
    }
    sortByCnt2DList.append(it.value());
  }
  std::sort(sortByCnt2DList.begin(), sortByCnt2DList.end(), sortByListSizeEleCountDescending);
  return sortByCnt2DList;
}

GroupedDupVidList DuplicateVideoGroupsModel::getFuzzySizeDurationDuplicateGroups(const DupVidMetaInfoList& plainList, qint64 sizeDev, int durationDev) {
  if (plainList.isEmpty()) {
    return {};
  }
  const GroupedDupVidList sizeGroups = getFuzzySizeDuplicateGroups(plainList, sizeDev);
  if (sizeGroups.isEmpty()) {
    return {};
  }

  GroupedDupVidList result;
  result.reserve(plainList.size());
  for (const DupVidMetaInfoList& sizeGroup : sizeGroups) {
    result += getFuzzyDurationDuplicateGroups(sizeGroup, durationDev);
  }

  std::sort(result.begin(), result.end(), sortByListSizeEleCountDescending);
  return result;
}
