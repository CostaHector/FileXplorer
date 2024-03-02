#include "ImagesFileSystemModel.h"

ImagesFileSystemModel::ImagesFileSystemModel(QObject* parent, bool showThumbnails_)
    : MyQFileSystemModel(parent), m_showThumbnails(showThumbnails_), IMG_NAME_FILTERS(GetImgNameFiltersList()) {
  setNameFilters(IMG_NAME_FILTERS);
}

QVariant ImagesFileSystemModel::getPreview(QModelIndex index) const {
  QString itemName = QFileSystemModel::data(index, Qt::DisplayRole).toString();
  QPixmap qpm(rootDirectory().absoluteFilePath(itemName));
  if (qpm.isNull()) {
    QIcon ico = QFileSystemModel::data(index, Qt::DecorationRole).value<QIcon>();
    if (not ico.isNull()) {  // QIcon
      qpm = ico.pixmap(cacheWidth, cacheHeight);
    }
    return qpm;
  }
  if (not qpm.isNull()) {
    qpm = qpm.scaledToHeight(cacheHeight, Qt::FastTransformation);
  }
  return qpm;
}

QVariant ImagesFileSystemModel::data(const QModelIndex& index, int role) const {
  if (role == Qt::DisplayRole) {
    return "";
  } else if (m_showThumbnails and role == Qt::DecorationRole) {
    return getPreview(index);  // show thumbnail
  } else {
    return QFileSystemModel::data(index, role);
  }
}

QStringList ImagesFileSystemModel::GetImgNameFiltersList() const {
  QStringList IMG_NAME_FILTERS;
  for (int i = 0; i < IMAGES_COUNT_LOAD_ONCE_MAX; ++i) {
    IMG_NAME_FILTERS += {QString("* %1.jpg").arg(i), QString("* %1.jpg").arg(i), QString("* %1.png").arg(i), QString("* %1.webp").arg(i)};
  }
  return IMG_NAME_FILTERS;
}
