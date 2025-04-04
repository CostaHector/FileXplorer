#include "ScenesListModel.h"
#include "PublicVariable.h"
#include <QObject>
#include <QPixmap>
#include <QDirIterator>
#include <QDir>
#include <QBrush>

ScenesListModel::ScenesListModel(QObject* object)  //
    : QAbstractListModelPub(object) {
  int sceneCnt1Page = PreferenceSettings().value("SCENES_COUNT_EACH_PAGE", 0).toInt();
  SCENES_CNT_1_PAGE = sceneCnt1Page;
}

QVariant ScenesListModel::data(const QModelIndex& index, int role) const {
  if (IsScnsEmpty()) {
    return {};
  }
  int linearInd = 0;
  if (!isIndexValid(index, linearInd)) {
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
      QPixmap pm;
      if (mPixCache.find(imgAbsPath, &pm)) {
        return pm;
      }
      pm = QPixmap{imgAbsPath};
      // w/h > 480/280 = 48 / 28 = 12 / 7
      if (pm.width() * IMAGE_SIZE::IMG_HEIGHT >= pm.height() * IMAGE_SIZE::IMG_WIDTH) {
        pm = pm.scaledToWidth(IMAGE_SIZE::IMG_WIDTH);
      } else {
        pm = pm.scaledToHeight(IMAGE_SIZE::IMG_HEIGHT);
      }
      mPixCache.insert(imgAbsPath, pm);
      return pm;
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

QVariant ScenesListModel::headerData(int section, Qt::Orientation orientation, int role) const {
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
  return QAbstractListModelPub::headerData(section, orientation, role);
}

QFileInfo ScenesListModel::fileInfo(const QModelIndex& index) const {
  int linearInd{-1};
  if (!isIndexValid(index, linearInd)) {
    return {};
  }

  if (mCurBegin[linearInd].vidName.isEmpty()) {
    qDebug("vidName is empty");
    return {};
  }
  return QFileInfo(mRootPath + mCurBegin[linearInd].rel2scn + mCurBegin[linearInd].vidName);
}

QString ScenesListModel::filePath(const QModelIndex& index) const {
  int linearInd{-1};
  if (!isIndexValid(index, linearInd)) {
    return {};
  }

  if (mCurBegin[linearInd].vidName.isEmpty()) {
    qDebug("vidName is empty");
    return {};
  }
  return mRootPath + mCurBegin[linearInd].rel2scn + mCurBegin[linearInd].vidName;
}

QString ScenesListModel::fileName(const QModelIndex& index) const {
  int linearInd{-1};
  if (!isIndexValid(index, linearInd)) {
    return {};
  }
  if (mCurBegin[linearInd].vidName.isEmpty()) {
    qDebug("vidName is empty");
    return {};
  }
  return mCurBegin[linearInd].vidName;
}

QString ScenesListModel::baseName(const QModelIndex& index) const {
  int linearInd{-1};
  if (!isIndexValid(index, linearInd)) {
    return {};
  }
  return mCurBegin[linearInd].name;
}

QString ScenesListModel::absolutePath(const QModelIndex& index) const {
  int linearInd{-1};
  if (!isIndexValid(index, linearInd)) {
    return {};
  }
  return mRootPath + mCurBegin[linearInd].rel2scn;
}

bool ScenesListModel::setRootPath(const QString& rootPath, const bool bForce) {
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

  const int beforeRow = rowCount();
  const int afterRow = newEnd - newBegin;
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
  return true;
}

QStringList ScenesListModel::GetImgs(const QModelIndex& index) const {
  int linearInd{-1};
  if (!isIndexValid(index, linearInd)) {
    return {};
  }
  QStringList imgs;
  imgs.reserve(mCurBegin[linearInd].imgs.size());
  for (const QString& name : mCurBegin[linearInd].imgs) {
    imgs.append(mRootPath + mCurBegin[linearInd].rel2scn + name);
  }
  return imgs;
}

QStringList ScenesListModel::GetVids(const QModelIndex& index) const {
  int linearInd{-1};
  if (!isIndexValid(index, linearInd)) {
    return {};
  }
  QStringList vids;
  vids.append(mRootPath + mCurBegin[linearInd].rel2scn + mCurBegin[linearInd].vidName);
  return vids;
}

bool ScenesListModel::ChangeItemsCntIn1Page(int scCnt1Page) {
  if (scCnt1Page < 0) {
    qDebug("Invalid newSc1Page %d", scCnt1Page);
    return true;
  }

  int beforeRowCnt = rowCount();
  int startIndex{-1}, endIndex{-1};

  const SCENES_TYPE& lst = GetEntryList();
  const int TOTAL_N = GetEntryListLen();
  if (scCnt1Page == 0) {
    qDebug("all items in one page");
    startIndex = 0;
    endIndex = TOTAL_N;
  } else {
    qDebug("%d items in one page", scCnt1Page);
    startIndex = std::min(scCnt1Page * mPageIndex, TOTAL_N);
    endIndex = std::min(scCnt1Page * (mPageIndex + 1), TOTAL_N);
  }

  RowsCountBeginChange(beforeRowCnt, endIndex - startIndex);

  SCENES_CNT_1_PAGE = scCnt1Page;
  mCurBegin = lst.cbegin() + startIndex;
  mCurEnd = lst.cbegin() + endIndex;
  RowsCountEndChange();
  return true;
}

void ScenesListModel::SortOrder(SceneInfoManager::SceneSortOption sortOption, bool reverse) {
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
  emit dataChanged(index(0, 0), index(rowCount() - 1), {Qt::ItemDataRole::DisplayRole, Qt::ItemDataRole::DecorationRole, Qt::ItemDataRole::BackgroundRole});
}

bool ScenesListModel::ShowAllScenesInOnePage() {
  ChangeItemsCntIn1Page(0);
  return true;
}

bool ScenesListModel::SetPageIndex(int newPageIndex) {
  if (SCENES_CNT_1_PAGE == 0) {
    qDebug("display all scenes in 1 page. no need pagination");
    return false;
  }
  if (newPageIndex < 0) {
    qDebug("invalid page index[%d]", newPageIndex);
    return false;
  }
  if (newPageIndex == mPageIndex) {
    qDebug("page index remains[%d]", newPageIndex);
    return true;
  }

  const SCENES_TYPE& lst = GetEntryList();
  const int TOTAL_N = GetEntryListLen();
  const int startIndex = std::min(SCENES_CNT_1_PAGE * newPageIndex, TOTAL_N);
  const int endIndex = std::min(SCENES_CNT_1_PAGE * (newPageIndex + 1), TOTAL_N);

  const int beforeRowCnt = rowCount();
  const int afterRowCnt = endIndex - startIndex;
  qDebug("SetPageIndex, rowCnt:%d->%d", beforeRowCnt, afterRowCnt);
  RowsCountBeginChange(beforeRowCnt, afterRowCnt);
  mPageIndex = newPageIndex;
  mCurBegin = lst.cbegin() + startIndex;
  mCurEnd = lst.cbegin() + endIndex;
  RowsCountEndChange();

  emit dataChanged(index(0, 0), index(beforeRowCnt), {Qt::ItemDataRole::DisplayRole, Qt::ItemDataRole::DecorationRole, Qt::ItemDataRole::BackgroundRole});
  return true;
}

std::pair<int, int> ScenesListModel::GetEntryIndexBE(int maxLen) const {
  if (SCENES_CNT_1_PAGE == 0) {
    return std::make_pair(0, maxLen);
  }
  const int begin = SCENES_CNT_1_PAGE * mPageIndex;
  const int end = SCENES_CNT_1_PAGE * (mPageIndex + 1);
  return std::make_pair(std::min(begin, maxLen), std::min(end, maxLen));
}

void ScenesListModel::setFilterRegularExpression(const QString& pattern) {
  mPattern = pattern;
  if (mPattern.isEmpty()) {
    int newBegin{0}, newEnd{0};
    std::tie(newBegin, newEnd) = GetEntryIndexBE(mEntryList.size());

    const int beforeRow = rowCount();
    const int afterRow = newEnd - newBegin;
    RowsCountBeginChange(beforeRow, afterRow);
    mFilterEnable = false;
    mCurBegin = mEntryList.cbegin() + newBegin;
    mCurEnd = mEntryList.cbegin() + newEnd;
    RowsCountEndChange();
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
  const int beforeRow = rowCount();
  const int afterRow = newEnd - newBegin;
  RowsCountBeginChange(beforeRow, afterRow);
  mEntryListFiltered.swap(newCurrentList);
  mFilterEnable = true;
  mCurBegin = mEntryListFiltered.cbegin() + newBegin;
  mCurEnd = mEntryListFiltered.cbegin() + newEnd;
  RowsCountEndChange();
}
