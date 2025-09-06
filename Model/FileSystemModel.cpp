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
  return data(curIndex.siblingAtColumn(MainKey::Name)).toString()//
         + '\t'//
         + data(curIndex.siblingAtColumn(MainKey::Size)).toString()//
         + '\t'//
         + rootPath();
}

Qt::ItemFlags FileSystemModel::flags(const QModelIndex& index) const {
  Qt::ItemFlags defaultFlags = QFileSystemModel::flags(index);
  if (canItemsBeDragged(index)) {
    defaultFlags |= Qt::ItemFlag::ItemIsDragEnabled;
  } else {
    defaultFlags &= ~Qt::ItemFlag::ItemIsDragEnabled;
  }
  if (canItemsDroppedHere(index)) {
    defaultFlags |= Qt::ItemFlag::ItemIsDropEnabled;
  } else {
    defaultFlags &= ~Qt::ItemFlag::ItemIsDropEnabled;
  }
  return defaultFlags;
}

bool FileSystemModel::canItemsBeDragged(const QModelIndex& index) const {
  // can dragged: valid and not root item
  return index.isValid() && !PathTool::isLinuxRootOrWinEmpty(filePath(index));
}

bool FileSystemModel::canItemsDroppedHere(const QModelIndex& index) const {
  // can dropped: valid and dir | invalid and not empty
  return (index.isValid() && QFileInfo{filePath(index)}.isDir()) || (!index.isValid() && !rootPath().isEmpty());
}

bool FileSystemModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int /*row*/, int /*column*/, const QModelIndex& /*parent*/) const {
  if ((action & supportedDropActions()) && data->hasUrls()) {
    return true;
  }
  return false;
}

bool FileSystemModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int /*row*/, int /*column*/, const QModelIndex& /*parent*/) {
  qDebug("Action[%d] %d item(s) Drop In FileSystemModel", action, data->urls().size());
  return true;
}

Qt::DropActions FileSystemModel::supportedDropActions() const {
  return Qt::MoveAction | Qt::CopyAction | Qt::LinkAction;
}

Qt::DropActions FileSystemModel::supportedDragActions() const {
  return Qt::MoveAction | Qt::CopyAction | Qt::LinkAction;
}

QVariant FileSystemModel::data(const QModelIndex &index, int role) const {
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
