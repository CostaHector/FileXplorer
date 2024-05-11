#include "ImagesFileSystemModel.h"
#include "PublicVariable.h"

ImagesFileSystemModel::ImagesFileSystemModel(QObject* parent, bool showThumbnails_) : MyQFileSystemModel(parent), m_showThumbnails(showThumbnails_) {
  setNameFilters(TYPE_FILTER::IMAGE_TYPE_SET);
  setNameFilterDisables(false);
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
