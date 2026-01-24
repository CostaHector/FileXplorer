#include "ScenesListModel.h"
#include "MemoryKey.h"
#include "StyleSheet.h"
#include "NotificatorMacro.h"
#include "RateHelper.h"
#include "StringTool.h"
#include <QObject>
#include <QPixmap>
#include <QDirIterator>
#include <QDir>
#include <QBrush>

ScenesListModel::ScenesListModel(QObject* object) //
  : QAbstractListModelPub(object) {
  int sceneCnt1Page = Configuration().value("SCENES_COUNT_EACH_PAGE", 0).toInt();
  mScenesCountPerPage = sceneCnt1Page > 0 ? sceneCnt1Page : 1000;
}

bool ScenesListModel::isIndexValid(const QModelIndex& index, int& linearInd) const {
  if (!index.isValid()) {
    return false;
  }
  linearInd = index.row();
  if (mCurBegin + linearInd >= mCurEnd) {
    LOG_W("Invalid index(%d) user input", linearInd);
    return false;
  }
  return true;
}

QVariant ScenesListModel::data(const QModelIndex& index, int role) const {
  int linearInd = -1;
  if (!isIndexValid(index, linearInd)) {
    return {};
  }
  switch (role) {
    case Qt::ItemDataRole::DisplayRole: {
      return mCurBegin[linearInd].name;
    }
    case Qt::ItemDataRole::DecorationRole: {
      const QString imgAbsPath{mCurBegin[linearInd].imgs.isEmpty() ? //
                                   ":img/IMAGE_NOT_FOUND"            //
                                                                   : //
                                   mCurBegin[linearInd].GetFirstImageAbsPath(mRootPath)};
      const QString imgKey{StringTool::PathJoinPixmapSize(imgAbsPath, mWidth, mHeight)};
      QPixmap pm;
      if (mPixCache.find(imgKey, &pm)) {
        return pm;
      }
      if (QFile{imgAbsPath}.size() > 10 * 1024 * 1024) { // 10MB
        return {};                                       // files too large
      }
      if (!pm.load(imgAbsPath)) {
        return {}; // load failed
      }
      if (pm.width() * mHeight >= pm.height() * mWidth) {
        pm = pm.scaledToWidth(mWidth, Qt::FastTransformation);
      } else {
        pm = pm.scaledToHeight(mHeight, Qt::FastTransformation);
      }
      mPixCache.insert(imgKey, pm);
      return pm;
    }
    case Qt::ItemDataRole::BackgroundRole: {
      if (mCurBegin[linearInd].vidName.isEmpty()) {
        return QBrush(Qt::GlobalColor::darkGray, Qt::BrushStyle::SolidPattern);
      }
      break;
    }
    case CustomRoles::RatingRole: {
      return mCurBegin[linearInd].rate;
    }
    default:
      break;
  }
  return {};
}

bool ScenesListModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (role == CustomRoles::RatingRole) {
    return ModifySceneInfoRateValue(index, value.toInt());
  }
  return QAbstractListModel::setData(index, value, role);
}

bool ScenesListModel::ModifySceneInfoRateValue(const QModelIndex& index, int newRate) {
  int linearInd = -1;
  if (!isIndexValid(index, linearInd)) {
    return false;
  }
  int beginIndex = std::distance(mEntryList.cbegin(), mCurBegin);
  int sourceDev = beginIndex + linearInd;
  const int beforeRate = mEntryList[sourceDev].rate;
  if (beforeRate == newRate) {
    LOG_INFO_P("Rate no need change", "value remains %d", beforeRate);
    return true;
  }
  mEntryList[sourceDev].rate = newRate;
  emit dataChanged(index, index, {CustomRoles::RatingRole});

  const QString scnAbsFilePath = GetScn(index);
  const QString eleRel2Scn = mEntryList[sourceDev].rel2scn;
  const QString eleBaseName = mEntryList[sourceDev].name;

  const bool bScnUpdatedOk = SceneHelper::UpdateNameWithNewRate(scnAbsFilePath, eleBaseName, newRate);
  const QString jsonAbsFilePath = GetJson(index);
  const bool bJsonUpdatedOk = RateHelper::RateMovie(jsonAbsFilePath, newRate);
  const bool bothUpdatedOk{bScnUpdatedOk && bJsonUpdatedOk};

  LOG_OE_P(bothUpdatedOk,
           "Rate Modify",
           "[%s%s] from %d to %d [bScnOk: %d, bJsonOk: %d]", //
           qPrintable(eleRel2Scn),
           qPrintable(eleBaseName), //
           beforeRate,
           newRate,
           bScnUpdatedOk,
           bJsonUpdatedOk);
  return bothUpdatedOk;
}

QFileInfo ScenesListModel::fileInfo(const QModelIndex& index) const {
  return QFileInfo(filePath(index));
}

QString ScenesListModel::filePath(const QModelIndex& index) const {
  int linearInd{-1};
  if (!isIndexValid(index, linearInd)) {
    return {};
  }
  return mCurBegin[linearInd].GetVideoAbsPath(mRootPath);
}

QString ScenesListModel::fileName(const QModelIndex& index) const {
  int linearInd{-1};
  if (!isIndexValid(index, linearInd)) {
    return {};
  }
  if (mCurBegin[linearInd].vidName.isEmpty()) {
    LOG_D("vidName is empty");
    return {};
  }
  return mCurBegin[linearInd].vidName;
}

int ScenesListModel::GetRate(const QModelIndex& index) const {
  int linearInd{-1};
  if (!isIndexValid(index, linearInd)) {
    return {};
  }
  if (mCurBegin[linearInd].vidName.isEmpty()) {
    LOG_D("vidName is empty");
    return {};
  }
  return mCurBegin[linearInd].rate;
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
  return mCurBegin[linearInd].GetAbsolutePath(mRootPath);
}

bool ScenesListModel::setRootPath(const QString& rootPath, const bool bForce) {
  if (mRootPath == rootPath && !bForce) {
    LOG_D("Scene ignore set same root path");
    return false;
  }
  mRootPath = rootPath;

  SceneInfoList newEntryList = SceneHelper::GetScnsLstFromPath(mRootPath);
#ifdef RUNNING_UNIT_TESTS
  newEntryList += SceneInfoManager::mockScenesInfoList();
#endif

  LOG_D("new path[%s], imgs[%d]", qPrintable(mRootPath), newEntryList.size());

  const int ELE_N = newEntryList.size();
  int newBegin{0}, newEnd{0};
  std::tie(newBegin, newEnd) = GetEntryIndexBE(mScenesCountPerPage, ELE_N);

  const int beforeRow = rowCount();
  const int afterRow = newEnd - newBegin;
  LOG_D("Entry elements: %d->%d", mEntryList.size(), newEntryList.size());
  LOG_D("setRootPath. RowCountChanged: %d->%d", beforeRow, afterRow);
  RowsCountBeginChange(beforeRow, afterRow);

  mEntryList.swap(newEntryList);
  mCurBegin = mEntryList.cbegin() + newBegin;
  mCurEnd = mEntryList.cbegin() + newEnd;

  RowsCountEndChange();

  emit pagesCountChanged(GetPageCnt());

  return true;
}

QStringList ScenesListModel::GetImgs(const QModelIndex& index) const {
  int linearInd{-1};
  if (!isIndexValid(index, linearInd)) {
    return {};
  }
  return mCurBegin[linearInd].GetImagesAbsPathList(mRootPath);
}

QStringList ScenesListModel::GetVids(const QModelIndex& index) const {
  int linearInd{-1};
  if (!isIndexValid(index, linearInd)) {
    return {};
  }
  return mCurBegin[linearInd].GetVideosAbsPath(mRootPath);
}

QString ScenesListModel::GetJson(const QModelIndex& index) const {
  int linearInd{-1};
  if (!isIndexValid(index, linearInd)) {
    return {};
  }
  return mCurBegin[linearInd].GetJsonAbsPath(mRootPath);
}

QString ScenesListModel::GetScn(const QModelIndex& index) const {
  QString folderPath = absolutePath(index);
  if (folderPath.isEmpty()) {
    return {};
  }
  folderPath.chop(1);
  return SceneInfoManager::ScnMgr::GetScnAbsFilePath(folderPath);
}

std::pair<int, int> ScenesListModel::GetEntryIndexBE(const int scenesCountPerPage, const int maxLen) const {
  if (scenesCountPerPage < 0) {
    return std::make_pair(0, maxLen);
  }
  const int begin = scenesCountPerPage * mPageIndex;
  const int end = scenesCountPerPage * (mPageIndex + 1);
  return std::make_pair(std::min(begin, maxLen), std::min(end, maxLen));
}

void ScenesListModel::onIconSizeChange(const QSize& newSize) {
  if (newSize.width() == mWidth && newSize.height() == mHeight) {
    return;
  }
  mWidth = newSize.width();
  mHeight = newSize.height();
  mPixCache.clear();
}

bool ScenesListModel::onScenesCountsPerPageChanged(int scenesCntInAPage) { // -1 means all, > 0 means count
  const int beforeRowCnt = rowCount();
  int startIndex{-1}, endIndex{-1};

  const SceneInfoList& lst = GetEntryList();
  const int totalScenesCount = GetEntryListLen();
  if (scenesCntInAPage == 0) {
    LOG_W("none in one page");
    startIndex = 0;
    endIndex = 0;
  } else if (scenesCntInAPage < 0) {
    LOG_D("all items in one page");
    startIndex = 0;
    endIndex = totalScenesCount;
  } else {
    LOG_D("%d items in one page", scenesCntInAPage);
    std::tie(startIndex, endIndex) = GetEntryIndexBE(scenesCntInAPage, totalScenesCount);
  }
  const int afterRowCnt = endIndex - startIndex;
  LOG_OK_P("Scenes Count per Page Changed", "%d scenes/Page. rowCnt:%d->%d", scenesCntInAPage, beforeRowCnt, afterRowCnt);
  RowsCountBeginChange(beforeRowCnt, afterRowCnt);

  mScenesCountPerPage = scenesCntInAPage;
  mCurBegin = lst.cbegin() + startIndex;
  mCurEnd = lst.cbegin() + endIndex;

  RowsCountEndChange();

  emit pagesCountChanged(GetPageCnt());
  return true;
}

bool ScenesListModel::onPageIndexChanged(int newPageIndex) {
  if (mScenesCountPerPage < 0) {
    LOG_D("display all scenes in 1 page. no need pagination");
    return true;
  }
  if (newPageIndex < 0) {
    LOG_D("invalid page index[%d]", newPageIndex);
    return false;
  }
  if (newPageIndex == mPageIndex) {
    LOG_D("page index remains[%d]", newPageIndex);
    return true;
  }

  const SceneInfoList& lst = GetEntryList();
  const int TOTAL_N = GetEntryListLen();
  const int startIndex = std::min(mScenesCountPerPage * newPageIndex, TOTAL_N);
  const int endIndex = std::min(mScenesCountPerPage * (newPageIndex + 1), TOTAL_N);

  const int beforeRowCnt = rowCount();
  const int afterRowCnt = endIndex - startIndex;
  LOG_OK_P("Page Changed", "Now page:%d rowCnt:%d->%d", newPageIndex, beforeRowCnt, afterRowCnt);
  // Don't use RowsCountBeginChange, RowsCountEndChange(); here. QPixmap Image varies here
  beginResetModel();
  mPageIndex = newPageIndex;
  mCurBegin = lst.cbegin() + startIndex;
  mCurEnd = lst.cbegin() + endIndex;
  endResetModel();
  return true;
}
