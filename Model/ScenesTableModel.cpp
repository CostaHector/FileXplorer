#include "ScenesTableModel.h"
#include "PublicVariable.h"
#include <QObject>
#include <QPixmap>
#include <QDirIterator>
#include <QDir>
#include <QBrush>

ScenesTableModel::ScenesTableModel(QObject* object) : QAbstractTableModelPub(object) {}

QVariant ScenesTableModel::data(const QModelIndex& index, int role) const {
  if (not index.isValid()) {
    return {};
  }
  if (mCurBegin == nullptr || mCurEnd == nullptr || mCurBegin == mCurEnd) {
    return {};
  }
  const int linearInd = index.row() * mSCENES_CNT_COLUMN + index.column();
  if (mCurBegin + linearInd >= mCurEnd) {
    return {};
  }
  const QString& nm = mCurBegin[linearInd].filename;
  switch (role) {
    case Qt::ItemDataRole::DisplayRole:
      return QString("%1\n%2\n%3").arg(nm).arg(mCurBegin[linearInd].size).arg(mCurBegin[linearInd].like);
    case Qt::ItemDataRole::DecorationRole: {
      if (nm.endsWith(".jpg", Qt::CaseInsensitive)) {
        QPixmap pm(mRootPath + '/' + nm);
        return pm;
      }
      break;
    }
    case Qt::ItemDataRole::BackgroundRole: {
      if (!mImg2Vid.contains(nm, nullptr)) {
        return QBrush(Qt::GlobalColor::darkGray, Qt::BrushStyle::SolidPattern);
      }
      break;
    }
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
  QString vidFullPath;
  if (mImg2Vid.contains(mCurBegin[linearInd].filename, &vidFullPath)) {
    return vidFullPath;
  }
  qDebug("fileInfo of img in mp4 not find");
  return {};
}

QString ScenesTableModel::filePath(const QModelIndex& index) const {
  int linearInd{-1};
  if (!isIndexValid(index, &linearInd)) {
    return {};
  }

  QString vidFullPath;
  if (mImg2Vid.contains(mCurBegin[linearInd].filename, &vidFullPath)) {
    return vidFullPath;
  }
  qDebug("filePath of img in mp4 not find");
  return {};
}

QString ScenesTableModel::fileName(const QModelIndex& index) const {
  int linearInd{-1};
  if (!isIndexValid(index, &linearInd)) {
    return {};
  }
  QString vidFullPath;
  if (mImg2Vid.contains(mCurBegin[linearInd].filename, &vidFullPath)) {
    return QFileInfo(vidFullPath).fileName();
  }
  qDebug("fileName of img in mp4 not find");
  return {};
}

QString ScenesTableModel::absolutePath(const QModelIndex& index) const {
  int linearInd{-1};
  if (!isIndexValid(index, &linearInd)) {
    return {};
  }

  QString vidFullPath;
  if (mImg2Vid.contains(mCurBegin[linearInd].filename, &vidFullPath)) {
    return QFileInfo(vidFullPath).absolutePath();
  }
  qDebug("filePath of img in mp4 not find");
  return {};
}

bool ScenesTableModel::setRootPath(const QString& rootPath) {
  if (mRootPath == rootPath) {
    qDebug("Scene ignore set same root path");
    return true;
  }
  mRootPath = rootPath;
  QDirIterator imgIt(mRootPath, TYPE_FILTER::IMAGE_TYPE_SET, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot,
                     QDirIterator::IteratorFlag::Subdirectories);
  const int PRE_PATH_STR_LEN = rootPath.size();
  SCENE_INFO_LIST newEntryList, newFilteredList;
  // mRootPath + / + relName
  while (imgIt.hasNext()) {
    const QString& relName = imgIt.next().mid(PRE_PATH_STR_LEN + 1);
    const QFileInfo& fi = imgIt.fileInfo();
    newEntryList.append(SCENE_INFO{relName, fi.size(), fi.size()});
    if (mFilterEnable && relName.contains(mPattern)) {
      newFilteredList.append(newEntryList.back());
    }
  }
  QDirIterator vidIt(mRootPath, TYPE_FILTER::VIDEO_TYPE_SET, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot,
                     QDirIterator::IteratorFlag::Subdirectories);
  while (vidIt.hasNext()) {
    const QString& relName = vidIt.next().mid(PRE_PATH_STR_LEN + 1);
    mImg2Vid.append(relName, vidIt.filePath());
  }

  qDebug("new path[%s], imgs[%d], imgsFiltered[%d], vids[%d]", qPrintable(mRootPath), newEntryList.size(), newFilteredList.size(), mImg2Vid.size());

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
  RowsCountStartChange(beforeRow, afterRow);

  mEntryList.swap(newEntryList);
  mEntryListFiltered.swap(newFilteredList);
  if (mFilterEnable) {
    mCurBegin = mEntryListFiltered.cbegin() + newBegin;
    mCurEnd = mEntryListFiltered.cbegin() + newEnd;
  } else {
    mCurBegin = mEntryList.cbegin() + newBegin;
    mCurEnd = mEntryList.cbegin() + newEnd;
  }

  RowsCountEndChange(beforeRow, afterRow);
  return true;
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
  const SCENE_INFO_LIST& lst = GetEntryList();
  const int TOTAL_N = GetEntryListLen();
  if (mSCENES_CNT_ROW == -1) {
    qDebug("Row count = -1, all items in one page");
    begin = 0;
    end = TOTAL_N - 1;

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

  ColumnsBeginChange(beforeColumnCnt, afterColumnCnt);
  RowsCountStartChange(beforeRowCnt, afterRowCnt);

  mSCENES_CNT_COLUMN = newColumnCnt;
  mPageIndex = newPageIndex;
  mCurBegin = lst.cbegin() + begin;
  mCurEnd = lst.cbegin() + end;

  RowsCountEndChange(beforeRowCnt, afterRowCnt);
  ColumnsEndChange(beforeColumnCnt, afterColumnCnt);

  qDebug("============== ChangeColumnsCnt new dimension: %dx%d ==============", rowCount(), columnCount());
  return true;
}
void ScenesTableModel::SortOrder(bool reverse) {
  std::sort(mEntryList.begin(), mEntryList.end(), [reverse](const SCENE_INFO& lhs, const SCENE_INFO& rhs) -> bool {
    bool less = lhs.filename < rhs.filename;
    return reverse ? !less : less;
  });
  emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1), {});
}

bool ScenesTableModel::ChangeRowsCnt(int newRowCnt, int newPageIndex) {
  if (newRowCnt <= 0 or newPageIndex == -1) {
    qDebug("Invalid row count %d or page index %d", newRowCnt, newPageIndex);
    return true;
  }
  const SCENE_INFO_LIST& lst = GetEntryList();
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
  ColumnsBeginChange(beforeColumnCnt, afterColumnCnt);
  RowsCountStartChange(beforeRowCnt, afterRowCnt);

  mSCENES_CNT_ROW = newRowCnt;
  mPageIndex = newPageIndex;
  mCurBegin = lst.cbegin() + begin;
  mCurEnd = lst.cbegin() + end;

  RowsCountEndChange(beforeRowCnt, afterRowCnt);
  ColumnsEndChange(beforeColumnCnt, afterColumnCnt);

  qDebug("==============dimension: %dx%d==============", rowCount(), columnCount());
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

  const SCENE_INFO_LIST& lst = GetEntryList();
  const int TOTAL_N = GetEntryListLen();
  const int begin = std::min(mSCENES_CNT_COLUMN * mSCENES_CNT_ROW * newPageIndex, TOTAL_N);
  const int end = std::min(mSCENES_CNT_COLUMN * mSCENES_CNT_ROW * (newPageIndex + 1), TOTAL_N);
  const int TOTAL_PAGE_IND = end - begin;

  const int beforeRowCnt = rowCount();
  const int afterRowCnt = TOTAL_PAGE_IND / mSCENES_CNT_COLUMN + int(TOTAL_PAGE_IND % mSCENES_CNT_COLUMN != 0);
  qDebug("SetPageIndex, rowCnt:%d->%d", beforeRowCnt, afterRowCnt);
  RowsCountStartChange(beforeRowCnt, afterRowCnt);

  mPageIndex = newPageIndex;
  RowsCountEndChange(beforeRowCnt, afterRowCnt);
  mCurBegin = lst.cbegin() + begin;
  mCurEnd = lst.cbegin() + end;

  emit dataChanged(index(0, 0), index(beforeRowCnt, afterRowCnt - 1), {});
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
void ScenesTableModel::setFilterRegExp(const QString& pattern) {
  mPattern = pattern;
  if (mPattern.isEmpty()) {
    int newBegin{0}, newEnd{0};
    std::tie(newBegin, newEnd) = GetEntryIndexBE(mEntryList.size());
    const int newScenesCnt = newEnd - newBegin;
    const int beforeRow = rowCount();
    const int afterRow = newScenesCnt / mSCENES_CNT_COLUMN + int(newScenesCnt % mSCENES_CNT_COLUMN != 0);
    RowsCountStartChange(beforeRow, afterRow);
    mFilterEnable = false;
    mCurBegin = mEntryList.cbegin() + newBegin;
    mCurEnd = mEntryList.cbegin() + newEnd;
    RowsCountEndChange(beforeRow, afterRow);
    qDebug("============== Filter now disabled, new dimension: %dx%d ==============", rowCount(), columnCount());
    return;
  }

  SCENE_INFO_LIST newCurrentList;
  for (const auto& item : mEntryList) {
    if (item.filename.contains(mPattern, Qt::CaseSensitivity::CaseInsensitive)) {
      newCurrentList.append(item);
    }
  }
  int newBegin{0}, newEnd{0};
  std::tie(newBegin, newEnd) = GetEntryIndexBE(newCurrentList.size());
  const int newScenesCnt = newEnd - newBegin;

  const int beforeRow = rowCount();
  const int afterRow = newScenesCnt / mSCENES_CNT_COLUMN + int(newScenesCnt % mSCENES_CNT_COLUMN != 0);
  RowsCountStartChange(beforeRow, afterRow);
  mEntryListFiltered.swap(newCurrentList);
  mFilterEnable = true;
  mCurBegin = mEntryListFiltered.cbegin() + newBegin;
  mCurEnd = mEntryListFiltered.cbegin() + newEnd;
  RowsCountEndChange(beforeRow, afterRow);
  qDebug("============== filter now enable pattern[%s], new dimension: %dx%d, scenes count:%d ==============", qPrintable(pattern), rowCount(),
         columnCount(), newScenesCnt);
}
