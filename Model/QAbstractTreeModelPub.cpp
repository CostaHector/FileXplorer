#include "QAbstractTreeModelPub.h"
#include "FavoriteItemData.h"
#include "StyleItemData.h"

#include "Logger.h"
#include <QSet>

// newDatas必须是new 出来的
template<typename TDataType>
bool QAbstractTreeModelPub<TDataType>::setDatas(std::unique_ptr<TDataType> newDatas) {
  beginResetModel();
  m_pRoot.swap(newDatas);
  endResetModel();
  m_bIsDirty = true;
  return true;
}
template<typename TDataType>
bool QAbstractTreeModelPub<TDataType>::setDatas(const QByteArray& dataByteArray) {
  std::unique_ptr<TDataType> rootNodeFromByteArray{TDataType::NewTreeNodeRoot()};
  if (!dataByteArray.isEmpty()) {
    QDataStream readDs{dataByteArray};
    readDs.setVersion(QDataStream::Qt_5_15);
    quint16 version{0};
    readDs >> version;
    if (version != GetVersion()) {
      return false;
    }
    readDs >> *rootNodeFromByteArray;
    if (readDs.status() != QDataStream::Status::Ok) {
      return false;
    }
  }
  return setDatas(std::move(rootNodeFromByteArray));
}
template<typename TDataType>
QModelIndex QAbstractTreeModelPub<TDataType>::index(int row, int column, const QModelIndex& parent) const {
  if (!hasIndex(row, column, parent)) {
    return {};
  }
  TDataType* parentNode = itemFromIndex(parent);
  if (!parentNode) {
    return {};
  }
  if (TDataType* childNode = parentNode->child(row)) {
    return createIndex(row, column, childNode);
  }
  return {};
}
template<typename TDataType>
QModelIndex QAbstractTreeModelPub<TDataType>::siblingAtColumn(const QModelIndex &childIndex, int siblingColumn) const {
  if (!childIndex.isValid()) {
    return {};
  }
  if (siblingColumn == childIndex.column()) {
    return childIndex;
  }
  if (siblingColumn < 0 || siblingColumn >= columnCount(childIndex.parent())) {
    return {};
  }
  TDataType* currentNode = static_cast<TDataType*>(childIndex.internalPointer());
  if (!currentNode) {
    return {};
  }
  return createIndex(childIndex.row(), siblingColumn, currentNode);
}
template<typename TDataType>
QModelIndex QAbstractTreeModelPub<TDataType>::parent(const QModelIndex& child) const {
  if (!child.isValid()) {
    return {};
  }

  TDataType* childNode = static_cast<TDataType*>(child.internalPointer());
  if (!childNode) {
    return {};
  }

  TDataType* parentNode = childNode->parent();
  if (!parentNode || parentNode == invisibleRootItem()) {
    return {};
  }

  return createIndex(parentNode->row(), 0, parentNode);
}
template<typename TDataType>
int QAbstractTreeModelPub<TDataType>::rowCount(const QModelIndex& parent) const {
  if (TDataType* parentNode = itemFromIndex(parent)) {
    return parentNode->childsCount();
  }
  return 0;
}
template<typename TDataType>
QVariant QAbstractTreeModelPub<TDataType>::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role == Qt::DisplayRole && orientation == Qt::Orientation::Horizontal) {
    if (0 <= section && section < columnCount()) {
      static const auto& headers = TDataType::horizontalHeaderTitles();
      return headers[section];
    }
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}
template<typename TDataType>
TDataType* QAbstractTreeModelPub<TDataType>::invisibleRootItem() const {
  return m_pRoot ? m_pRoot.get() : nullptr;
}
template<typename TDataType>
TDataType* QAbstractTreeModelPub<TDataType>::itemFromIndex(const QModelIndex& index) const {
  if (!index.isValid()) {
    return invisibleRootItem();
  }
  return static_cast<TDataType*>(index.internalPointer());
}
template<typename TDataType>
QModelIndex QAbstractTreeModelPub<TDataType>::indexFromItem(const TDataType* item) const {
  if (!item || item == invisibleRootItem() || !item->parent()) {
    return {};
  }

  int row = item->row();
  if (row >= 0) {
    return createIndex(row, 0, const_cast<TDataType*>(item));
  }

  return {};
}
template<typename TDataType>
QByteArray QAbstractTreeModelPub<TDataType>::toByteArray() const {
  const TDataType* pRoot = invisibleRootItem();
  if (!pRoot) {
    return {};
  }
  QByteArray data;
  QDataStream stream(&data, QIODevice::WriteOnly);
  stream.setVersion(QDataStream::Qt_5_15);
  stream << GetVersion();
  stream << *pRoot;
  if (stream.status() != QDataStream::Status::Ok) {
    return {};
  }
  return data;
}
template<typename TDataType>
bool QAbstractTreeModelPub<TDataType>::isGroup(const QModelIndex& parentIndex) const {
  if (isRoot(parentIndex)) {
    return false;
  }
  TDataType* item = itemFromIndex(parentIndex);
  if (item == nullptr) {
    return false;  // failed
  }
  return item->isGroup();
}
template<typename TDataType>
QString QAbstractTreeModelPub<TDataType>::groupName(const QModelIndex& parentIndex) const {
  TDataType* item = itemFromIndex(parentIndex);
  if (item == nullptr) {
    return "";  // failed
  }
  return item->name();
}
template<typename TDataType>
bool QAbstractTreeModelPub<TDataType>::onRename(const QModelIndex& parentIndex, const QString& newName) {
  if (!setData(parentIndex, newName)) {
    return false;
  }
  setDirty();
  return true;
}
template<typename TDataType>
bool QAbstractTreeModelPub<TDataType>::isIndexValidAndDescendantOfValidAncestor(const QModelIndex& descendant, const QModelIndex& father) {
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
template<typename TDataType>
TDataType* QAbstractTreeModelPub<TDataType>::canDropIntoIndex(const QModelIndex& destParentIndex) const {
  if (!destParentIndex.isValid()) {
    return invisibleRootItem();
  }

  if (!isGroup(destParentIndex)) {
    LOG_D("Can only move to group or root");
    return nullptr;
  }
  return itemFromIndex(destParentIndex);
}

template<typename TDataType>
TDataType* QAbstractTreeModelPub<TDataType>::addGroup(const QString& grpName, const QModelIndex& parentIndex) {
  TDataType* parentItem = nullptr;
  if (parentIndex.isValid()) {
    parentItem = itemFromIndex(parentIndex);
    if (parentItem == nullptr) {
      LOG_W("Cannot get parent item from index");
      return nullptr;
    }
  }
  return addGroup(grpName, parentItem);
}
template<typename TDataType>
TDataType* QAbstractTreeModelPub<TDataType>::addGroup(const QString& grpName, TDataType* parentItem) {
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
  auto childNode = parentItem->appendRow(TDataType::create(typename TDataType::DataType{grpName}));
  endInsertRows();
  setDirty();
  return childNode;
}

// 检查节点是否有祖先在集合中
template<typename TDataType>
bool QAbstractTreeModelPub<TDataType>::hasAncestorInSet(const TDataType* node, const QSet<TDataType*>& ancestorSet) {
  // 若 ancestorSet 中包含了根, 将跳过所有移动/删除操作, 避免错删整个树
  for (TDataType* ancestor = node->parent(); ancestor != nullptr; ancestor = ancestor->parent()) {
    if (ancestorSet.contains(ancestor)) {
      return true;
    }
  }
  return false;
}

// 过滤不包含祖先的节点
template<typename TDataType>
QList<TDataType*> QAbstractTreeModelPub<TDataType>::filterWithoutAncestor(const QList<TDataType*>& items, const QSet<TDataType*>& ancestorSet) {
  QList<TDataType*> result;
  for (TDataType* item : items) {
    if (!hasAncestorInSet(item, ancestorSet)) {
      result.append(item);
    }
  }
  return result;
}

// 去除冗余, 先按照父索引排序, 在按照行号降序列
template<typename TDataType>
QList<TDataType*> QAbstractTreeModelPub<TDataType>::GetItemsNeedProcess(const QModelIndexList& parentIndexes, TDataType* destItem) const {
  const bool bMoveMode{destItem != nullptr};
  const TDataType* rootItem = invisibleRootItem();
  QList<TDataType*> allItems;
  {
    // 收集要移动的项，去重
    QSet<TDataType*> uniqueItems;

    QList<TDataType*> itemsToDelete;
    QSet<TDataType*> groupsToDelete;
    for (const QModelIndex& idx : parentIndexes) {
      if (!idx.isValid()) {
        continue;
      }
      TDataType* item = itemFromIndex(idx);
      if (item == nullptr || item == rootItem || uniqueItems.contains(item)) {
        continue;
      }
      if (bMoveMode) {
        if (item == destItem) {
          LOG_W("Cannot move elements to itself");
          return {};
        }
        if (destItem->isDescendantOf(item)) {
          LOG_W("Cannot move ancestor[item] to it's children[destItem]");
          return {};
        }
      }
      uniqueItems.insert(item);

      if (item->isGroup()) {
        groupsToDelete.insert(item);
      } else {
        itemsToDelete.append(item);
      }
    }

    allItems.reserve(itemsToDelete.size() + groupsToDelete.size());
    // 非分组在要移动/删除的分组中时, 不需要单独移动/删除它, 避免重复移动/删除
    allItems += filterWithoutAncestor(itemsToDelete, groupsToDelete);
    // 分组在要移动/删除的分组中时, 不需要单独移动/删除它, 避免重复移动/删除
    allItems += filterWithoutAncestor(QList<TDataType*>{groupsToDelete.begin(), groupsToDelete.end()}, groupsToDelete);
  }
  if (allItems.isEmpty()) {
    return {};
  }

  // 按父项和行号排序, 让相同父项的项在一起, 按行号从大到小排序
  std::sort(allItems.begin(), allItems.end(), [rootItem](TDataType* a, TDataType* b) {
    const TDataType* parentA = a->parent();
    if (parentA == nullptr) {
      parentA = rootItem;
    }
    const TDataType* parentB = b->parent();
    if (parentB == nullptr) {
      parentB = rootItem;
    }
    return parentA != parentB ? parentA < parentB : a->row() > b->row();
  });

  return allItems;
}
template<typename TDataType>
int QAbstractTreeModelPub<TDataType>::moveParentIndexesTo(const QModelIndexList& parentIndexes, const QModelIndex& destInd) {
  if (parentIndexes.isEmpty()) {
    return 0;
  }
  TDataType* destNode = canDropIntoIndex(destInd);
  if (destNode == nullptr) {
    return -1;
  }

  QList<TDataType*> allItems = GetItemsNeedProcess(parentIndexes, destNode);
  if (allItems.isEmpty()) {
    return -1;
  }

  // 开始移动
  int succeedCnt{0};
  TDataType* notConstRootItem = invisibleRootItem();
  for (TDataType* item : allItems) {
    TDataType* srcNode = item->parent();
    if (srcNode == nullptr) {
      srcNode = notConstRootItem;
    }
    const int row{item->row()};
    if (row < 0 || row >= srcNode->rowCount()) {
      continue;
    }

    const QModelIndex srcInd{indexFromItem(srcNode)};
    {
      const int destRowCount{destNode->rowCount()};
      const int destRow{destRowCount};
      const bool bSameParent{srcNode == destNode};
      if (bSameParent && (row <= destRow && destRow <= row + 1)) {
        // 1. 同父, 向后移动时, 若 destRow in [row, row+1] 直接跳过, 顺序已符合预期, 无需调整
        succeedCnt++;
        continue;
      }
      // 2. 差异: 此处允许子结点往父节点上移动, 但不允许父节点往子结点上移动(避免环),
      // 原生beginMoveRows不允许任何有父子关系的移动, 否则false;
      // 原生beginMoveRows 在同父间移动时不允许destRow在[rowFront, rowBack+1]区间内(无需多余移动), 否则false
      if (beginMoveRows(srcInd, row, row, destInd, destRow)) {
        TDataType* pNode = srcNode->takeRow(row);
        destNode->appendRow(pNode);
        endMoveRows();
      } else {
        LOG_W("beginMoveRows return false[sameParent:%d, row:%d, destRow:%d, destRowCnt:%d]", bSameParent, row, destRow, destRowCount);
        /* or do remove and append seperately
         {
          beginRemoveRows(srcInd, row, row);
          TDataType* pNode = srcNode->takeRow(row);
          endRemoveRows();
          beginInsertRows(destInd, destRowCount, destRowCount);
          destNode->appendRow(pNode);
          endInsertRows();
         }
         */
        return -2;
      }
    }
    succeedCnt++;
  }
  setDirty();
  return succeedCnt;
}
template<typename TDataType>
int QAbstractTreeModelPub<TDataType>::removeParentIndexes(const QModelIndexList& parentIndexes) {
  if (parentIndexes.isEmpty()) {
    return 0;
  }

  QList<TDataType*> allItems = GetItemsNeedProcess(parentIndexes, nullptr);
  if (allItems.isEmpty()) {
    return -1;
  }

  TDataType* notConstRootItem = invisibleRootItem();

  int succeedCnt{0};
  for (TDataType* item : allItems) {
    TDataType* parent = item->parent();
    if (parent == nullptr) {
      parent = notConstRootItem;
    }
    int row = item->row();
    if (row < 0 || row >= parent->rowCount()) {
      continue;
    }
    QModelIndex parentIndex = indexFromItem(parent);
    beginRemoveRows(parentIndex, row, row);
    parent->removeRow(row);
    endRemoveRows();
    succeedCnt++;
  }
  setDirty();
  LOG_D("Removed %d items successfully", succeedCnt);
  return succeedCnt;
}

template class QAbstractTreeModelPub<FavTreeNode>;
template class QAbstractTreeModelPub<StyleTreeNode>;
