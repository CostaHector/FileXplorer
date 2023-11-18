#ifndef IMAGESFILESYSTEMMODEL_H
#define IMAGESFILESYSTEMMODEL_H

#include "MyQFileSystemModel.h"

class ImagesFileSystemModel : public MyQFileSystemModel {
 public:
  explicit ImagesFileSystemModel(QObject* parent = nullptr, bool showThumbnails_ = false);
  QVariant getPreview(QModelIndex index) const;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QStringList GetImgNameFiltersList() const;

 protected:
  bool m_showThumbnails;
  const QStringList IMG_NAME_FILTERS;
};

#endif  // IMAGESFILESYSTEMMODEL_H
