#ifndef DUPLICATEIMAGESMODEL_H
#define DUPLICATEIMAGESMODEL_H

#include "QAbstractTableModelPub.h"
#include "DuplicateImagesHelper.h"

class DuplicateImagesModel : public QAbstractTableModelPub {
public:
  explicit DuplicateImagesModel(QObject* parent = nullptr);
  int setRootPath(const QString& folderPath, DuplicateImageDetectionCriteria::DICriteriaE findBy, bool bIncludeEmptyImage);
  int setFindBy(DuplicateImageDetectionCriteria::DICriteriaE findBy);
  int setIncludeEmptyImg(bool bInclude);
  QString rootPath() const { return mRootPath; }

  int rowCount(const QModelIndex& /*parent*/ = {}) const override { return m_dupImgs.size(); }
  int columnCount(const QModelIndex& /*parent*/ = {}) const override { return DuplicateImageMetaInfo::DI_TABLE_HEADERS_COUNT; }
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  QString filePath(const QModelIndex& index) const;

  bool onDisableImageDecorationChanged(bool bDisabled);
  int UpdateDisplayWhenRecycled(const QModelIndexList& indexes);

  static constexpr int SMALL_PIXMAP_WIDTH{64};

private:
  int setDatas(DuplicateImageMetaInfo::RedundantImagesList& newDatas);

  DuplicateImageMetaInfo::RedundantImagesList m_dupImgs;
  bool m_bDisableImage{false};

  QString mRootPath;
  DuplicateImageDetectionCriteria::DICriteriaE mFindBy{DuplicateImageDetectionCriteria::DICriteriaE::END_INVALID};
  bool m_bIncludeEmptyImage{true};
};

#endif // DUPLICATEIMAGESMODEL_H
