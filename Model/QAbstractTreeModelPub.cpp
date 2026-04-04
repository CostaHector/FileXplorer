#include "QAbstractTreeModelPub.h"
#include "Logger.h"
#include <QSet>

bool MyTreeNode::operator==(const MyTreeNode& rhs) const {
  if (val != rhs.val) {
    return false;
  }
  if (childsCount() != rhs.childsCount()) {
    return false;
  }
  for (int i = 0; i < childsCount(); ++i) {
    const MyTreeNode* leftChild = child(i);
    const MyTreeNode* rightChild = rhs.child(i);
    if (leftChild == nullptr && rightChild == nullptr) {
      continue;
    }
    if (leftChild != nullptr && rightChild != nullptr) {
      if (*leftChild != *rightChild) {
        return false;
      }
      continue;
    }
    return false;
  }
  return true;
}

QDataStream& operator<<(QDataStream& out, const MyTreeNode& item) {
  out << item.val;
  int n = item.childsCount();
  out << n;
  for (MyTreeNode* pChild : item.childs) {
    out << *pChild;
  }
  return out;
}

QDataStream& operator>>(QDataStream& in, MyTreeNode& item) {
  item.releaseAndClearChilds();

  in >> item.val;

  int n{0};
  in >> n;

  item.childs.reserve(n);
  for (int i = 0; i < n; ++i) {
    MyTreeNode* node = new MyTreeNode;
    in >> *node;
    node->pParent = &item;
    item.childs.push_back(node);
  }
  return in;
}

// newDatas必须是new 出来的
bool QAbstractTreeModelPub::setDatas(std::unique_ptr<MyTreeNode> newDatas) {
  beginResetModel();
  m_pRoot.swap(newDatas);
  endResetModel();
  m_bIsDirty = true;
  return true;
}

bool QAbstractTreeModelPub::setDatas(const QByteArray& dataByteArray) {
  std::unique_ptr<MyTreeNode> rootNodeFromByteArray{new MyTreeNode};
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

QModelIndex QAbstractTreeModelPub::index(int row, int column, const QModelIndex& parent) const {
  if (!hasIndex(row, column, parent)) {
    return {};
  }
  MyTreeNode* parentNode = itemFromIndex(parent);
  if (!parentNode) {
    return {};
  }
  if (MyTreeNode* childNode = parentNode->child(row)) {
    return createIndex(row, column, childNode);
  }
  return {};
}

QModelIndex QAbstractTreeModelPub::parent(const QModelIndex& child) const {
  if (!child.isValid()) {
    return {};
  }

  MyTreeNode* childNode = static_cast<MyTreeNode*>(child.internalPointer());
  if (!childNode) {
    return {};
  }

  MyTreeNode* parentNode = childNode->parent();
  if (!parentNode || parentNode == invisibleRootItem()) {
    return {};
  }

  return createIndex(parentNode->row(), 0, parentNode);
}

int QAbstractTreeModelPub::rowCount(const QModelIndex& parent) const {
  if (MyTreeNode* parentNode = itemFromIndex(parent)) {
    return parentNode->childsCount();
  }
  return 0;
}

QVariant QAbstractTreeModelPub::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role == Qt::DisplayRole && orientation == Qt::Orientation::Horizontal) {
    if (0 <= section && section < FavoriteItemData::COLUMN_COUNT) {
      return FavoriteItemData::HOR_HEADER_TITLES[section];
    }
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}

MyTreeNode* QAbstractTreeModelPub::invisibleRootItem() const {
  return m_pRoot ? m_pRoot.get() : nullptr;
}

MyTreeNode* QAbstractTreeModelPub::itemFromIndex(const QModelIndex& index) const {
  if (!index.isValid()) {
    return invisibleRootItem();
  }
  return static_cast<MyTreeNode*>(index.internalPointer());
}

QModelIndex QAbstractTreeModelPub::indexFromItem(const MyTreeNode* item) const {
  if (!item || item == invisibleRootItem() || !item->parent()) {
    return {};
  }

  int row = item->row();
  if (row >= 0) {
    return createIndex(row, 0, const_cast<MyTreeNode*>(item));
  }

  return {};
}

QByteArray QAbstractTreeModelPub::toByteArray() const {
  const MyTreeNode* pRoot = invisibleRootItem();
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

bool QAbstractTreeModelPub::isGroup(const QModelIndex& parentIndex) const {
  if (isRoot(parentIndex)) {
    return false;
  }
  MyTreeNode* item = itemFromIndex(parentIndex);
  if (item == nullptr) {
    return false;  // failed
  }
  return item->isGroup();
}

QString QAbstractTreeModelPub::groupName(const QModelIndex& parentIndex) const {
  MyTreeNode* item = itemFromIndex(parentIndex);
  if (item == nullptr) {
    return "";  // failed
  }
  return item->name();
}

bool QAbstractTreeModelPub::onRename(const QModelIndex& parentIndex, const QString& newName) {
  if (!setData(parentIndex, newName)) {
    return false;
  }
  setDirty();
  return true;
}

bool QAbstractTreeModelPub::isIndexValidAndDescendantOfValidAncestor(const QModelIndex& descendant, const QModelIndex& father) {
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

MyTreeNode* QAbstractTreeModelPub::canDropIntoIndex(const QModelIndex& destParentIndex) const {
  if (!destParentIndex.isValid()) {
    return invisibleRootItem();
  }

  if (!isGroup(destParentIndex)) {
    LOG_D("Can only move to group or root");
    return nullptr;
  }
  return itemFromIndex(destParentIndex);
}

// 检查节点是否有祖先在集合中
bool hasAncestorInSet(const MyTreeNode* node, const QSet<MyTreeNode*>& ancestorSet) {
  // 若 ancestorSet 中包含了根, 将跳过所有移动/删除操作, 避免错删整个树
  for (MyTreeNode* ancestor = node->parent(); ancestor != nullptr; ancestor = ancestor->parent()) {
    if (ancestorSet.contains(ancestor)) {
      return true;
    }
  }
  return false;
}

// 过滤不包含祖先的节点
QList<MyTreeNode*> filterWithoutAncestor(const QList<MyTreeNode*>& items, const QSet<MyTreeNode*>& ancestorSet) {
  QList<MyTreeNode*> result;
  for (MyTreeNode* item : items) {
    if (!hasAncestorInSet(item, ancestorSet)) {
      result.append(item);
    }
  }
  return result;
}

// 去除冗余, 先按照父索引排序, 在按照行号降序列
QList<MyTreeNode*> QAbstractTreeModelPub::GetItemsNeedProcess(const QModelIndexList& parentIndexes, MyTreeNode* destItem) const {
  const bool bMoveMode{destItem != nullptr};
  const MyTreeNode* rootItem = invisibleRootItem();
  QList<MyTreeNode*> allItems;
  {
    // 收集要移动的项，去重
    QSet<MyTreeNode*> uniqueItems;

    QList<MyTreeNode*> itemsToDelete;
    QSet<MyTreeNode*> groupsToDelete;
    for (const QModelIndex& idx : parentIndexes) {
      if (!idx.isValid()) {
        continue;
      }
      MyTreeNode* item = itemFromIndex(idx);
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
    allItems += filterWithoutAncestor(QList<MyTreeNode*>{groupsToDelete.begin(), groupsToDelete.end()}, groupsToDelete);
  }
  if (allItems.isEmpty()) {
    return {};
  }

  // 按父项和行号排序, 让相同父项的项在一起, 按行号从大到小排序
  std::sort(allItems.begin(), allItems.end(), [rootItem](MyTreeNode* a, MyTreeNode* b) {
    const MyTreeNode* parentA = a->parent();
    if (parentA == nullptr) {
      parentA = rootItem;
    }
    const MyTreeNode* parentB = b->parent();
    if (parentB == nullptr) {
      parentB = rootItem;
    }
    return parentA != parentB ? parentA < parentB : a->row() > b->row();
  });

  return allItems;
}

int QAbstractTreeModelPub::moveParentIndexesTo(const QModelIndexList& parentIndexes, const QModelIndex& destInd) {
  if (parentIndexes.isEmpty()) {
    return 0;
  }
  MyTreeNode* destNode = canDropIntoIndex(destInd);
  if (destNode == nullptr) {
    return -1;
  }

  QList<MyTreeNode*> allItems = GetItemsNeedProcess(parentIndexes, destNode);
  if (allItems.isEmpty()) {
    return -1;
  }

  // 开始移动
  int succeedCnt{0};
  MyTreeNode* notConstRootItem = invisibleRootItem();
  for (MyTreeNode* item : allItems) {
    MyTreeNode* srcNode = item->parent();
    if (srcNode == nullptr) {
      srcNode = notConstRootItem;
    }
    const int row{item->row()};
    if (row < 0 || row >= srcNode->rowCount()) {
      continue;
    }

    const QModelIndex srcInd{indexFromItem(srcNode)};
    {
      int destRow = destNode->rowCount();
      if (srcNode == destNode) {
        // 1. 同父, 向后移动时, 要--目的行, 若源和目的相等, 则跳过;
        if (row < destRow) {
          --destRow;
        }
        if (destRow == row) {
          succeedCnt++;  // no need move
          continue;
        }
      }
      // 2. 差异: 此处允许子结点往父节点上移动, 但不允许父节点往子结点上移动(避免环),
      // 原生beginMoveRows不允许任何有父子关系的移动, 否则false;
      // 原生beginMoveRows不允许destRow, 在[row1, row2]区间内, 否则false
      const bool begSucceed{beginMoveRows(srcInd, row, row, destInd, destRow)};
      MyTreeNode* pNode = srcNode->takeRow(row);
      destNode->appendRow(pNode);
      if (begSucceed) {
        endMoveRows();
      }
    }

    /* or do remove and append seperately
     {
      beginRemoveRows(srcInd, row, row);
      MyTreeNode* pNode = srcNode->takeRow(row);
      endRemoveRows();
      int destRow = destNode->rowCount();
      beginInsertRows(destInd, destRow, destRow);
      destNode->appendRow(pNode);
      endInsertRows();
     }
     */

    succeedCnt++;
  }
  setDirty();
  return succeedCnt;
}

int QAbstractTreeModelPub::removeParentIndexes(const QModelIndexList& parentIndexes) {
  if (parentIndexes.isEmpty()) {
    return 0;
  }

  QList<MyTreeNode*> allItems = GetItemsNeedProcess(parentIndexes, nullptr);
  if (allItems.isEmpty()) {
    return -1;
  }

  MyTreeNode* notConstRootItem = invisibleRootItem();

  int succeedCnt{0};
  for (MyTreeNode* item : allItems) {
    MyTreeNode* parent = item->parent();
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
