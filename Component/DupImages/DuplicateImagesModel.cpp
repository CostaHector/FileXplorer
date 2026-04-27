#include "DuplicateImagesModel.h"
#include "PathTool.h"
#include "DataFormatter.h"
#include "ImageTool.h"
#include "RedunImgFinderKey.h"
#include "Configuration.h"
#include "NotificatorMacro.h"
#include "ImagesInfoManager.h"
#include <QFile>

using namespace DuplicateImageMetaInfo;
using namespace DuplicateImageDetectionCriteria;

constexpr int DuplicateImagesModel::SMALL_PIXMAP_WIDTH;

DuplicateImagesModel::DuplicateImagesModel(QObject* parent)
  : QAbstractTableModelPub{parent} {
  m_bDisableImage = getConfig(RedunImgFinderKey::DISABLE_IMAGE_DECORATION).toBool();
}

int DuplicateImagesModel::setRootPath(const QString& folderPath, DICriteriaE findBy, bool bIncludeEmptyImage) {
  if (mRootPath == folderPath && mFindBy == findBy && m_bIncludeEmptyImage == bIncludeEmptyImage) {
    LOG_D("Parms no change, remains rootPath[%s], mFindBy[%d], m_bIncludeEmptyImage[%d]", qPrintable(folderPath), findBy, bIncludeEmptyImage);
    return 0;
  }
  if (PathTool::isRootOrEmpty(folderPath)) {
    LOG_WARN_P("Root path or empty path", "[%s] not accept", qPrintable(folderPath));
    return -1;
  }
  if (!QFileInfo{folderPath}.isDir()) {
    LOG_ERR_P("Not exist", "Path[%s] not exists", qPrintable(folderPath));
    return -1;
  }

  if (!isDICriteriaEValid(findBy)) {
    LOG_ERR_P("findBy[%s] not support", c_str(findBy));
    return -1;
  }

  mRootPath = folderPath;
  mFindBy = findBy;
  m_bIncludeEmptyImage = bIncludeEmptyImage;

  RedundantImagesList newImgs;
  static const ImagesInfoManager& redunLibsInst = ImagesInfoManager::getInst();

  switch (findBy) {
    case DICriteriaE::LIBRARY: {
      newImgs = redunLibsInst.FindRedunImgs(mRootPath, m_bIncludeEmptyImage);
      break;
    }
    case DICriteriaE::MD5: {
      newImgs = ImagesInfoManager::FindDuplicateImgs(mRootPath, m_bIncludeEmptyImage);
      break;
    }
    default: {
      break;
    }
  }
  const int afterRowCnt = setDatas(newImgs);
  LOG_OK_P(mRootPath, "%d images(s) found. searchBy[%s]", afterRowCnt, c_str(findBy));
  return afterRowCnt;
}

int DuplicateImagesModel::setFindBy(DICriteriaE findBy) {
  return setRootPath(rootPath(), findBy, m_bIncludeEmptyImage);
}

int DuplicateImagesModel::setIncludeEmptyImg(bool bInclude) {
  return setRootPath(rootPath(), mFindBy, bInclude);
}

int DuplicateImagesModel::setDatas(DuplicateImageMetaInfo::RedundantImagesList& newDatas) {
  const int beforeRowCnt = m_dupImgs.size();
  const int afterRowCnt = newDatas.size();

  RowsCountBeginChange(beforeRowCnt, afterRowCnt);
  m_dupImgs.swap(newDatas);
  RowsCountEndChange();

  LOG_D("%d->%d duplicates images(s) found", beforeRowCnt, afterRowCnt);
  return afterRowCnt;
}

QVariant DuplicateImagesModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return {};
  }
  const REDUNDANT_IMG_INFO& item = m_dupImgs[index.row()];
  const int col = index.column();
  switch (role) {
    case Qt::DisplayRole: {
      switch (col) {
#define DUP_IMAGE_META_INFO_KEY_ITEM(enu, enumVal, VariableType, formatter) \
  case DuplicateImageMetaInfo::enu: \
    return formatter(item.m_##enu);     //
        DUP_IMAGE_META_INFO_KEY_MAPPING //
#undef DUP_IMAGE_META_INFO_KEY_ITEM     //
            default : return {};
      }
      break;
    }
    case Qt::DecorationRole: {
      if (m_bDisableImage) {
        return {};
      }
      if (col != DuplicateImageMetaInfo::DIColumnE::DUP_IMG_PREVIEW) {
        return {};
      }
      const QString pth = PathTool::GetAbsFilePathFromRootRelName(rootPath(), item.m_RelPath, item.m_Name);
      return ImageTool::GetPixmapFromCached(pth, SMALL_PIXMAP_WIDTH, SMALL_PIXMAP_WIDTH, false);
    }
    default:
      break;
  }
  return {};
}

QVariant DuplicateImagesModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role == Qt::TextAlignmentRole) {
    if (orientation == Qt::Vertical) {
      return Qt::AlignRight;
    }
  } else if (role == Qt::DisplayRole) {
    if (0 <= section && section < columnCount() && orientation == Qt::Orientation::Horizontal) {
      return DI_TABLE_HEADERS[section];
    }
    return section + 1;
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

QString DuplicateImagesModel::filePath(const QModelIndex& index) const {
  if (!index.isValid()) {
    return "";
  }
  const int r = index.row();
  if (r < 0 || r >= rowCount()) {
    LOG_W("r[%d] out of range[0, %d)", r, rowCount());
    return "";
  }
  return PathTool::GetAbsFilePathFromRootRelName(rootPath(), m_dupImgs[r].m_RelPath, m_dupImgs[r].m_Name);
}

bool DuplicateImagesModel::onDisableImageDecorationChanged(bool bDisabled) {
  if (m_bDisableImage == bDisabled) {
    return false;
  }
  const int cnt = rowCount();
  if (cnt == 0) {
    return true;
  }
  m_bDisableImage = bDisabled;
  emit dataChanged(index(0, DuplicateImageMetaInfo::DIColumnE::DUP_IMG_PREVIEW),       //
                   index(cnt - 1, DuplicateImageMetaInfo::DIColumnE::DUP_IMG_PREVIEW), //
                   {Qt::DecorationRole});
  return true;
}

int DuplicateImagesModel::UpdateDisplayWhenRecycled(const QModelIndexList& indexes) {
  const auto rowElementsRmv = [this](int beg, int end) { m_dupImgs.erase(m_dupImgs.begin() + beg, m_dupImgs.begin() + end); };
  return onRowsRemoved(indexes, rowElementsRmv);
}

QItemSelection DuplicateImagesModel::GetSameHashRowWithFirstOneIgnored() const {
  if (rowCount() == 0) {
    return {};
  }
  QItemSelection srcSelection;

  QMap<QByteArray, QList<int>> md5ToRowsDict;
  for (int i = 0; i < rowCount(); ++i) {
    const REDUNDANT_IMG_INFO& info = m_dupImgs[i];
    md5ToRowsDict[info.m_MD5].push_back(i);
  }
  // hash列内容相同时, 会用原始list中的顺序排序, 只保留每批hash中的首行不选, 其它的全选
  const int lastColumn = columnCount() - 1;
  for (auto it = md5ToRowsDict.cbegin(); it != md5ToRowsDict.cend(); ++it) {
    const QList<int>& rows = it.value();
    if (rows.size() < 2) {
      continue;
    }
    // start from 1. ignore first row
    for (int i = 1; i < rows.size(); ++i) {
      srcSelection.select(index(rows[i], 0), index(rows[i], lastColumn));
    }
  }
  return srcSelection;
}
