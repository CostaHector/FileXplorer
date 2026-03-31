#include "FavoritesTreeModel.h"
#include "MemoryKey.h"
#include "Logger.h"
#include "PublicVariable.h"
#include "PathTool.h"
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>

constexpr quint16 FavoritesTreeModel::VERSION;
constexpr const char* FavoritesTreeModel::MIME_TYPE;

FavoritesTreeModel::FavoritesTreeModel(const QString& belongToName, QObject* parent, bool bInitialCollectionsWhenEmpty)  //
    : QStandardItemModel{0, 1, parent}, m_belongToName{belongToName} {
  setHorizontalHeaderLabels({"Favorites"});

  const QByteArray& datas = Configuration().value(GetDataKeyInQSetting(), QByteArray{}).toByteArray();
  if (!datas.isEmpty() && setDatas(datas)) {
    // Key exist and ByteArray not empty and valid(may contains 0 elements)
    return;
  }
  if (!bInitialCollectionsWhenEmpty) {
    return;
  }
  addInitialFavoritesGroup();
}

FavoritesTreeModel::~FavoritesTreeModel() {
  if (mNotSaveDatasThisTimeBeforeDestruct) {
    return;
  }
  saveToSettings();
}

bool FavoritesTreeModel::setDatas(const QVector<FavoriteItemData>& topLevelItems) {
  beginResetModel();
  setRowCount(0);
  for (const FavoriteItemData& itemData : topLevelItems) {
    QStandardItem* item = convertDataToItem(itemData);
    if (item) {
      invisibleRootItem()->appendRow(item);
    }
  }
  endResetModel();
  return true;
}

bool FavoritesTreeModel::setDatas(const QByteArray& dataByteArray) {
  QVector<FavoriteItemData> topLevelItems;
  if (!fromByteArray(dataByteArray, topLevelItems)) {
    return false;
  }
  return setDatas(topLevelItems);
}

Qt::ItemFlags FavoritesTreeModel::flags(const QModelIndex& index) const {
  static const Qt::ItemFlags defaultFlags{QStandardItemModel::flags(index) & ~(Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled)};

  // root: drop only
  if (!index.isValid()) {
    return defaultFlags | Qt::ItemIsDropEnabled;
  }
  // group: drag and drop
  if (isGroup(index)) {
    return defaultFlags | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
  }
  // nongroup, drag only
  return defaultFlags | Qt::ItemIsDragEnabled;
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

bool FavoritesTreeModel::isIndexValidAndDescendantOfValidAncestor(const QModelIndex& descendant, const QModelIndex& father) {
  if (!descendant.isValid() || !father.isValid()) {
    return false;
  }

  QModelIndex parent = descendant.parent();
  while (parent.isValid()) {
    if (parent == father) {
      return true;
    }
    parent = parent.parent();
  }
  return false;
}

QMimeData* FavoritesTreeModel::mimeData(const QModelIndexList& indexes) const {
  if (indexes.isEmpty()) {
    return nullptr;
  }

  // 只处理第一列
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
  stream << static_cast<quint16>(VERSION);

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

QByteArray FavoritesTreeModel::toByteArray() const {
  QByteArray data;
  QDataStream stream(&data, QIODevice::WriteOnly);
  stream.setVersion(QDataStream::Qt_5_15);
  stream << VERSION;

  // 转换所有顶级项
  QVector<FavoriteItemData> topLevelItems;
  QStandardItem* root = invisibleRootItem();

  for (int i = 0; i < root->rowCount(); ++i) {
    QStandardItem* item = root->child(i);
    if (item) {
      topLevelItems.append(convertItemToData(item));
    }
  }

  stream << topLevelItems;
  return data;
}

bool FavoritesTreeModel::fromByteArray(const QByteArray& data, QVector<FavoriteItemData>& topLevelItems) {
  topLevelItems.clear();
  if (data.isEmpty()) {  // used in clear all rows
    return true;
  }

  QDataStream stream(data);
  stream.setVersion(QDataStream::Qt_5_15);

  quint16 version{0};
  stream >> version;
  if (version != VERSION) {
    LOG_D("Version dismatch. expect:%u, actual:%d", VERSION, version);
    return false;
  }
  stream >> topLevelItems;
  if (stream.status() != QDataStream::Ok) {
    LOG_W("read stream failed");
    return false;
  }

  return true;
}

QString FavoritesTreeModel::filePath(const QModelIndex& parentIndex) const {
  if (!parentIndex.isValid()) {
    return "";
  }
  QStandardItem* item = itemFromIndex(parentIndex);
  if (item == nullptr) {
    return "";
  }
  if (item->data(FavoriteItemData::Role::IS_GROUP_ROLE).toBool()) {
    return "";
  }
  return item->data(FavoriteItemData::Role::FULL_PATH_ROLE).toString();
}

bool FavoritesTreeModel::isGroup(const QModelIndex& parentIndex) const {
  if (isRoot(parentIndex)) {
    return false;
  }
  QStandardItem* item = itemFromIndex(parentIndex);
  if (item == nullptr) {
    return false;  // failed
  }
  return item->data(FavoriteItemData::Role::IS_GROUP_ROLE).toBool();
}

QString FavoritesTreeModel::groupName(const QModelIndex& parentIndex) const {
  QStandardItem* item = itemFromIndex(parentIndex);
  if (item == nullptr) {
    return "";  // failed
  }
  return item->text();
}

FavoriteItemData FavoritesTreeModel::convertItemToData(QStandardItem* item) {
  if (!item) {
    return {};
  }

  FavoriteItemData data;
  data.name = item->text();
  data.isGroup = item->data(FavoriteItemData::Role::IS_GROUP_ROLE).toBool();
  data.fullPath = item->data(FavoriteItemData::Role::FULL_PATH_ROLE).toString();
  data.lastAccess = item->data(FavoriteItemData::Role::LAST_ACCESS_ROLE).toInt();
  data.accessCount = item->data(FavoriteItemData::Role::ACCESS_COUNT_ROLE).toInt();

  // 递归处理子项
  for (int i = 0; i < item->rowCount(); ++i) {
    QStandardItem* child = item->child(i);
    if (child) {
      data.children.append(convertItemToData(child));
    }
  }

  return data;
}

QStandardItem* FavoritesTreeModel::convertDataToItem(const FavoriteItemData& data) {
  if (!data.isValid()) {
    return nullptr;
  }

  QStandardItem* item = nullptr;
  if (data.isGroup) {
    item = new QStandardItem(data.name);
  } else {
    item = new QStandardItem(QIcon(":img/FAVORITES_LINK"), data.name);
    item->setToolTip(QString("<b>%1</b><br>%2").arg(data.name, data.fullPath));
  }
  item->setEditable(true);  // 允许重命名
  item->setData(data.isGroup, FavoriteItemData::Role::IS_GROUP_ROLE);
  item->setData(data.fullPath, FavoriteItemData::Role::FULL_PATH_ROLE);
  item->setData(data.lastAccess, FavoriteItemData::Role::LAST_ACCESS_ROLE);
  item->setData(data.accessCount, FavoriteItemData::Role::ACCESS_COUNT_ROLE);

  // 递归创建子项
  for (const FavoriteItemData& childData : data.children) {
    QStandardItem* child = convertDataToItem(childData);
    if (child) {
      item->appendRow(child);
    }
  }

  return item;
}

QStandardItem* FavoritesTreeModel::addGroup(const QString& grpName, const QModelIndex& parentIndex) {
  QStandardItem* parentItem = nullptr;
  if (parentIndex.isValid()) {
    parentItem = itemFromIndex(parentIndex);
    if (parentItem == nullptr) {
      LOG_W("Cannot get parent item from index");
      return nullptr;
    }
  }
  return addGroup(grpName, parentItem);
}

QStandardItem* FavoritesTreeModel::addGroup(const QString& grpName, QStandardItem* parentItem) {
  if (parentItem) {
    if (!parentItem->data(FavoriteItemData::Role::IS_GROUP_ROLE).toBool()) {
      LOG_D("Cannot insert under non-group item");
      return nullptr;
    }
  } else {
    parentItem = invisibleRootItem();
  }
  QStandardItem* item = new QStandardItem(grpName);
  item->setData(true, FavoriteItemData::Role::IS_GROUP_ROLE);
  item->setData("", FavoriteItemData::Role::FULL_PATH_ROLE);
  item->setEditable(true);
  parentItem->appendRow(item);
  LOG_D("Group[%s] added successfully", qPrintable(grpName));
  return item;
}

QStandardItem* FavoritesTreeModel::addPath(const QString& name, const QString& path, const QModelIndex& parentIndex) {
  QStandardItem* parentItem = nullptr;
  if (parentIndex.isValid()) {
    parentItem = itemFromIndex(parentIndex);
    if (parentItem == nullptr) {
      LOG_W("Cannot get parent item from index");
      return nullptr;
    }
  }
  return addPath(name, path, parentItem);
}

QStandardItem* FavoritesTreeModel::addPath(const QString& name, const QString& path, QStandardItem* parentItem) {
  if (parentItem) {
    if (!parentItem->data(FavoriteItemData::Role::IS_GROUP_ROLE).toBool()) {
      LOG_D("Cannot insert under non-group item");
      return nullptr;
    }
  } else {
    parentItem = invisibleRootItem();
  }
  QStandardItem* item = new QStandardItem(QIcon(":img/FAVORITES_LINK"), name);
  item->setData(false, FavoriteItemData::Role::IS_GROUP_ROLE);
  item->setData(path, FavoriteItemData::Role::FULL_PATH_ROLE);
  item->setEditable(true);  // 允许重命名
  item->setToolTip(QString("<b>%1</b><br>%2").arg(name, path));
  parentItem->appendRow(item);
  return item;
}

// 去除冗余, 先按照父索引排序, 在按照行号降序列
QList<QStandardItem*> FavoritesTreeModel::GetItemsNeedProcess(const QModelIndexList& parentIndexes, QStandardItem* destItem) const {
  const QStandardItem* rootItem = invisibleRootItem();

  QList<QStandardItem*> allItems;
  {
    // 收集要移动的项，去重
    QSet<QStandardItem*> uniqueItems;

    QList<QStandardItem*> itemsToDelete;
    QSet<QStandardItem*> groupsToDelete;
    for (const QModelIndex& idx : parentIndexes) {
      if (!idx.isValid()) {
        continue;
      }
      QStandardItem* item = itemFromIndex(idx);
      if (item == nullptr || item == rootItem || uniqueItems.contains(item)) {
        continue;
      }
      if (item == destItem) {
        LOG_W("Cannot move rows to itself");
        return {};
      }
      uniqueItems.insert(item);

      if (item->data(FavoriteItemData::Role::IS_GROUP_ROLE).toBool()) {
        groupsToDelete.insert(item);
      } else {
        itemsToDelete.append(item);
      }
    }

    // 非分组在要移动的分组中时, 不需要单独移动它, 避免重复移动
    for (QStandardItem* item : itemsToDelete) {
      bool bExistInGroup = false;
      QStandardItem* ancestor = item->parent();
      while (ancestor != nullptr && ancestor != rootItem) {
        if (groupsToDelete.contains(ancestor)) {
          bExistInGroup = true;
          break;
        }
        ancestor = ancestor->parent();
      }
      if (!bExistInGroup) {
        allItems.append(item);
      }
    }

    // 分组在要移动的分组中时, 不需要单独移动它, 避免重复移动
    for (QStandardItem* item : groupsToDelete) {
      bool bExistInGroup = false;
      QStandardItem* ancestor = item->parent();
      while (ancestor != nullptr && ancestor != rootItem) {
        if (groupsToDelete.contains(ancestor)) {
          bExistInGroup = true;
          break;
        }
        ancestor = ancestor->parent();
      }
      if (!bExistInGroup) {
        allItems.append(item);
      }
    }
  }
  if (allItems.isEmpty()) {
    return {};
  }

  // 按父项和行号排序, 让相同父项的项在一起, 按行号从大到小排序
  std::sort(allItems.begin(), allItems.end(), [rootItem](QStandardItem* a, QStandardItem* b) {
    const QStandardItem* parentA = a->parent();
    if (parentA == nullptr) {
      parentA = rootItem;
    }
    const QStandardItem* parentB = b->parent();
    if (parentB == nullptr) {
      parentB = rootItem;
    }
    return parentA != parentB ? parentA < parentB : a->row() > b->row();
  });

  return allItems;
}

int FavoritesTreeModel::moveParentIndexesTo(const QModelIndexList& parentIndexes, const QModelIndex& dest) {
  if (parentIndexes.isEmpty()) {
    return 0;
  }
  QStandardItem* destItem = nullptr;
  if (dest.isValid()) {
    if (!isGroup(dest)) {
      LOG_D("Can only move to group or root");
      return -1;
    }
    destItem = itemFromIndex(dest);
  } else {
    destItem = invisibleRootItem();
  }

  QList<QStandardItem*> allItems = GetItemsNeedProcess(parentIndexes, destItem);
  if (allItems.isEmpty()) {
    return -1;
  }
  // 开始移动
  int succeedCnt{0};
  QStandardItem* notConstRootItem = invisibleRootItem();
  for (QStandardItem* item : allItems) {
    QStandardItem* parent = item->parent();
    if (parent == nullptr) {
      parent = notConstRootItem;
    }
    int row = item->row();
    if (row < 0 || row >= parent->rowCount()) {
      continue;
    }
#ifdef RUNNING_UNIT_TESTS
    // 只在测试环境中记录日志
    LOG_D("Append row:%d [%s]", row, qPrintable(item->text()));
#endif
    destItem->appendRow(parent->takeRow(row));
    succeedCnt++;
  }
  return succeedCnt;
}

int FavoritesTreeModel::removeParentIndexes(const QModelIndexList& parentIndexes) {
  if (parentIndexes.isEmpty()) {
    return 0;
  }

  QList<QStandardItem*> allItems = GetItemsNeedProcess(parentIndexes, nullptr);
  if (allItems.isEmpty()) {
    return -1;
  }

  QStandardItem* notConstRootItem = invisibleRootItem();

  int succeedCnt{0};
  for (QStandardItem* item : allItems) {
    QStandardItem* parent = item->parent();
    if (parent == nullptr) {
      parent = notConstRootItem;
    }
    int row = item->row();
    if (row < 0 || row >= parent->rowCount()) {
      continue;
    }
#ifdef RUNNING_UNIT_TESTS
    // 只在测试环境中记录日志
    LOG_D("Removing row: %d[%s]", row, qPrintable(item->text()));
#endif
    parent->removeRow(row);
    succeedCnt++;
  }

  LOG_D("Removed %d items successfully", succeedCnt);
  return succeedCnt;
}

bool FavoritesTreeModel::onRename(const QModelIndex& parentIndex, const QString& newName) {
  return setData(parentIndex, newName);
}

void FavoritesTreeModel::addInitialFavoritesGroup() {
  // initial configs
  QStandardItem* workGroup = addGroup(tr("Work"), nullptr);
  addPath("Documents", SystemPath::HOME_PATH() + "/Documents", workGroup);
  addPath("Project Configurations", SystemPath::HOME_PATH(), workGroup);

  QStandardItem* lifeGroup = addGroup(tr("Life"), nullptr);
  addPath("Pictures", SystemPath::HOME_PATH() + "/Pictures", lifeGroup);
  addPath("Videos", SystemPath::HOME_PATH() + "/Videos", lifeGroup);

  addPath("Code", SystemPath::HOME_PATH() + "/code", nullptr);
}

void FavoritesTreeModel::saveToSettings() {
  Configuration().setValue(GetDataKeyInQSetting(), toByteArray());
  LOG_D("All favorites saved");
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
    QStandardItem* addedItem = addPath(nameTextShown, fullPath, dstParent);
    if (addedItem != nullptr) {
      addedCount++;
    }
  }
  return addedCount;
}

int FavoritesTreeModel::handleInternalDrop(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& dstParent) {
  QByteArray encodedData = data->data(MIME_TYPE);
  QDataStream stream(&encodedData, QIODevice::ReadOnly);

  quint16 version;
  stream >> version;
  if (version != VERSION) {
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

  return moveParentIndexesTo(draggedIndexes, dstParent);
}

// todo: add a mutable sign to tell if need save changes
