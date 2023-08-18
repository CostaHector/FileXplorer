#include "ImagesFileSystemModel.h"

ImagesFileSystemModel::ImagesFileSystemModel(QObject *parent, bool showThumbnails_) :
    MyQFileSystemModel(parent), m_showThumbnails(showThumbnails_)
{
    setNameFilters({"*.jpeg", "*.jpg", "*.png", "*.tiff", "*.jfif", "*.gif", "*.webp", "*.ico", "*.svg"});
}

QVariant ImagesFileSystemModel::getPreview(QModelIndex index) const{
    QString itemName = QFileSystemModel::data(index, Qt::DisplayRole).toString();
    if (!previews.contains(itemName)){
        if (previews.size() >= IMAGES_COUNT_LOAD_ONCE_MAX){
            return QFileSystemModel::data(index, Qt::DecorationRole);
        }
        QString itemAbsPath(rootPath() + "/" + itemName);
        if (QFile(itemAbsPath).size() >= IMAGES_SIZE_LOADDABLE_MAX){
            return QFileSystemModel::data(index, Qt::DecorationRole);
        }
        QPixmap qpm(itemAbsPath);
        if (qpm.isNull()){
            QIcon ico = QFileSystemModel::data(index, Qt::DecorationRole).value<QIcon>();
            if (not ico.isNull()){ // QIcon
                qpm = ico.pixmap(cacheWidth, cacheHeight);
            }
            return qpm;
        }
        if (not qpm.isNull()){
            qpm = qpm.scaledToHeight(cacheHeight, Qt::FastTransformation);
        }
        previews[itemName] = qpm;
    }
    return previews[itemName];
}

QVariant ImagesFileSystemModel::data(const QModelIndex &index, int role) const{
    if (role == Qt::DisplayRole){
        return "";
    }else if (m_showThumbnails and role == Qt::DecorationRole){
        return getPreview(index); // show thumbnail
    }else{
        return QFileSystemModel::data(index, role);
    }
}
