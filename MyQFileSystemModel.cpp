#include "MyQFileSystemModel.h"

#include <QFileIconProvider>

QMap<QString, QPixmap> MyQFileSystemModel::previews;
constexpr int MyQFileSystemModel::cacheWidth;
constexpr int MyQFileSystemModel::cacheHeight;
constexpr int MyQFileSystemModel::IMAGES_SIZE_LOADDABLE_MAX;
constexpr int MyQFileSystemModel::IMAGES_COUNT_LOAD_ONCE_MAX;

MyQFileSystemModel::MyQFileSystemModel(QObject *parent) :
    QFileSystemModel(parent), m_imagesSizeLoaded(0)
{
    setRootPath("");  // C and D Disk
    setFilter(QDir::Filter::Dirs | QDir::Filter::Files | QDir::Filter::NoDotAndDotDot);

    setReadOnly(true);
    setNameFilterDisables(false)  ; // gray(True) or hide(False) for items filtered out
    setIconProvider(new QFileIconProvider);

    connect(this, &MyQFileSystemModel::rootPathChanged, this, &MyQFileSystemModel::whenRootPathChanged);
    connect(this, &MyQFileSystemModel::directoryLoaded, this, &MyQFileSystemModel::whenDirectoryLoaded);
}

Qt::ItemFlags MyQFileSystemModel::flags(const QModelIndex &index) const {
    const auto& defaultFlags = QFileSystemModel::flags(index);
    if (not index.isValid()){
        return Qt::ItemFlag::ItemIsDropEnabled | defaultFlags;
    }
    const QFileInfo& itemFi = fileInfo(index);
    if (itemFi.isDir()){ // folders should be be drag/drop enabled
        return Qt::ItemFlag::ItemIsDragEnabled | Qt::ItemFlag::ItemIsDropEnabled | defaultFlags;
    } else if (itemFi.isFile()){  // files should *not* be drop enabled
        return Qt::ItemFlag::ItemIsDragEnabled | defaultFlags;
    }
    return defaultFlags;
}

#include "PublicVariable.h"

void MyQFileSystemModel::whenRootPathChanged(const QString& newpath){
    previews.clear();
    int logicalIndex = PreferenceSettings().value(MemoryKey::HEARVIEW_SORT_INDICATOR_LOGICAL_INDEX.name, MemoryKey::HEARVIEW_SORT_INDICATOR_LOGICAL_INDEX.v).toInt();
    const QString& orderString(PreferenceSettings().value(MemoryKey::HEARVIEW_SORT_INDICATOR_ORDER.name, MemoryKey::HEARVIEW_SORT_INDICATOR_ORDER.v).toString());
    if (HEADERVIEW_SORT_INDICATOR_ORDER::string2SortOrderEnumListTable.contains(orderString)){
        sort(logicalIndex, HEADERVIEW_SORT_INDICATOR_ORDER::string2SortOrderEnumListTable[orderString]);
    }
}

void MyQFileSystemModel::whenDirectoryLoaded(const QString& path){
    QModelIndex currentIndex(index(path));
    int rowCnt = rowCount(currentIndex);
//    if(logger){
//        emit self.logger.pathInfoChanged(rowCnt, 0)
//    }
}
