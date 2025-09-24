#ifndef REDUNDANTIMAGEMODEL_H
#define REDUNDANTIMAGEMODEL_H

#include "QAbstractTableModelPub.h"
#include "ImagesInfoManager.h"

class RedundantImageFinder;
class RedundantImageModel : public QAbstractTableModelPub {
 public:
  friend class RedundantImageFinder;
  using QAbstractTableModelPub::QAbstractTableModelPub;
  auto rowCount(const QModelIndex& /*parent*/ = {}) const -> int override { return m_paf != nullptr ? m_paf->size() : 0; }
  auto columnCount(const QModelIndex& /*parent*/ = {}) const -> int override { return DuplicateImageMetaInfo::DI_TABLE_HEADERS_COUNT; }
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  QString filePath(const QModelIndex& index) const;
  void setRootPath(const DuplicateImageMetaInfo::RedundantImagesList* p_af);

 private:
  const DuplicateImageMetaInfo::RedundantImagesList* m_paf{nullptr};
};


#endif // REDUNDANTIMAGEMODEL_H
