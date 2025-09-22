#include "LeftVideoGroupsModel.h"
#include "MemoryKey.h"

using namespace RedundantVideoTool;

const QStringList LeftVideoGroupsModel::DUPLICATE_LIST_HEADER{"Count", "Value"};

LeftVideoGroupsModel::LeftVideoGroupsModel(QObject* parent)  //
    : QAbstractTableModelPub{parent} {
  m_deviationSz = Configuration()
                      .value(MemoryKey::DUPLICATE_FINDER_DEVIATION_FILESIZE.name,  //
                             MemoryKey::DUPLICATE_FINDER_DEVIATION_FILESIZE.v)
                      .toInt();
  m_deviationDur = Configuration()
                       .value(MemoryKey::DUPLICATE_FINDER_DEVIATION_DURATION.name,  //
                              MemoryKey::DUPLICATE_FINDER_DEVIATION_DURATION.v)
                       .toInt();
}

QVariant LeftVideoGroupsModel::data(const QModelIndex& index, int role) const {
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
        case DIFFER_BY_TYPE::DURATION: {
          return grpsList.front().dur;
        }
        case DIFFER_BY_TYPE::SIZE: {
          return grpsList.front().sz;
        }
        default: {
          return c_str(m_currentDiffer);
        }
      }
    }
    default:
      break;
  }
  return {};
}

int LeftVideoGroupsModel::onDuplicateVideosListChanged(DupVidMetaInfoList needAnalyzeVidLst) {
  const int beforeRowCnt = rowCount();
  int afterRowCnt = 0;
  {
    GroupedDupVidList newSizeGroupedLst = getSizeLst(needAnalyzeVidLst, m_deviationSz);
    if (m_currentDiffer == DIFFER_BY_TYPE::SIZE) {
      afterRowCnt = rowCountHelper(newSizeGroupedLst);
      RowsCountBeginChange(beforeRowCnt, afterRowCnt);
    }
    SwapGroupedVidLstArr(DIFFER_BY_TYPE::SIZE, newSizeGroupedLst);
    if (m_currentDiffer == DIFFER_BY_TYPE::SIZE) {
      RowsCountEndChange();
    }
  }

  {
    GroupedDupVidList newDurationGroupedLst = getDurationsLst(needAnalyzeVidLst, m_deviationDur);
    if (m_currentDiffer == DIFFER_BY_TYPE::DURATION) {
      afterRowCnt = rowCountHelper(newDurationGroupedLst);
      RowsCountBeginChange(beforeRowCnt, afterRowCnt);
    }
    SwapGroupedVidLstArr(DIFFER_BY_TYPE::DURATION, newDurationGroupedLst);
    if (m_currentDiffer == DIFFER_BY_TYPE::DURATION) {
      RowsCountEndChange();
    }
  }
  m_plainDupVidLst.swap(needAnalyzeVidLst);
  LOG_D("needAnalyzeVidLst Changed leading groups count changed from %d->%d", beforeRowCnt, afterRowCnt);
  return afterRowCnt - beforeRowCnt;
}

int LeftVideoGroupsModel::setDifferType(const DIFFER_BY_TYPE& newDifferType) {
  if (newDifferType == m_currentDiffer) {
    LOG_D("no need to update differ type, already[%s]", getCurDifferTypeStr());
    return 0;
  }

  const int beforeRowCnt = rowCount();
  int afterRowCnt = 0;
  switch (newDifferType) {
    case DIFFER_BY_TYPE::SIZE: {
      auto newSizeGroupedLst = getSizeLst(m_plainDupVidLst, m_deviationSz);
      afterRowCnt = rowCountHelper(newSizeGroupedLst);
      RowsCountBeginChange(beforeRowCnt, afterRowCnt);
      SwapGroupedVidLstArr(newDifferType, newSizeGroupedLst);
      break;
    }
    case DIFFER_BY_TYPE::DURATION: {
      auto newDurationGroupedLst = getDurationsLst(m_plainDupVidLst, m_deviationDur);
      afterRowCnt = rowCountHelper(newDurationGroupedLst);
      RowsCountBeginChange(beforeRowCnt, afterRowCnt);
      SwapGroupedVidLstArr(newDifferType, newDurationGroupedLst);
      break;
    }
    default: {
      LOG_W("Differ type[%s] invalid", getCurDifferTypeStr());
      return 0;
    }
  }

  m_currentDiffer = newDifferType;
  RowsCountEndChange();
  emit headerDataChanged(Qt::Horizontal, 0, columnCount() - 1);
  LOG_D("DifferType Changed to %s leading groups count changed from %d->%d", getCurDifferTypeStr(), beforeRowCnt, afterRowCnt);
  return afterRowCnt - beforeRowCnt;
}

int LeftVideoGroupsModel::setDeviationDuration(int newDuration) {
  if (m_currentDiffer != DIFFER_BY_TYPE::DURATION) {
    LOG_D("Skip, current differ by is not [%s]", c_str(DIFFER_BY_TYPE::DURATION));
    return 0;
  }
  if (newDuration == m_deviationDur) {
    LOG_D("no need to update DeviationDuration, already [%d]ms", (int)newDuration);
    return 0;
  }
  Configuration().setValue(MemoryKey::DUPLICATE_FINDER_DEVIATION_DURATION.name, newDuration);

  const int oldDuration{newDuration};
  const int beforeRowCnt = rowCount();
  GroupedDupVidList newDurationGroupedLst = getDurationsLst(m_plainDupVidLst, newDuration);
  const int afterRowCnt = rowCountHelper(newDurationGroupedLst);

  RowsCountBeginChange(beforeRowCnt, afterRowCnt);
  SwapGroupedVidLstArr(DIFFER_BY_TYPE::DURATION, newDurationGroupedLst);
  std::swap(m_deviationDur, newDuration);
  RowsCountEndChange();
  LOG_D("Deviation duration changed from %d to %d leading groups count changed from %d->%d", oldDuration, newDuration, beforeRowCnt, afterRowCnt);
  return afterRowCnt - beforeRowCnt;
}

int LeftVideoGroupsModel::setDeviationSize(qint64 newSize) {
  if (m_currentDiffer != DIFFER_BY_TYPE::SIZE) {
    LOG_D("Skip, current differ by is not [%s]", c_str(DIFFER_BY_TYPE::SIZE));
    return 0;
  }
  if (newSize == m_deviationSz) {
    LOG_D("no need to update DeviationSize, already [%d]bytes", (int)newSize);
    return 0;
  }
  Configuration().setValue(MemoryKey::DUPLICATE_FINDER_DEVIATION_FILESIZE.name, newSize);

  const qint64 oldSize{newSize};
  const int beforeRowCnt = rowCount();
  GroupedDupVidList newSizeGroupedLst = getSizeLst(m_plainDupVidLst, newSize);
  const int afterRowCnt = rowCountHelper(newSizeGroupedLst);

  RowsCountBeginChange(beforeRowCnt, afterRowCnt);
  SwapGroupedVidLstArr(DIFFER_BY_TYPE::SIZE, newSizeGroupedLst);
  std::swap(m_deviationSz, newSize);
  RowsCountEndChange();
  LOG_D("Deviation size changed from %d to %d leading groups count changed from %d->%d", oldSize, newSize, beforeRowCnt, afterRowCnt);
  return afterRowCnt - beforeRowCnt;
}

bool sortByListSizeEleCountDescending(const DupVidMetaInfoList& lhsLst, const DupVidMetaInfoList& rhsLst) {
  return lhsLst.size() > rhsLst.size();
}

GroupedDupVidList LeftVideoGroupsModel::getDurationsLst(const DupVidMetaInfoList& plainList, int dev) {
  const auto getFuzzyDur = [dev](int dur) -> int { return dev == 0 ? dur : (dur + dev / 2) / dev; };
  QMap<int, DupVidMetaInfoList> curMap;
  for (auto it = plainList.cbegin(); it != plainList.cend(); ++it) {
    auto fuzzyDur = getFuzzyDur(it->dur);
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

GroupedDupVidList LeftVideoGroupsModel::getSizeLst(const DupVidMetaInfoList& plainList, qint64 dev) {
  const auto getFuzzySz = [dev](qint64 sz) -> qint64 { return dev == 0 ? sz : (sz + dev / 2) / dev; };
  QMap<qint64, DupVidMetaInfoList> curMap;
  for (auto it = plainList.cbegin(); it != plainList.cend(); ++it) {
    auto fuzzySz = getFuzzySz(it->sz);
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
