#ifndef IMAGESFILESYSTEMMODEL_H
#define IMAGESFILESYSTEMMODEL_H

#include "MyQFileSystemModel.h"

#include <QFileIconProvider>
#include <QPixmap>
#include "PublicVariable.h"

class ImgIconProvider : public QFileIconProvider {
 public:
  using QFileIconProvider::icon;

  QIcon icon(const QFileInfo& info) const override{
    const QString& suffix = "*." + info.suffix().toLower();
    if (not TYPE_FILTER::IMAGE_TYPE_SET.contains(suffix)) {
      return QFileIconProvider::icon(info);
    }
    return QIcon(QPixmap{info.absoluteFilePath()});
  }
};

class ImagesFileSystemModel : public MyQFileSystemModel {
 public:
  explicit ImagesFileSystemModel(QObject* parent = nullptr, bool showThumbnails_ = false);

 protected:
  bool m_showThumbnails;
  QFileIconProvider* m_iconProvider;
};

#endif  // IMAGESFILESYSTEMMODEL_H
