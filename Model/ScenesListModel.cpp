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

ScenesListModel::ScenesListModel(const QString& listViewName, QObject* object)  //
    : QAbstractListModelPub{listViewName, object} {
  m_bDisableImage = Configuration().value(SceneKey::SCENE_DISABLE_IMAGE_DECORATION.name, SceneKey::SCENE_DISABLE_IMAGE_DECORATION.v).toBool();

  const int perPageCnt{Configuration().value(SceneKey::SCENES_COUNT_EACH_PAGE.name, SceneKey::SCENES_COUNT_EACH_PAGE.v).toInt()};
  mPagedData.initPerPageCnt(perPageCnt);
  mPagedData.registerCallback(                                                         //
      std::bind(&ScenesListModel::beginResetModel, this),                              //
      std::bind(&ScenesListModel::endResetModel, this),                                //
      std::bind(&ScenesListModel::EmitPagesCountChanged, this, std::placeholders::_1)  //
  );
}

void ScenesListModel::initSortSetting(SceneSortOrderHelper::SortDimE newSortDimension, bool bResultReverse) {
  auto comparator = SceneInfo::getCompareFunc(newSortDimension);
  mPagedData.initSortSetting(comparator, bResultReverse);
}

bool ScenesListModel::setSortDimension(SceneSortOrderHelper::SortDimE newSortDimension) {
  auto comparator = SceneInfo::getCompareFunc(newSortDimension);
  return mPagedData.setSorter(comparator);
}

bool ScenesListModel::setSortResultReverse(bool bResultReverse) {
  return mPagedData.setSortResultReverse(bResultReverse);
}

QVariant ScenesListModel::data(const QModelIndex& index, int role) const {
  int i = -1;
  if (!mPagedData.isLocalIndexValid(index, i)) {
    return {};
  }
  const SceneInfo& item = mPagedData[i];
  switch (role) {
    case Qt::ItemDataRole::DisplayRole: {
      return item.name;
    }
    case Qt::ItemDataRole::DecorationRole: {
      if (m_bDisableImage) {
        return {};
      }
      const QString imgAbsPath{item.imgs.isEmpty() ? ":img/IMAGE_NOT_FOUND" : item.GetFirstImageAbsPath(mRootPath)};
      return GetDecorationPixmap(imgAbsPath);
    }
    case Qt::ItemDataRole::BackgroundRole: {
      if (item.vidName.isEmpty()) {
        return QBrush(Qt::GlobalColor::darkGray, Qt::BrushStyle::SolidPattern);
      }
      break;
    }
    case CustomRole::RatingRole: {
      return item.rate;
    }
    default:
      break;
  }
  return {};
}

bool ScenesListModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (index.isValid() && role == CustomRole::RatingRole) {
    return ModifySceneInfoRateValue(index, value.toInt());
  }
  return QAbstractListModel::setData(index, value, role);
}

bool ScenesListModel::ModifySceneInfoRateValue(const QModelIndex& index, int newRate) {
  int i = -1;
  if (!mPagedData.isLocalIndexValid(index, i)) {
    return false;
  }
  SceneInfo& item = mPagedData[i];
  const int beforeRate = item.rate;
  if (beforeRate == newRate) {
    LOG_INFO_P("Rate no need change", "value remains %d", beforeRate);
    return true;
  }
  item.rate = newRate;
  emit dataChanged(index, index, {CustomRole::RatingRole});

  const QString scnAbsFilePath = GetScn(index);
  const QString eleRel2Scn = item.rel2scn;
  const QString eleBaseName = item.name;

  const bool bScnUpdatedOk = SceneHelper::UpdateNameWithNewRate(scnAbsFilePath, eleBaseName, newRate);
  const QString jsonAbsFilePath = GetJson(index);
  const bool bJsonUpdatedOk = RateHelper::RateMovie(jsonAbsFilePath, newRate);
  const bool bothUpdatedOk{bScnUpdatedOk && bJsonUpdatedOk};

  LOG_OE_P(bothUpdatedOk, "Rate Modify",
           "[%s%s] from %d to %d [bScnOk: %d, bJsonOk: %d]",  //
           qPrintable(eleRel2Scn),
           qPrintable(eleBaseName),  //
           beforeRate, newRate, bScnUpdatedOk, bJsonUpdatedOk);
  return bothUpdatedOk;
}

QFileInfo ScenesListModel::fileInfo(const QModelIndex& index) const {
  return QFileInfo(filePath(index));
}

QString ScenesListModel::filePath(const QModelIndex& index) const {
  int i{-1};
  if (!mPagedData.isLocalIndexValid(index, i)) {
    return {};
  }
  return mPagedData[i].GetVideoAbsPath(mRootPath);
}

QString ScenesListModel::fileName(const QModelIndex& index) const {
  int i{-1};
  if (!mPagedData.isLocalIndexValid(index, i)) {
    return {};
  }
  if (mPagedData[i].vidName.isEmpty()) {
    LOG_D("vidName is empty");
    return {};
  }
  return mPagedData[i].vidName;
}

int ScenesListModel::GetRate(const QModelIndex& index) const {
  int i{-1};
  if (!mPagedData.isLocalIndexValid(index, i)) {
    return 0;
  }
  return mPagedData[i].rate;
}

QString ScenesListModel::baseName(const QModelIndex& index) const {
  int i{-1};
  if (!mPagedData.isLocalIndexValid(index, i)) {
    return {};
  }
  return mPagedData[i].name;
}

QString ScenesListModel::absolutePath(const QModelIndex& index) const {
  int i{-1};
  if (!mPagedData.isLocalIndexValid(index, i)) {
    return {};
  }
  return mPagedData[i].GetAbsolutePath(mRootPath);
}

bool ScenesListModel::setRootPath(const QString& rootPath, const bool bForce) {
  if (mRootPath == rootPath && !bForce) {
    LOG_D("Scene ignore set same root path");
    return false;
  }
  mRootPath = rootPath;
  const SceneInfoList& newEntryList{SceneHelper::GetScnsLstFromPath(mRootPath)};
  mPagedData.setData(newEntryList);
  return true;
}

QStringList ScenesListModel::GetImgs(const QModelIndex& index) const {
  int i{-1};
  if (!mPagedData.isLocalIndexValid(index, i)) {
    return {};
  }
  return mPagedData[i].GetImagesAbsPathList(mRootPath);
}

QStringList ScenesListModel::GetVids(const QModelIndex& index) const {
  int i{-1};
  if (!mPagedData.isLocalIndexValid(index, i)) {
    return {};
  }
  return mPagedData[i].GetVideosAbsPath(mRootPath);
}

QString ScenesListModel::GetJson(const QModelIndex& index) const {
  int i{-1};
  if (!mPagedData.isLocalIndexValid(index, i)) {
    return {};
  }
  return mPagedData[i].GetJsonAbsPath(mRootPath);
}

QString ScenesListModel::GetScn(const QModelIndex& index) const {
  QString folderPath = absolutePath(index);
  if (folderPath.isEmpty()) {
    return {};
  }
  folderPath.chop(1);
  return SceneInfoManager::ScnMgr::GetScnAbsFilePath(folderPath);
}

QStringList ScenesListModel::rel2fileNames(const QModelIndexList& indexes) const {
  // full: "/home/to/a.json"
  // root: "/home"
  // rel2fileNames: "to/a.json"
  QStringList relativePaths2FileName;
  relativePaths2FileName.reserve(indexes.size());
  const int N = rootPath().size();
  for (const QModelIndex& index : indexes) {
    const QString& fullPath = GetJson(index);
    relativePaths2FileName.push_back(fullPath.isEmpty() ? "" : fullPath.mid(N + 1));
  }
  return relativePaths2FileName;
}

bool ScenesListModel::onScenesCountsPerPageChanged(int newPerPage) {
  return mPagedData.setPerPageEleCnt(newPerPage);
}

bool ScenesListModel::onPageIndexChanged(int newPageIndex) {
  return mPagedData.setCurPageIndex(newPageIndex);
}

bool ScenesListModel::onDisableImageDecorationChanged(bool bDisabled) {
  if (m_bDisableImage == bDisabled) {
    return false;
  }
  int cnt = rowCount();
  if (cnt == 0) {
    return true;
  }
  m_bDisableImage = bDisabled;
  emit dataChanged(index(0), index(cnt - 1), {Qt::DecorationRole});
  return true;
}

extern template class PaginatedListRangeEraseGuard<SceneInfo>;
using PaginatedSceneListRangeEraseGuard = PaginatedListRangeEraseGuard<SceneInfo>;

int ScenesListModel::AfterJsonFilesNameRenamed(const QModelIndexList& indexes) {
  if (indexes.isEmpty()) {
    return 0;
  }
  PaginatedSceneListRangeEraseGuard guard(&mPagedData);
  const auto rowElementsRmv = mPagedData.GetRangeEraser();
  const int rowRmvedCnt{onRowsRemoved(indexes, rowElementsRmv)};
  return rowRmvedCnt;
}
