#include "DuplicateVideoModel.h"

#include "PublicVariable.h"
#include "Tools/QMediaInfo.h"
#include "Tools/MD5Calculator.h"

#include "public/DisplayEnhancement.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>

#include <QBrush>
#include <QFileIconProvider>

const QStringList DuplicateDetailsModel::VIDS_DETAIL_HEADER{"Name", "Date", "Size", "Duration", "Hash", "FullPath"};
const QStringList VidInfoModel::DUPLICATE_LIST_HEADER{"Count", "Value"};
using namespace MD5Calculator;

QVariant DuplicateDetailsModel::data(const QModelIndex& index, int role) const {
  if (p_classifiedSort == nullptr or m_leftRow == -1)
    return {};
  if (not index.isValid())
    return {};
  const int row = index.row();
  const int column = index.column();
  const DUP_INFO& inf = (*p_classifiedSort)[(int)*m_currentDiffer][m_leftRow][row];
  switch (role) {
    case Qt::DisplayRole: {
      switch (column) {
        case 0:
          return inf.name;
        case 1:
          return QDateTime::fromMSecsSinceEpoch(inf.modifiedDate);
        case 2:
          return FILE_PROPERTY_DSP::sizeToHumanReadFriendly(inf.sz);
        case 3:
          return FILE_PROPERTY_DSP::durationToHumanReadFriendly(inf.dur);
        case 4: {
          if (inf.hash.isEmpty() && rowCount() <= 10) {
            return GetMD5(inf.abspath, 1024);
          }
          return inf.hash;
        }
        case 5:
          return inf.abspath;
        default:
          return {};
      }
    }
    case Qt::ForegroundRole: {
      return QFile::exists(inf.abspath) ? QBrush(Qt::GlobalColor::black) : QBrush(Qt::GlobalColor::gray);
    }
    case Qt::DecorationRole: {
      if (column == 0) {
        static QFileIconProvider fip;
          return fip.icon(QFileInfo(inf.abspath));
      }
      return {};
    }
    default:
      return {};
  }
}

auto DuplicateDetailsModel::rowCount(const QModelIndex& /*parent*/) const -> int {
  if (p_classifiedSort == nullptr || m_currentDiffer == nullptr) {
    qWarning("nullptr");
    return 0;
  }
  if (m_leftRow == -1) {
    // nothing selected in left dup list. so right detail model should be 0 row
    return 0;
  }
  const auto& infoLstLst = (*p_classifiedSort)[(int)*m_currentDiffer];
  if (not(0 <= m_leftRow and m_leftRow < infoLstLst.size())) {
    // when differtype changed, m_leftRow should be invalid
    return 0;
  }
  return infoLstLst[m_leftRow].size();
}

auto DuplicateDetailsModel::headerData(int section, Qt::Orientation orientation, int role) const -> QVariant {
  if (role == Qt::TextAlignmentRole) {
    if (orientation == Qt::Vertical) {
      return Qt::AlignRight;
    }
  }
  if (role == Qt::DisplayRole) {
    if (orientation == Qt::Orientation::Horizontal) {
      return VIDS_DETAIL_HEADER[section];
    }
    return section + 1;
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

void DuplicateDetailsModel::onChangeDetailIndex(int newRow) {
  if (p_classifiedSort == nullptr or m_currentDiffer == nullptr) {
    qWarning("m_classifiedSort is nullptr. cannot change m_leftRow in left list");
    return;
  }
  const int beforeRowN = rowCount();
  m_leftRow = newRow;
  const int afterRowN = rowCount();

  if (beforeRowN < afterRowN) {
    beginInsertRows({}, beforeRowN, afterRowN - 1);
    endInsertRows();
  } else if (beforeRowN > afterRowN) {
    beginRemoveRows({}, afterRowN, beforeRowN - 1);
    endRemoveRows();
  }
  if (afterRowN > 1) {
    emit dataChanged(index(0, 0, {}), index(afterRowN - 1, columnCount() - 1, {}), {Qt::ItemDataRole::DisplayRole});
  }
  qDebug("details rowCount %d->%d", beforeRowN, afterRowN);
}

QString DuplicateDetailsModel::fileNameEverything(const QModelIndex& index) const {
  if (not index.isValid()) {
    qWarning("modelindex is invalid");
    return {};
  }
  if (p_classifiedSort == nullptr or m_currentDiffer == nullptr) {
    qWarning("shared member is nullptr");
    return {};
  }
  const DUP_INFO& inf = (*p_classifiedSort)[(int)*m_currentDiffer][m_leftRow][index.row()];
  QString name = QFileInfo(inf.abspath).fileName();
  name.replace(JSON_RENAME_REGEX::INVALID_TABLE_NAME_LETTER, " ");
  return name;
}

QString DuplicateDetailsModel::filePath(const QModelIndex& index) const {
  if (not index.isValid()) {
    qWarning("modelindex is invalid");
    return {};
  }
  if (p_classifiedSort == nullptr or m_currentDiffer == nullptr) {
    qWarning("shared member is nullptr");
    return {};
  }
  const DUP_INFO& inf = (*p_classifiedSort)[(int)*m_currentDiffer][m_leftRow][index.row()];
  return inf.abspath;
}

// call this before change differ type in dupList
// call this before append a path in dupList
void DuplicateDetailsModel::whenDifferTypeAboutToChanged() {
  if (rowCount() > 0) {
    beginRemoveRows({}, 0, rowCount() - 1);
    m_leftRow = -1;
    endRemoveRows();
  } else {
    m_leftRow = -1;
  }
}

void DuplicateDetailsModel::whenItemsRecycledUpdateForgroundColor() {
  const int rowN = rowCount();
  if (rowN <= 0) {
    return;
  }
  emit dataChanged(index(0, 0, {}), index(0, rowN - 1, {}), {Qt::ItemDataRole::ForegroundRole});
}

// --------------------------------------------------------------------------------------

int VidInfoModel::m_deviationDur = 2 * 1000;    // ms => 60s
qint64 VidInfoModel::m_deviationSz = 2 * 1024;  // Byte => 10MB
constexpr int VidInfoModel::NAME_LEVEL_COUNT;

VidInfoModel::VidInfoModel(QObject* parent) : QAbstractTableModel{parent} {
  m_deviationSz =
      PreferenceSettings().value(MemoryKey::DUPLICATE_FINDER_DEVIATION_FILESIZE.name, MemoryKey::DUPLICATE_FINDER_DEVIATION_FILESIZE.v).toInt();
  m_deviationDur =
      PreferenceSettings().value(MemoryKey::DUPLICATE_FINDER_DEVIATION_DURATION.name, MemoryKey::DUPLICATE_FINDER_DEVIATION_DURATION.v).toInt();
}

QVariant VidInfoModel::data(const QModelIndex& index, int role) const {
  if (role != Qt::DisplayRole) {
    return {};
  }
  if (not index.isValid()) {
    return {};
  }
  const int row = index.row();
  const int column = index.column();
  auto& DUP_INFO = m_classifiedSort[(int)m_currentDiffer][row];
  if (column == 0) {
    return DUP_INFO.size();
  }
  if (column == 1) {
    if (m_currentDiffer == DIFFER_BY_TYPE::DURATION) {
      return DUP_INFO.front().dur;
    } else if (m_currentDiffer == DIFFER_BY_TYPE::SIZE) {
      return DUP_INFO.front().sz;
    }
  }
  return {};
}

void VidInfoModel::ChangeTableGroups(const QStringList& tbls) {
  setDataChangedFlag();
  auto& aimd = AIMediaDuplicate::GetInst();
  aimd.ReadSpecifiedTables2List(tbls, m_vidsInfo);
  UpdateMemberList();
  TryUpdateRowCountAndDisplay();
}

void VidInfoModel::TryUpdateRowCountAndDisplay() {
  if (m_beforeRowN == -1 or m_afterRow == -1) {
    resetBeforeAfterRow();
    return;
  }
  if (m_beforeRowN < m_afterRow) {
    beginInsertRows({}, m_beforeRowN, m_afterRow - 1);
    endInsertRows();
  } else if (m_beforeRowN > m_afterRow) {
    beginRemoveRows({}, m_afterRow, m_beforeRowN - 1);
    endRemoveRows();
  }
  emit dataChanged(index(0, 0, {}), index(m_afterRow - 1, columnCount() - 1, {}), {Qt::ItemDataRole::DisplayRole});
  qDebug("dup list rowCount %d->%d", m_beforeRowN, m_afterRow);
  resetBeforeAfterRow();
}

void VidInfoModel::setDifferType(const DIFFER_BY_TYPE& newDifferType) {
  if (m_currentDiffer == newDifferType) {
    qDebug("no need to update differ type, already[%d]", (int)m_currentDiffer);
    return;
  }

  const int totalBeforeRow = m_classifiedSort[(int)m_currentDiffer].size();
  m_currentDiffer = newDifferType;
  UpdateMemberList();
  const int totalAfterRow = m_classifiedSort[(int)m_currentDiffer].size();

  m_beforeRowN = totalBeforeRow;
  m_afterRow = totalAfterRow;
  TryUpdateRowCountAndDisplay();
}

void VidInfoModel::setDeviationDuration(int newDuration) {
  if (newDuration == m_deviationDur) {
    return;
  }
  PreferenceSettings().setValue(MemoryKey::DUPLICATE_FINDER_DEVIATION_DURATION.name, newDuration);
  m_deviationDur = newDuration;
  setDataChangedFlag();
  UpdateMemberList();
  TryUpdateRowCountAndDisplay();
}

void VidInfoModel::setDeviationSize(int newSize) {
  if (newSize == m_deviationSz) {
    return;
  }
  PreferenceSettings().setValue(MemoryKey::DUPLICATE_FINDER_DEVIATION_FILESIZE.name, newSize);
  m_deviationSz = newSize;
  setDataChangedFlag();
  UpdateMemberList();
  TryUpdateRowCountAndDisplay();
}

bool sortByListSize(const QList<DUP_INFO>& lhsLst, const QList<DUP_INFO>& rhsLst) {
  return lhsLst.size() > rhsLst.size();
}

void VidInfoModel::UpdateMemberList() {
  if (not m_dataChangedFlag[(int)m_currentDiffer]) {
    qDebug("m_classifiedSort[%d] not changed, no need update member", (int)m_currentDiffer);
    return;
  }
  m_beforeRowN = m_classifiedSort[(int)m_currentDiffer].size();
  switch (m_currentDiffer) {
    case DIFFER_BY_TYPE::SIZE:
      m_classifiedSort[(int)m_currentDiffer] = getSizeLst();
      m_dataChangedFlag[(int)m_currentDiffer] = true;
      break;
    case DIFFER_BY_TYPE::DURATION:
      m_classifiedSort[(int)m_currentDiffer] = getDurationsLst();
      m_dataChangedFlag[(int)m_currentDiffer] = true;
      break;
    default:
      resetBeforeAfterRow();
      return;
  }
  m_afterRow = m_classifiedSort[(int)m_currentDiffer].size();
}

QList<QList<DUP_INFO>> VidInfoModel::getDurationsLst() const {
  QMap<qint64, QList<DUP_INFO>> curMap;
  for (auto it = m_vidsInfo.cbegin(); it != m_vidsInfo.cend(); ++it) {
    auto fuzzyDur = getFuzzyDur(it->dur);
    auto durIt = curMap.find(fuzzyDur);
    if (durIt == curMap.end()) {
      curMap.insert(fuzzyDur, {*it});
      continue;
    }
    durIt.value().append(*it);
  }

  QList<QList<DUP_INFO>> sortByCnt2DList;
  for (auto it = curMap.cbegin(); it != curMap.cend(); ++it) {
    if (it.value().size() < 2) {
      continue;
    }
    sortByCnt2DList.append(it.value());
  }
  std::sort(sortByCnt2DList.begin(), sortByCnt2DList.end(), sortByListSize);
  return sortByCnt2DList;
}

QList<QList<DUP_INFO>> VidInfoModel::getSizeLst() const {
  QMap<qint64, QList<DUP_INFO>> curMap;
  for (auto it = m_vidsInfo.cbegin(); it != m_vidsInfo.cend(); ++it) {
    auto fuzzySz = getFuzzySz(it->sz);
    auto szIt = curMap.find(fuzzySz);
    if (szIt == curMap.end()) {
      curMap.insert(fuzzySz, {*it});
      continue;
    }
    szIt.value().append(*it);
  }

  QList<QList<DUP_INFO>> sortByCnt2DList;
  for (auto it = curMap.cbegin(); it != curMap.cend(); ++it) {
    if (it.value().size() < 2) {
      continue;
    }
    sortByCnt2DList.append(it.value());
  }
  std::sort(sortByCnt2DList.begin(), sortByCnt2DList.end(), sortByListSize);
  return sortByCnt2DList;
}
