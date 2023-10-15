#include "MyQFileSystemModel.h"

#include <QFileIconProvider>
#include <QMimeData>
#include <QUrl>
#include "FileOperation/FileOperation.h"
#include "UndoRedo.h"

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

bool MyQFileSystemModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    if ((action & supportedDropActions()) and data->hasUrls()){
        return true;
    }
    return false;
}

bool MyQFileSystemModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    QModelIndex toIndex = parent.isValid()? parent: index(row, column, parent);
    const QString& to = filePath(toIndex);
    QStringList selectedItems;
    for (const QUrl& url: data->urls()){
        if (url.isLocalFile()){
            selectedItems.append(url.toLocalFile());
        }
    }
    qDebug("dropMimeData. action=[%d]", int(action));

    FileOperation::BATCH_COMMAND_LIST_TYPE cmds;
    for (const QString& localFile: selectedItems){
        QFileInfo fi(localFile);
        const QString& pre = fi.absolutePath();
        const QString& rel = fi.fileName();
        if (action == Qt::DropAction::MoveAction){
            cmds.append({"rename", pre, rel, to, rel});
        }else if (action == Qt::DropAction::CopyAction){
            if (fi.isFile()){
                cmds.append({"cpfile", pre, rel, to});
            }
            else if (fi.isDir()){
                cmds.append({"cpdir", pre, rel, to});
            } else{
                qDebug("Should never go here [%s]", fi.absoluteFilePath().toStdString().c_str());
            }
        }else if (action == Qt::DropAction::LinkAction){
            cmds.append({"link", pre, rel, to});
        } else{
            qDebug("Action Not Supported");
        }
    }
    if (cmds.isEmpty()){
        return true;
    }
    auto isAllSucceed = g_undoRedo.Do(cmds);
    qDebug("%d Command(s) result: %d", cmds.size(), int(isAllSucceed));
    return isAllSucceed;
}

Qt::DropActions MyQFileSystemModel::supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction | Qt::LinkAction;
}

Qt::DropActions MyQFileSystemModel::supportedDragActions() const
{
    return Qt::MoveAction | Qt::CopyAction | Qt::LinkAction;
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
