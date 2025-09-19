#ifndef REDUNDANTIMAGEMODEL_H
#define REDUNDANTIMAGEMODEL_H

#include "QAbstractTableModelPub.h"
#include "ImagesInfoManager.h"

class RedundantImageFinder;
class RedundantImageModel : public QAbstractTableModelPub {
 public:
  friend class RedundantImageFinder;
  explicit RedundantImageModel(QObject* parent = nullptr);
  auto rowCount(const QModelIndex& /*parent*/ = {}) const -> int override { return m_paf != nullptr ? m_paf->size() : 0; }
  auto columnCount(const QModelIndex& /*parent*/ = {}) const -> int override { return HORIZONTAL_HEADER.size(); }
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  QString filePath(const QModelIndex& index) const;
  void setRootPath(const RedundantImagesList* p_af);

 private:
  const RedundantImagesList* m_paf{nullptr};
  static const QStringList HORIZONTAL_HEADER;
};


#endif // REDUNDANTIMAGEMODEL_H
