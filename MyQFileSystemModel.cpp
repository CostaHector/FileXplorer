#include "MyQFileSystemModel.h"

#include <QFileIconProvider>
#include <QMimeData>
#include <QUrl>
#include "FileOperation/FileOperation.h"
#include "Tools/PathTool.h"
#include "UndoRedo.h"

int MyQFileSystemModel::previewsCnt = 0;
constexpr int MyQFileSystemModel::cacheWidth;
constexpr int MyQFileSystemModel::cacheHeight;
constexpr int MyQFileSystemModel::IMAGES_COUNT_LOAD_ONCE_MAX;

MyQFileSystemModel::MyQFileSystemModel(CustomStatusBar* _statusBar, QObject* parent)
    : QFileSystemModel(parent), logger(_statusBar), m_imagesSizeLoaded(0) {
  setRootPath("");  // C and D Disk
  setFilter(QDir::Filter::Dirs | QDir::Filter::Files | QDir::Filter::NoDotAndDotDot);

  setReadOnly(true);
  setNameFilterDisables(false);  // gray(True) or hide(False) for items filtered out
  setIconProvider(new QFileIconProvider);

  connect(this, &MyQFileSystemModel::rootPathChanged, this, &MyQFileSystemModel::whenRootPathChanged);
  connect(this, &MyQFileSystemModel::directoryLoaded, this, &MyQFileSystemModel::whenDirectoryLoaded);
}

Qt::ItemFlags MyQFileSystemModel::flags(const QModelIndex& index) const {
  const auto& defaultFlags = QFileSystemModel::flags(index);
  if (not index.isValid()) {
    return Qt::ItemFlag::ItemIsDropEnabled | defaultFlags;
  }
  const QFileInfo itemFi = fileInfo(index);
  if (PATHTOOL::isRootOrEmpty(itemFi.absoluteFilePath())){
    return defaultFlags;
  }
  if (itemFi.isDir()) {  // folders should be be drag/drop enabled
    return Qt::ItemFlag::ItemIsDragEnabled | Qt::ItemFlag::ItemIsDropEnabled | defaultFlags;
  } else if (itemFi.isFile()) {  // files should *not* be drop enabled
    return Qt::ItemFlag::ItemIsDragEnabled | defaultFlags;
  }
  return defaultFlags;
}

bool MyQFileSystemModel::canItemsBeDragged(const QModelIndex& index) const
{
  return index.isValid() and not PATHTOOL::isRootOrEmpty(filePath(index));
}

bool MyQFileSystemModel::canItemsDroppedHere(const QModelIndex& index) const
{
  if (rootPath().isEmpty()){
    return false;
  }
  if (not index.isValid()){
    return true;
  }
  return QFileInfo(filePath(index)).isDir();
}

bool MyQFileSystemModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const {
  if ((action & supportedDropActions()) and data->hasUrls()) {
    return true;
  }
  return false;
}

#include "Component/RenameConflicts.h"

bool MyQFileSystemModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
  QStringList selectedItems;
  selectedItems.reserve(data->urls().size());
  for (const QUrl& url : data->urls()) {
    if (url.isLocalFile()) {
      selectedItems.append(url.toLocalFile());
    }
  }
  if (selectedItems.isEmpty()) {
    return true;
  }

  qDebug() << "dropMimeData. action:" << action;
  CCMMode opMode = CCMMode::ERROR;
  if (action == Qt::DropAction::CopyAction) {
    opMode = CCMMode::COPY;
  } else if (action == Qt::DropAction::MoveAction) {  // move or merge or link
    opMode = CCMMode::CUT;
  } else if (action == Qt::DropAction::LinkAction) {
    opMode = CCMMode::LINK;  // should not conflict
  } else {
    qDebug("[Err] Unknown action[%d]", int(action));
    return false;
  }

  QModelIndex toIndex = parent.isValid() ? parent : index(row, column, parent);
  const QString& to = filePath(toIndex);
  ConflictsItemHelper conflictIF(selectedItems, to);
  auto* tfm = new RenameConflicts(conflictIF, opMode);

  if (to == conflictIF.l and opMode != CCMMode::LINK) {  // skip
    return false;
  }

  if (not conflictIF) {  // conflict
    tfm->on_Submit();
  } else {
    tfm->exec();
  }
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
      PreferenceSettings().value(MemoryKey::HEARVIEW_SORT_INDICATOR_LOGICAL_INDEX.name, MemoryKey::HEARVIEW_SORT_INDICATOR_LOGICAL_INDEX.v).toInt();
  const QString& orderString(
      PreferenceSettings().value(MemoryKey::HEARVIEW_SORT_INDICATOR_ORDER.name, MemoryKey::HEARVIEW_SORT_INDICATOR_ORDER.v).toString());
  if (HEADERVIEW_SORT_INDICATOR_ORDER::string2SortOrderEnumListTable.contains(orderString)) {
    sort(logicalIndex, HEADERVIEW_SORT_INDICATOR_ORDER::string2SortOrderEnumListTable[orderString]);
  }
}

void MyQFileSystemModel::whenDirectoryLoaded(const QString& path) {
  QModelIndex currentIndex(index(path));
  int rowCnt = rowCount(currentIndex);
  if (logger) {
    logger->pathInfo(rowCnt, 0);
  }
}
