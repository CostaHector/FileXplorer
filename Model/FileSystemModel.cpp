#include "FileSystemModel.h"
#include "PathTool.h"
#include "TableFields.h"
#include "PublicMacro.h"
#include <QDebug>
#include <QMimeData>
#include <QUrl>
#include <QApplication>
#include <QStyle>

FileSystemModel::FileSystemModel(QObject* parent) : QFileSystemModel{parent} {
  setReadOnly(true);
  connect(this, &FileSystemModel::directoryLoaded, this, &FileSystemModel::whenDirectoryLoaded);
}

void FileSystemModel::BindLogger(CustomStatusBar* pLogger) const {
  CHECK_NULLPTR_RETURN_VOID(pLogger)
  _mPLogger = pLogger;
}

QString FileSystemModel::fullInfo(const QModelIndex& curIndex) const {
  return data(curIndex.siblingAtColumn(MainKey::Name)).toString()    //
         + '\t'                                                      //
         + data(curIndex.siblingAtColumn(MainKey::Size)).toString()  //
         + '\t'                                                      //
         + rootPath();
}

bool FileSystemModel::canDrop(const QModelIndex& targetDropIndex) const {
  return targetDropIndex.isValid() ? fileInfo(targetDropIndex).isDir() : !PathTool::isLinuxRootOrWinEmpty(rootPath());
}

Qt::ItemFlags FileSystemModel::flags(const QModelIndex& index) const {
  Qt::ItemFlags defaultFlags = QFileSystemModel::flags(index);
  defaultFlags |= Qt::ItemIsDragEnabled;
  if (fileInfo(index).isDir()) {
    defaultFlags |= Qt::ItemIsDropEnabled;
  }
  return defaultFlags;
}

bool FileSystemModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const {
  if (!supportedDropActions().testFlag(action)) {
    return false;
  }
  if (!data->hasUrls()) {
    return false;
  }
  // model does not know its view type, we need to find targetDropIndex manually
  QModelIndex targetDropIndex;  // by default: drop in parent
  if (parent.isValid()) {
    if (row >= 0 && column >= 0) {  // table view: parent as root,row/column
      targetDropIndex = index(row, column, parent);
    } else {  // list/tree view:
      targetDropIndex = parent;
    }
  }
  // Only when event position is just at an item, targetDropIndex will be valid. Otherwise targetDropIndex invalid
  // For the former, allowed if item is a directory. and for the latter allowed if rootPath() is "non-empty" (i.e., windows !="" in linux !="/")
  return canDrop(targetDropIndex);
}

bool FileSystemModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int /*row*/, int /*column*/, const QModelIndex& /*parent*/) {
  LOG_D("Action[%d] %d item(s) Drop In FileSystemModel", action, data->urls().size());
  return true;
}

Qt::DropActions FileSystemModel::supportedDropActions() const {
  return Qt::MoveAction | Qt::CopyAction | Qt::LinkAction;
}

Qt::DropActions FileSystemModel::supportedDragActions() const {
  return Qt::MoveAction | Qt::CopyAction | Qt::LinkAction;
}

QVariant FileSystemModel::data(const QModelIndex& index, int role) const {
  if (role == Qt::DecorationRole && index.column() == 0) {
    if (mCutIndexes.contains(rootPath(), index.row())) {
      static const QIcon CUT_ICON{":img/CUT_ITEM"};
      return CUT_ICON;
    } else if (mCopyIndexes.contains(rootPath(), index.row())) {
      static const QIcon COPY_ICON{":img/COPY_ITEM"};
      return COPY_ICON;
    }
  }
  return QFileSystemModel::data(index, role);
}

QVariant FileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Vertical) {
    if (role == Qt::TextAlignmentRole) {
      return Qt::AlignRight;
    } else if (role == Qt::DecorationRole) {
      if (m_draggedHoverIndex.row() == section) {
        static const QIcon DROP_HERE_ICON{":img/DROP_ITEM_HERE"};
        return DROP_HERE_ICON;
      }
    }
  }
  return QFileSystemModel::headerData(section, orientation, role);
}

void FileSystemModel::whenDirectoryLoaded(const QString& path) {
  if (_mPLogger != nullptr) {
    _mPLogger->onPathInfoChanged(rowCount(mRootIndex), 0);
  }
}
