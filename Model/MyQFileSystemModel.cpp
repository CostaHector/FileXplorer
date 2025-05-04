#include "MyQFileSystemModel.h"
#include "public/PathTool.h"
#include "Tools/FileDescriptor/TableFields.h"
#include <QDebug>
#include <QMimeData>
#include <QUrl>

int MyQFileSystemModel::previewsCnt = 0;
constexpr int MyQFileSystemModel::cacheWidth;
constexpr int MyQFileSystemModel::cacheHeight;
constexpr int MyQFileSystemModel::IMAGES_COUNT_LOAD_ONCE_MAX;

MyQFileSystemModel::MyQFileSystemModel(QObject* parent) : QFileSystemModel(parent), _logger(nullptr), m_imagesSizeLoaded(0) {
  setReadOnly(true);

  connect(this, &MyQFileSystemModel::rootPathChanged, this, &MyQFileSystemModel::whenRootPathChanged);
  connect(this, &MyQFileSystemModel::directoryLoaded, this, &MyQFileSystemModel::whenDirectoryLoaded);
}

void MyQFileSystemModel::BindLogger(CustomStatusBar* logger) {
  if (logger == nullptr) {
    qWarning("Don't try to bind nullptr to _logger");
    return;
  }
  if (_logger != nullptr) {
    qWarning("Don't try to rebind logger to non nullptr _logger");
    return;
  }
  _logger = logger;
}

auto MyQFileSystemModel::fullInfo(const QModelIndex& curIndex) const -> QString {
  return data(curIndex.siblingAtColumn(MainKey::Name)).toString()//
         + '\t'//
         + data(curIndex.siblingAtColumn(MainKey::Size)).toString()//
         + '\t'//
         + rootPath();
}

Qt::ItemFlags MyQFileSystemModel::flags(const QModelIndex& index) const {
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

bool MyQFileSystemModel::canItemsBeDragged(const QModelIndex& index) const {
  // can dragged: valid and not root item
  return index.isValid() and not PATHTOOL::isLinuxRootOrWinEmpty(filePath(index));
}

bool MyQFileSystemModel::canItemsDroppedHere(const QModelIndex& index) const {
  // can dropped: valid and dir | invalid and not empty
  return (index.isValid() and QFileInfo(filePath(index)).isDir()) or (not index.isValid() and not rootPath().isEmpty());
}

bool MyQFileSystemModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int /*row*/, int /*column*/, const QModelIndex& /*parent*/) const {
  if ((action & supportedDropActions()) && data->hasUrls()) {
    return true;
  }
  return false;
}

bool MyQFileSystemModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int /*row*/, int /*column*/, const QModelIndex& /*parent*/) {
  qDebug() << action << QString("%1 item(s) Drop In MyQFileSystemModel").arg(data->urls().size());
  return true;
}

Qt::DropActions MyQFileSystemModel::supportedDropActions() const {
  return Qt::MoveAction | Qt::CopyAction | Qt::LinkAction;
}

Qt::DropActions MyQFileSystemModel::supportedDragActions() const {
  return Qt::MoveAction | Qt::CopyAction | Qt::LinkAction;
}

#include "public/PublicVariable.h"

void MyQFileSystemModel::whenRootPathChanged(const QString& /*newpath*/) {
  previewsCnt = 0;
}

void MyQFileSystemModel::whenDirectoryLoaded(const QString& path) {
  QModelIndex currentIndex(index(path));
  int rowCnt = rowCount(currentIndex);
  if (_logger) {
    _logger->pathInfo(rowCnt, 0);
  }
}
