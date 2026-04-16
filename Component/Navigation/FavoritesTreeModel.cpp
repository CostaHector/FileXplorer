#include "FavoritesTreeModel.h"
#include "MemoryKey.h"
#include "Logger.h"
#include "PublicVariable.h"
#include "PathTool.h"
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <QIcon>

constexpr const char* FavoritesTreeModel::MIME_TYPE;

FavoritesTreeModel::FavoritesTreeModel(const QString& belongToName, QObject* parent, bool bInitialCollectionsWhenEmpty)  //
    : QAbstractTreeModelPub<FavTreeNode>{parent}, m_belongToName{belongToName} {
  const QByteArray& datas = Configuration().value(GetDataKeyInQSetting(), QByteArray{}).toByteArray();
  if (!datas.isEmpty() && setDatas(datas)) {
    // Key exist and ByteArray not empty and valid(may contains 0 elements)
    clearDirty();
    return;
  }
  setDatas(QByteArray{});
  if (!bInitialCollectionsWhenEmpty) {
    return;
  }
  addInitialFavoritesGroup();
  clearDirty();
}

FavoritesTreeModel::~FavoritesTreeModel() {
  if (mNotSaveDatasThisTimeBeforeDestruct) {
    return;
  }
  if (isDirty()) {
    saveToSettings();
  }
}

QVariant FavoritesTreeModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return {};
  }

  FavTreeNode* node = static_cast<FavTreeNode*>(index.internalPointer());
  if (!node) {
    return {};
  }

  const FavoriteItemData& item = node->value();
  const int column = index.column();
  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    switch (column) {
      case FavoriteItemData::DEF_NAME_TEXT_ROLE - FavoriteItemData::DEF_BEGIN_ROLE:
        return item.name;
      case FavoriteItemData::FULL_PATH_ROLE - FavoriteItemData::DEF_BEGIN_ROLE:
        return item.fullPath;
      case FavoriteItemData::IS_GROUP_ROLE - FavoriteItemData::DEF_BEGIN_ROLE:
        return item.isGroup;
      case FavoriteItemData::LAST_ACCESS_ROLE - FavoriteItemData::DEF_BEGIN_ROLE:
        return item.lastAccess;
      case FavoriteItemData::ACCESS_COUNT_ROLE - FavoriteItemData::DEF_BEGIN_ROLE:
        return item.accessCount;
      default:
        return {};
    }
  } else if (role == Qt::DecorationRole) {
    if (column == 0 && !item.isGroup) {
      static const QIcon favNonGroupIcon{":img/FAVORITES_LINK"};
      return favNonGroupIcon;
    }
    return {};
  } else {
    switch (role) {
      case FavoriteItemData::DEF_NAME_TEXT_ROLE:
        return item.name;
      case Qt::ToolTipRole:
      case FavoriteItemData::FULL_PATH_ROLE:
        return item.fullPath;
      case FavoriteItemData::IS_GROUP_ROLE:
        return item.isGroup;
      case FavoriteItemData::LAST_ACCESS_ROLE:
        return item.lastAccess;
      case FavoriteItemData::ACCESS_COUNT_ROLE:
        return item.accessCount;
      default:
        return {};
    }
  }

  return {};
}

bool FavoritesTreeModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (!index.isValid()) {
    return false;
  }

  if (role == Qt::EditRole) {
    FavTreeNode* node = static_cast<FavTreeNode*>(index.internalPointer());
    if (node == nullptr) {
      return false;
    }
    if (!node->setName(value.toString())) {
      return false;
    }
    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    return true;
  }
  return QAbstractTreeModelPub::setData(index, value, role);
}

Qt::ItemFlags FavoritesTreeModel::flags(const QModelIndex& index) const {
  // root: drop only
  if (!index.isValid()) {
    return Qt::ItemIsDropEnabled;
  }
  // group: drag and drop
  if (isGroup(index)) {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
  }
  // nongroup, drag only
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
}

Qt::DropActions FavoritesTreeModel::supportedDropActions() const {
  return Qt::MoveAction | Qt::CopyAction;
}

Qt::DropActions FavoritesTreeModel::supportedDragActions() const {
  return Qt::MoveAction | Qt::CopyAction;
}

QStringList FavoritesTreeModel::mimeTypes() const {
  static const QStringList types{MIME_TYPE};
  return types;
}

bool FavoritesTreeModel::canDropOn(const QModelIndex& index) const {
  return flags(index).testFlag(Qt::ItemIsDropEnabled);
}

QMimeData* FavoritesTreeModel::mimeData(const QModelIndexList& indexes) const {
  if (indexes.isEmpty()) {
    return nullptr;
  }

  // 只处理首列
  QModelIndexList firstColumnIndexes;
  for (const QModelIndex& index : indexes) {
    if (index.column() == 0) {
      firstColumnIndexes.append(index);
    }
  }

  if (firstColumnIndexes.isEmpty()) {
    return nullptr;
  }

  // 序列化选中的项
  QByteArray data;
  QDataStream stream(&data, QIODevice::WriteOnly);
  stream << GetVersion();

  // 存储每个选中项 从该项到根的索引
  for (const QModelIndex& index : firstColumnIndexes) {
    QList<int> pathFromCurrentToRoot;
    QModelIndex current = index;
    while (current.isValid()) {
      pathFromCurrentToRoot.push_back(current.row());
      current = current.parent();
    }
    stream << pathFromCurrentToRoot;
  }

  QMimeData* mimeData = new QMimeData();
  mimeData->setData(MIME_TYPE, data);
  return mimeData;
}

bool FavoritesTreeModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& dstParent) const {
  if (action == Qt::IgnoreAction) {
    return true;
  }
  if (!supportedDropActions().testFlag(action)) {
    return false;
  }

  const bool hasInternalMime{data->hasFormat(MIME_TYPE)};
  const bool hasExternalUrls{data->hasUrls()};
  if (!hasInternalMime && !hasExternalUrls) {
    return false;
  }

  // 检查目标位置是否有效
  if (!canDropOn(dstParent)) {
    return false;
  }

  // 如果拖动到项上（不是在项之间），row 为 -1
  if (row == -1) {
    // 拖放到项上
    if (!canDropOn(dstParent)) {
      return false;
    }
  }

  return true;
}

// 处理拖放
bool FavoritesTreeModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& dstParent) {
  if (action == Qt::IgnoreAction) {
    return true;
  }
  if (column > 0) {
    return false;
  }

  if (data->hasFormat(MIME_TYPE)) {
    // 处理自定义 MIME 类型（内部拖放）
    return handleInternalDrop(data, action, row, column, dstParent) > 0;
  } else if (data->hasUrls()) {
    // 处理外部 URL 拖放（如资源管理器中的文件夹）
    return handleExternalDrop(data, action, row, column, dstParent) > 0;
  }

  return false;
}

QString FavoritesTreeModel::filePath(const QModelIndex& parentIndex) const {
  if (!parentIndex.isValid()) {
    return "";
  }
  FavTreeNode* item = itemFromIndex(parentIndex);
  if (item == nullptr) {
    return "";
  }
  if (item->value().isGroup) {
    return "";
  }
  return item->value().fullPath;
}

FavTreeNode* FavoritesTreeModel::addPath(const QString& name, const QString& path, const QModelIndex& parentIndex) {
  FavTreeNode* parentItem = nullptr;
  if (parentIndex.isValid()) {
    parentItem = itemFromIndex(parentIndex);
    if (parentItem == nullptr) {
      LOG_W("Cannot get parent item from index");
      return nullptr;
    }
  }
  return addPath(name, path, parentItem);
}

FavTreeNode* FavoritesTreeModel::addPath(const QString& name, const QString& path, FavTreeNode* parentItem) {
  if (parentItem) {
    if (!parentItem->value().isGroup) {
      LOG_D("Cannot insert under non-group item");
      return nullptr;
    }
  } else {
    parentItem = invisibleRootItem();
  }
  QModelIndex parentIndex = indexFromItem(parentItem);
  beginInsertRows(parentIndex, parentItem->rowCount(), parentItem->rowCount());

  auto childNode = parentItem->appendRow(FavTreeNode::create(FavoriteItemData{name, path}));
  endInsertRows();
  setDirty();
  return childNode;
}

void FavoritesTreeModel::addInitialFavoritesGroup() {
  // initial configs
  FavTreeNode* workGroup = addGroup(tr("Work"), nullptr);
  addPath("Documents", SystemPath::HOME_PATH() + "/Documents", workGroup);
  addPath("Project Configurations", SystemPath::HOME_PATH(), workGroup);

  FavTreeNode* lifeGroup = addGroup(tr("Life"), nullptr);
  addPath("Pictures", SystemPath::HOME_PATH() + "/Pictures", lifeGroup);
  addPath("Videos", SystemPath::HOME_PATH() + "/Videos", lifeGroup);

  addPath("Code", SystemPath::HOME_PATH() + "/code", nullptr);
}

void FavoritesTreeModel::saveToSettings() {
  Configuration().setValue(GetDataKeyInQSetting(), toByteArray());
  clearDirty();
}

int FavoritesTreeModel::handleExternalDrop(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& dstParent) {
  const QList<QUrl>& urlList{data->urls()};

  int addedCount = 0;
  foreach (const QUrl& url, urlList) {
    QString fullPath{url.toLocalFile()};
    if (fullPath.isEmpty()) {
      continue;
    }
    const QFileInfo fi{fullPath};
    if (!fi.isDir()) {
      fullPath = fi.absolutePath();
    }
    const QString nameTextShown{PathTool::GetBaseName(fullPath)};
    FavTreeNode* addedItem = addPath(nameTextShown, fullPath, dstParent);
    if (addedItem != nullptr) {
      addedCount++;
    }
  }
  setDirty();
  return addedCount;
}

int FavoritesTreeModel::handleInternalDrop(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& dstParent) {
  QByteArray encodedData = data->data(MIME_TYPE);
  QDataStream stream(&encodedData, QIODevice::ReadOnly);

  quint16 version;
  stream >> version;
  if (version != GetVersion()) {
    return false;
  }

  // 获取所有被拖动的项的索引
  QModelIndexList draggedIndexes;
  while (!stream.atEnd()) {
    QList<int> pathFromCurrentToRoot;
    stream >> pathFromCurrentToRoot;
    if (pathFromCurrentToRoot.isEmpty()) {
      continue;
    }
    // 根据路径重建索引
    QModelIndex selectIndex;
    selectIndex = index(pathFromCurrentToRoot.back(), 0, QModelIndex());
    for (auto it = pathFromCurrentToRoot.crbegin() + 1; it < pathFromCurrentToRoot.crend(); ++it) {
      selectIndex = index(*it, 0, selectIndex);
      if (!selectIndex.isValid()) {
        break;
      }
    }
    if (selectIndex.isValid()) {
      draggedIndexes.append(selectIndex);
    }
  }

  if (draggedIndexes.isEmpty()) {
    return false;
  }

  // 检查是否试图拖放到自身或后代
  for (const QModelIndex& selIndex : draggedIndexes) {
    if (selIndex == dstParent) {
      return false;
    }
    // cannot select father and move inside son. infinite loop!
    bool bSelectedFather = isIndexValidAndDescendantOfValidAncestor(dstParent, selIndex);
    if (bSelectedFather) {
      return false;
    }
  }
  setDirty();
  return moveParentIndexesTo(draggedIndexes, dstParent);
}
