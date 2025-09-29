#include "SortedUniqStrLst.h"
#include "NameTool.h"
#include "Logger.h"

QStringList SortedUniqStrLst::Sentence2StringList(const QString& sentence) {
  QStringList newItems = sentence.trimmed().split(NameTool::CAST_STR_SPLITTER, Qt::SkipEmptyParts);
  newItems.removeAll("");
  return newItems;
}

SortedUniqStrLst::SortedUniqStrLst(const QString& sentence) {
  insertBatchFromSentence(sentence);
}

SortedUniqStrLst::SortedUniqStrLst(const QStringList& initList) {
  insertBatch(initList);
}

SortedUniqStrLst::SortedUniqStrLst(const SortedUniqStrLst& rhs)  //
    : m_set{rhs.m_set},                                          //
      m_sortedCache{rhs.m_sortedCache},
      mAnsCSV{rhs.mAnsCSV},
      mJoinCalled{rhs.mJoinCalled}  //
{}

void SortedUniqStrLst::swap(SortedUniqStrLst& rhs) noexcept {
  m_set.swap(rhs.m_set);
  m_sortedCache.swap(rhs.m_sortedCache);
  mAnsCSV.swap(rhs.mAnsCSV);
  std::swap(mJoinCalled, rhs.mJoinCalled);
}

SortedUniqStrLst& SortedUniqStrLst::operator+=(const SortedUniqStrLst& rhs) noexcept {
  m_set.unite(rhs.m_set);

  QStringList merged;
  merged.reserve(m_sortedCache.size() + rhs.m_sortedCache.size());
  std::merge(m_sortedCache.cbegin(), m_sortedCache.cend(),          //
             rhs.m_sortedCache.cbegin(), rhs.m_sortedCache.cend(),  //
             std::back_inserter(merged));                           //
  m_sortedCache.swap(merged);
  m_sortedCache.removeDuplicates();

  mJoinCalled = false;
  return *this;
}

SortedUniqStrLst& SortedUniqStrLst::operator-=(const QString& oneElement) noexcept {
  remove(oneElement);
  return *this;
}

void SortedUniqStrLst::setBatchFromSentence(const QString& sentence) {
  clear();
  insertBatchFromSentence(sentence);
}

void SortedUniqStrLst::setBatch(const QStringList& list) {
  QStringList lst{list};
  lst.sort();
  lst.removeDuplicates();

  decltype(m_set) temp{lst.begin(), lst.end()};
  m_set.swap(temp);
  m_sortedCache.swap(lst);

  mJoinCalled = false;
}

void SortedUniqStrLst::insertBatchFromSentence(const QString& sentence) {
  const QStringList& newItems = Sentence2StringList(sentence);
  if (newItems.isEmpty()) {
    // insert nothing
    return;
  }
  insertBatch(newItems);
}

void SortedUniqStrLst::insertBatch(const QStringList& newItems) {
  switch (newItems.size()) {
    case 0:
      mJoinCalled = false;
      return;
    case 1:
    case 2:
    case 3:
      insertBatch_LE_3(newItems);
      break;
    case 4:
    case 5:
      insertBatch_LE_5(newItems);
      break;
    default:
      insertBatch_GT_5(newItems);
      break;
  }
}

void SortedUniqStrLst::insertBatch_LE_3(const QStringList& list) {
  for (const auto& str : list) {
    auto strIt = m_set.find(str);
    if (strIt != m_set.end()) {
      continue;
    }
    m_set.insert(str);
    auto it = std::lower_bound(m_sortedCache.begin(), m_sortedCache.end(), str);
    m_sortedCache.insert(it, str);
  }
  mJoinCalled = false;
}

void SortedUniqStrLst::insertBatch_LE_5(const QStringList& list) {
  QStringList tmp;
  tmp.reserve(list.size());
  for (const auto& str : list) {
    if (m_set.contains(str))
      continue;
    tmp.append(str);
    m_set.insert(str);
  }
  if (!tmp.isEmpty()) {
    m_sortedCache.append(tmp);
    std::sort(m_sortedCache.begin(), m_sortedCache.end());
  }
  mJoinCalled = false;
}

void SortedUniqStrLst::insertBatch_GT_5(const QStringList& newItems) {
  QSet<QString> newSet(newItems.begin(), newItems.end());
  newSet.subtract(m_set);
  if (newSet.isEmpty()) {
    return;
  }

  QStringList processed = newSet.values();
  std::sort(processed.begin(), processed.end());

  if (m_sortedCache.isEmpty()) {
    m_sortedCache.swap(processed);
  } else {
    QStringList merged;
    merged.reserve(m_sortedCache.size() + processed.size());
    std::merge(m_sortedCache.cbegin(), m_sortedCache.cend(), processed.cbegin(), processed.cend(), std::back_inserter(merged));
    m_sortedCache.swap(merged);
  }

  m_set.unite(newSet);
  mJoinCalled = false;
}

void SortedUniqStrLst::insertOneElementFromHint(const QString& target) {
  // no process at target
  insertBatch_LE_3({target});
}

bool SortedUniqStrLst::remove(const QString& target) {
  auto targetIt = m_set.find(target);
  if (targetIt == m_set.end()) {
    return false;
  }
  mJoinCalled = false;
  m_set.erase(targetIt);
  if (m_sortedCache.size() > 5) {
    auto it = std::lower_bound(m_sortedCache.begin(), m_sortedCache.end(), target);
    if (it != m_sortedCache.end() && *it == target) {
      m_sortedCache.erase(it);
      return true;
    }
  } else {
    m_sortedCache.removeOne(target);
    return true;
  }
  LOG_W("[Error] m_set and m_sortedCache not correspond.");
  return false;
}

void SortedUniqStrLst::format() {
  const QStringList& lst = Sentence2StringList(join());
  setBatch(lst);
}

QSet<QString> SortedUniqStrLst::toLowerSets() const {
  QSet<QString> result;
  result.reserve(m_set.size());
  for (const QString& str : m_set) {
    result.insert(str.toLower());
  }
  return result;
}

const QString& SortedUniqStrLst::join() const {
  if (!mJoinCalled) {
    mAnsCSV = m_sortedCache.join(NameTool::CSV_COMMA);
    mAnsCell = m_sortedCache.join(NameTool::CELL_NEW_LINE);
    mJoinCalled = true;
  }
  return mAnsCSV;
}

const QString& SortedUniqStrLst::joinToShowInACell() const {
  if (!mJoinCalled) {
    mAnsCSV = m_sortedCache.join(NameTool::CSV_COMMA);
    mAnsCell = m_sortedCache.join(NameTool::CELL_NEW_LINE);
    mJoinCalled = true;
  }
  return mAnsCell;
}
