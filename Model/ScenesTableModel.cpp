#include "ScenesTableModel.h"
#include <QObject>
#include <QPixmap>
#include <QDirIterator>
#include <QDir>
#include <QBrush>

ScenesTableModel::ScenesTableModel(QObject* object) : QAbstractTableModelPub(object) {}

QVariant ScenesTableModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return {};
  }
  if (IsScnsEmpty()) {
    return {};
  }
  const int linearInd = toLinearIndex(index);
  if (mCurBegin + linearInd >= mCurEnd) {
    return {};
  }
  switch (role) {
    case Qt::ItemDataRole::DisplayRole: {
      return mCurBegin[linearInd].name;
    }
    case Qt::ItemDataRole::DecorationRole: {
      if (mCurBegin[linearInd].imgs.isEmpty()) {
        return {};
      }
      const QString imgAbsPath = mRootPath + mCurBegin[linearInd].rel2scn + mCurBegin[linearInd].imgs.front();
      if (!QFile::exists(imgAbsPath)) {
        return {};
      }
      const QPixmap pm{imgAbsPath};
      if (pm.width() * 280 >= pm.height() * 480) {
        return pm.scaledToWidth(480);
      }
      return pm.scaledToHeight(280);
    }
    case Qt::ItemDataRole::BackgroundRole: {
      if (mCurBegin[linearInd].vidName.isEmpty()) {
        return QBrush(Qt::GlobalColor::darkGray, Qt::BrushStyle::SolidPattern);
      }
      break;
    }
//    case Qt::ItemDataRole::ToolTipRole: {
//      static const QString TOOLTIP_TEMPLATE = R"(<b>%1</b><br/>%2<br/>%3)";
//      return TOOLTIP_TEMPLATE.arg(mCurBegin[linearInd].name,                                                 // name
//                                  FILE_PROPERTY_DSP::sizeToHumanReadFriendly(mCurBegin[linearInd].vidSize),  // size
//                                  mCurBegin[linearInd].GetFirstKImagesLabel(mRootPath));                     // images
//    }
    default:
      break;
  }
  return {};
}

QVariant ScenesTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role == Qt::TextAlignmentRole) {
    if (orientation == Qt::Vertical) {
      return Qt::AlignRight;
    }
  } else if (role == Qt::DisplayRole) {
    if (orientation == Qt::Orientation::Horizontal) {
      return section + 1;
    }
    return section + 1;
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

QFileInfo ScenesTableModel::fileInfo(const QModelIndex& index) const {
  int linearInd{-1};
  if (!isIndexValid(index, &linearInd)) {
    return {};
  }

  if (mCurBegin[linearInd].vidName.isEmpty()) {
    qDebug("vidName is empty");
    return {};
  }
  return QFileInfo(mRootPath + mCurBegin[linearInd].rel2scn + mCurBegin[linearInd].vidName);
}

QString ScenesTableModel::filePath(const QModelIndex& index) const {
  int linearInd{-1};
  if (!isIndexValid(index, &linearInd)) {
    return {};
  }

  if (mCurBegin[linearInd].vidName.isEmpty()) {
    qDebug("vidName is empty");
    return {};
  }
  return mRootPath + mCurBegin[linearInd].rel2scn + mCurBegin[linearInd].vidName;
}

QString ScenesTableModel::fileName(const QModelIndex& index) const {
  int linearInd{-1};
  if (!isIndexValid(index, &linearInd)) {
    return {};
  }
  if (mCurBegin[linearInd].vidName.isEmpty()) {
    qDebug("vidName is empty");
    return {};
  }
  return mCurBegin[linearInd].vidName;
}

QString ScenesTableModel::baseName(const QModelIndex& index) const {
  int linearInd{-1};
  if (!isIndexValid(index, &linearInd)) {
    return {};
  }
  return mCurBegin[linearInd].name;
}

QString ScenesTableModel::absolutePath(const QModelIndex& index) const {
  int linearInd{-1};
  if (!isIndexValid(index, &linearInd)) {
    return {};
  }
  return mRootPath + mCurBegin[linearInd].rel2scn;
}

bool ScenesTableModel::setRootPath(const QString& rootPath, const bool bForce) {
  if (mRootPath == rootPath && !bForce) {
    qDebug("Scene ignore set same root path");
    return true;
  }
  mRootPath = rootPath;

  SCENES_TYPE newFilteredList;
  SCENES_TYPE newEntryList = SceneInfoManager::GetScenesFromPath(mRootPath, mFilterEnable, mPattern, &newFilteredList);
  qDebug("new path[%s], imgs[%d], imgsFiltered[%d]", qPrintable(mRootPath), newEntryList.size(), newFilteredList.size());

  const int ELE_N = mFilterEnable ? newFilteredList.size() : newEntryList.size();
  int newBegin{0}, newEnd{0};
  std::tie(newBegin, newEnd) = GetEntryIndexBE(ELE_N);
  const int newScenesCnt = newEnd - newBegin;

  const int beforeRow = rowCount();
  const int afterRow = newScenesCnt / mSCENES_CNT_COLUMN + int(newScenesCnt % mSCENES_CNT_COLUMN != 0);
  if (mFilterEnable) {
    qDebug("Filtered[%s] elements: %d->%d", qPrintable(mPattern), mEntryListFiltered.size(), newFilteredList.size());
  } else {
    qDebug("Entry elements: %d->%d", mEntryList.size(), newEntryList.size());
  }
  qDebug("setRootPath. RowCountChanged: %d->%d", beforeRow, afterRow);
  RowsCountBeginChange(beforeRow, afterRow);

  mEntryList.swap(newEntryList);
  mEntryListFiltered.swap(newFilteredList);
  if (mFilterEnable) {
    mCurBegin = mEntryListFiltered.cbegin() + newBegin;
    mCurEnd = mEntryListFiltered.cbegin() + newEnd;
  } else {
    mCurBegin = mEntryList.cbegin() + newBegin;
    mCurEnd = mEntryList.cbegin() + newEnd;
  }

  RowsCountEndChange();
  qDebug("set root succeed");
  return true;
}

QStringList ScenesTableModel::GetImgs(const QModelIndex& index) const{
  static QStringList EMPTY_IMGS_LIST;
  const int linearInd = toLinearIndex(index);
  if (mCurBegin + linearInd >= mCurEnd) {
    return EMPTY_IMGS_LIST;
  }
  QStringList imgs;
  imgs.reserve(mCurBegin[linearInd].imgs.size());
  for (const QString& name: mCurBegin[linearInd].imgs) {
    imgs.append(mRootPath + mCurBegin[linearInd].rel2scn + name);
  }
  return imgs;
}

bool ScenesTableModel::ChangeColumnsCnt(int newColumnCnt, int newPageIndex) {
  if (newColumnCnt <= 0 or newPageIndex < -1) {
    qDebug("Invalid column count %d or page index %d", newColumnCnt, newPageIndex);
    return true;
  }

  int beforeRowCnt{0};
  int afterRowCnt{0};
  int beforeColumnCnt{0};
  int afterColumnCnt{0};

  int begin{0}, end{0};
  const SCENES_TYPE& lst = GetEntryList();
  const int TOTAL_N = GetEntryListLen();
  if (mSCENES_CNT_ROW == -1) {
    qDebug("Row count = -1, all items in one page");
    begin = 0;
    end = TOTAL_N;

    beforeRowCnt = rowCount();
    afterRowCnt = TOTAL_N / newColumnCnt + int(TOTAL_N % newColumnCnt != 0);
    beforeColumnCnt = columnCount();
    afterColumnCnt = newColumnCnt;
  } else {
    begin = std::min(mSCENES_CNT_ROW * newColumnCnt * mPageIndex, TOTAL_N);
    end = std::min(mSCENES_CNT_ROW * newColumnCnt * (mPageIndex + 1), TOTAL_N);
    const int TOTAL_PAGE_IND = end - begin;

    beforeRowCnt = rowCount();
    afterRowCnt = mSCENES_CNT_ROW;
    beforeColumnCnt = columnCount();
    afterColumnCnt = TOTAL_PAGE_IND / mSCENES_CNT_ROW + int(TOTAL_PAGE_IND % mSCENES_CNT_ROW != 0);
  }
  qDebug("ChangeColumnsCnt. columnCnt: %d->%d, rowCnt: %d->%d", beforeColumnCnt, afterColumnCnt, beforeRowCnt, afterRowCnt);

  ColumnsCountBeginChange(beforeColumnCnt, afterColumnCnt);
  RowsCountBeginChange(beforeRowCnt, afterRowCnt);

  mSCENES_CNT_COLUMN = newColumnCnt;
  mPageIndex = newPageIndex;
  mCurBegin = lst.cbegin() + begin;
  mCurEnd = lst.cbegin() + end;

  RowsCountEndChange();
  ColumnsCountEndChange();

  qDebug("============== ChangeColumnsCnt new dimension: %dx%d ==============", rowCount(), columnCount());
  return true;
}

void ScenesTableModel::SortOrder(SceneInfoManager::SceneSortOption sortOption, bool reverse) {
  if (sortOption == SceneInfoManager::SceneSortOption::BUTT) {
    qDebug("sortOption[%d] is invalid, cannot used to sort", (int)sortOption);
    return;
  }
  int newBegin{0}, newEnd{0};
  if (mFilterEnable) {
    newBegin = mCurBegin - mEntryListFiltered.cbegin();
    newEnd = mCurEnd - mEntryListFiltered.cbegin();
    SceneInfoManager::sort(mEntryListFiltered, sortOption, reverse);
    mCurBegin = mEntryListFiltered.cbegin() + newBegin;
    mCurEnd = mEntryListFiltered.cbegin() + newEnd;
  } else {
    newBegin = mCurBegin - mEntryList.cbegin();
    newEnd = mCurEnd - mEntryList.cbegin();
    SceneInfoManager::sort(mEntryList, sortOption, reverse);
    mCurBegin = mEntryList.cbegin() + newBegin;
    mCurEnd = mEntryList.cbegin() + newEnd;
  }
  emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1), {Qt::ItemDataRole::DisplayRole, Qt::ItemDataRole::DecorationRole, Qt::ItemDataRole::BackgroundRole});
}

bool ScenesTableModel::ChangeRowsCnt(int newRowCnt, int newPageIndex) {
  if (newRowCnt <= 0 or newPageIndex == -1) {
    qDebug("Invalid row count %d or page index %d", newRowCnt, newPageIndex);
    return true;
  }
  const SCENES_TYPE& lst = GetEntryList();
  const int TOTAL_N = GetEntryListLen();
  const int begin = std::min(mSCENES_CNT_COLUMN * newRowCnt * newPageIndex, TOTAL_N);
  const int end = std::min(mSCENES_CNT_COLUMN * newRowCnt * (newPageIndex + 1), TOTAL_N);
  const int TOTAL_PAGE_IND = end - begin;

  const int beforeRowCnt = rowCount();
  const int afterRowCnt = TOTAL_PAGE_IND / mSCENES_CNT_COLUMN + int(TOTAL_PAGE_IND % mSCENES_CNT_COLUMN != 0);
  const int beforeColumnCnt = columnCount();
  const int afterColumnCnt = mSCENES_CNT_COLUMN;
  qDebug("Change to page %d", newPageIndex);
  qDebug("ChangeRowsCnt. columnCnt: %d->%d", beforeColumnCnt, afterColumnCnt);
  qDebug("ChangeRowsCnt. rowCnt: %d->%d", beforeRowCnt, afterRowCnt);
  ColumnsCountBeginChange(beforeColumnCnt, afterColumnCnt);
  RowsCountBeginChange(beforeRowCnt, afterRowCnt);

  mSCENES_CNT_ROW = newRowCnt;
  mPageIndex = newPageIndex;
  mCurBegin = lst.cbegin() + begin;
  mCurEnd = lst.cbegin() + end;

  RowsCountEndChange();
  ColumnsCountEndChange();

  qDebug("==============dimension: %dx%d==============", rowCount(), columnCount());
  return true;
}

bool ScenesTableModel::ShowAllScenesInOnePage() {
  const int TOTAL_N = GetEntryListLen();

  const int beforeRowCnt = rowCount();
  const int afterRowCnt = TOTAL_N / mSCENES_CNT_COLUMN + int(TOTAL_N % mSCENES_CNT_COLUMN != 0);
  const int beforeColumnCnt = columnCount();
  const int afterColumnCnt = mSCENES_CNT_COLUMN;
  qDebug("Display by page disable  (%d, %d)->(%d, %d)", beforeRowCnt, beforeColumnCnt, afterRowCnt, afterColumnCnt);
  ColumnsCountBeginChange(beforeColumnCnt, afterColumnCnt);
  RowsCountBeginChange(beforeRowCnt, afterRowCnt);

  mSCENES_CNT_ROW = -1;
  mPageIndex = -1;
  const SCENES_TYPE& lst = GetEntryList();
  mCurBegin = lst.cbegin();
  mCurEnd = lst.cend();

  RowsCountEndChange();
  ColumnsCountEndChange();
  return true;
}

bool ScenesTableModel::SetPageIndex(int newPageIndex) {
  if (newPageIndex == -1) {
    qDebug("invalid page index[%d]", newPageIndex);
    return false;
  }
  if (mSCENES_CNT_ROW == -1) {
    qDebug("no need display by page");
    return false;
  }

  const SCENES_TYPE& lst = GetEntryList();
  const int TOTAL_N = GetEntryListLen();
  const int begin = std::min(mSCENES_CNT_COLUMN * mSCENES_CNT_ROW * newPageIndex, TOTAL_N);
  const int end = std::min(mSCENES_CNT_COLUMN * mSCENES_CNT_ROW * (newPageIndex + 1), TOTAL_N);
  const int TOTAL_PAGE_IND = end - begin;

  const int beforeRowCnt = rowCount();
  const int afterRowCnt = TOTAL_PAGE_IND / mSCENES_CNT_COLUMN + int(TOTAL_PAGE_IND % mSCENES_CNT_COLUMN != 0);
  qDebug("SetPageIndex, rowCnt:%d->%d", beforeRowCnt, afterRowCnt);
  RowsCountBeginChange(beforeRowCnt, afterRowCnt);

  mPageIndex = newPageIndex;
  RowsCountEndChange();
  mCurBegin = lst.cbegin() + begin;
  mCurEnd = lst.cbegin() + end;

  emit dataChanged(index(0, 0), index(beforeRowCnt, afterRowCnt - 1), {Qt::ItemDataRole::DisplayRole, Qt::ItemDataRole::DecorationRole, Qt::ItemDataRole::BackgroundRole});
  qDebug("============== SetPageIndex new dimension: %dx%d ==============", rowCount(), columnCount());
  return true;
}
std::pair<int, int> ScenesTableModel::GetEntryIndexBE(int maxLen) const {
  if (mPageIndex == -1 or mSCENES_CNT_ROW == -1) {
    return std::make_pair(0, maxLen);
  }
  const int begin = mSCENES_CNT_COLUMN * mSCENES_CNT_ROW * mPageIndex;
  const int end = mSCENES_CNT_COLUMN * mSCENES_CNT_ROW * (mPageIndex + 1);
  return std::make_pair(std::min(begin, maxLen), std::min(end, maxLen));
}
void ScenesTableModel::setFilterRegularExpression(const QString& pattern) {
  mPattern = pattern;
  if (mPattern.isEmpty()) {
    int newBegin{0}, newEnd{0};
    std::tie(newBegin, newEnd) = GetEntryIndexBE(mEntryList.size());
    const int newScenesCnt = newEnd - newBegin;
    const int beforeRow = rowCount();
    const int afterRow = newScenesCnt / mSCENES_CNT_COLUMN + int(newScenesCnt % mSCENES_CNT_COLUMN != 0);
    RowsCountBeginChange(beforeRow, afterRow);
    mFilterEnable = false;
    mCurBegin = mEntryList.cbegin() + newBegin;
    mCurEnd = mEntryList.cbegin() + newEnd;
    RowsCountEndChange();
    qDebug("============== Filter now disabled, new dimension: %dx%d ==============", rowCount(), columnCount());
    return;
  }

  SCENES_TYPE newCurrentList;
  for (const auto& item : mEntryList) {
    if (item.name.contains(mPattern, Qt::CaseSensitivity::CaseInsensitive)) {
      newCurrentList.append(item);
    }
  }
  int newBegin{0}, newEnd{0};
  std::tie(newBegin, newEnd) = GetEntryIndexBE(newCurrentList.size());
  const int newScenesCnt = newEnd - newBegin;

  const int beforeRow = rowCount();
  const int afterRow = newScenesCnt / mSCENES_CNT_COLUMN + int(newScenesCnt % mSCENES_CNT_COLUMN != 0);
  RowsCountBeginChange(beforeRow, afterRow);
  mEntryListFiltered.swap(newCurrentList);
  mFilterEnable = true;
  mCurBegin = mEntryListFiltered.cbegin() + newBegin;
  mCurEnd = mEntryListFiltered.cbegin() + newEnd;
  RowsCountEndChange();
  qDebug("============== filter now enable pattern[%s], new dimension: %dx%d, scenes count:%d ==============", qPrintable(pattern), rowCount(), columnCount(), newScenesCnt);
}
