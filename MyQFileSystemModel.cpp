#include "MyQFileSystemModel.h"

#include <QFileIconProvider>

QMap<QString, QPixmap> MyQFileSystemModel::previews;
constexpr int MyQFileSystemModel::cacheWidth;
constexpr int MyQFileSystemModel::cacheHeight;
constexpr int MyQFileSystemModel::IMAGES_SIZE_LOADDABLE_MAX;
constexpr int MyQFileSystemModel::IMAGES_COUNT_LOAD_ONCE_MAX;

const QMap<QString, Qt::SortOrder> MyQFileSystemModel::string2SortOrderEnumListTable={{"AscendingOrder",Qt::SortOrder::AscendingOrder},
                                                                                  {"DescendingOrder",Qt::SortOrder::DescendingOrder}};

QString MyQFileSystemModel::SortOrderEnum2String(Qt::SortOrder orderEnum){
    if (string2SortOrderEnumListTable.isEmpty()){
        qDebug("[Error] string2SortOrderListTable is empty");
        return "";
    }
    for (auto it=string2SortOrderEnumListTable.cbegin();it!=string2SortOrderEnumListTable.cend();++it){
        if (it.value() == orderEnum){
            return it.key();
        }
    }
    return string2SortOrderEnumListTable.cbegin().key();
}

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

#include "PublicVariable.h"

void MyQFileSystemModel::whenRootPathChanged(const QString& newpath){
    previews.clear();
    int logicalIndex = PreferenceSettings().value("HEARVIEW_SORT_INDICATOR_LOGICAL_INDEX", 0).toInt();
    const QString& orderString(PreferenceSettings().value("HEARVIEW_SORT_INDICATOR_ORDER", "AscendingOrder").toString());
    if (string2SortOrderEnumListTable.contains(orderString)){
        sort(logicalIndex, string2SortOrderEnumListTable[orderString]);
    }
}

void MyQFileSystemModel::whenDirectoryLoaded(const QString& path){
    QModelIndex currentIndex(index(path));
    int rowCnt = rowCount(currentIndex);
//    if(logger){
//        emit self.logger.pathInfoChanged(rowCnt, 0)
//    }
}
