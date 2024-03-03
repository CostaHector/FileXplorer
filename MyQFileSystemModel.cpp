#include "MyQFileSystemModel.h"
#include "Tools/PathTool.h"

#include <QDebug>
#include <QFileIconProvider>
#include <QMimeData>
#include <QUrl>

int MyQFileSystemModel::previewsCnt = 0;
constexpr int MyQFileSystemModel::cacheWidth;
constexpr int MyQFileSystemModel::cacheHeight;
constexpr int MyQFileSystemModel::IMAGES_COUNT_LOAD_ONCE_MAX;

MyQFileSystemModel::MyQFileSystemModel(QObject* parent) : QFileSystemModel(parent), _logger(nullptr), m_imagesSizeLoaded(0) {
  setFilter(QDir::Filter::Dirs | QDir::Filter::Files | QDir::Filter::NoDotAndDotDot);

  setReadOnly(true);
  setNameFilterDisables(false);  // gray(True) or hide(False) for items filtered out
  setIconProvider(new QFileIconProvider);

  connect(this, &MyQFileSystemModel::rootPathChanged, this, &MyQFileSystemModel::whenRootPathChanged);
  connect(this, &MyQFileSystemModel::directoryLoaded, this, &MyQFileSystemModel::whenDirectoryLoaded);
}

void MyQFileSystemModel::BindLogger(CustomStatusBar* logger) {
  if (logger == nullptr) {
    qWarning("Bind logger failed. nullptr passed here");
    return;
  }
  _logger = logger;
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
  return index.isValid() and not PATHTOOL::isRootOrEmpty(filePath(index));
}

bool MyQFileSystemModel::canItemsDroppedHere(const QModelIndex& index) const {
  // can dropped: valid and dir | invalid and not empty
  return (index.isValid() and QFileInfo(filePath(index)).isDir()) or (not index.isValid() and not rootPath().isEmpty());
}

bool MyQFileSystemModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const {
  if ((action & supportedDropActions()) and data->hasUrls()) {
    return true;
  }
  return false;
}

bool MyQFileSystemModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
  qDebug() << action << QString("%1 item(s) Drop In MyQFileSystemModel").arg(data->urls().size());
  return true;
}

Qt::DropActions MyQFileSystemModel::supportedDropActions() const {
  return Qt::MoveAction | Qt::CopyAction | Qt::LinkAction;
}

Qt::DropActions MyQFileSystemModel::supportedDragActions() const {
  return Qt::MoveAction | Qt::CopyAction | Qt::LinkAction;
}

#include "PublicVariable.h"

void MyQFileSystemModel::whenRootPathChanged(const QString& newpath) {
  previewsCnt = 0;
  int logicalIndex =
      PreferenceSettings().value(MemoryKey::HEADVIEW_SORT_INDICATOR_LOGICAL_INDEX.name, MemoryKey::HEADVIEW_SORT_INDICATOR_LOGICAL_INDEX.v).toInt();
  const QString& orderString(
      PreferenceSettings().value(MemoryKey::HEADVIEW_SORT_INDICATOR_ORDER.name, MemoryKey::HEADVIEW_SORT_INDICATOR_ORDER.v).toString());
  qDebug() << "FileSystemModel start to sort";
  if (HEADERVIEW_SORT_INDICATOR_ORDER::string2SortOrderEnumListTable.contains(orderString)) {
    sort(logicalIndex, HEADERVIEW_SORT_INDICATOR_ORDER::string2SortOrderEnumListTable[orderString]);
  }
}

void MyQFileSystemModel::whenDirectoryLoaded(const QString& path) {
  QModelIndex currentIndex(index(path));
  int rowCnt = rowCount(currentIndex);
  if (_logger) {
    _logger->pathInfo(rowCnt, 0);
  }
}
