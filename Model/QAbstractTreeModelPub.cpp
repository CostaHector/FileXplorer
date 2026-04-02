#include "QAbstractTreeModelPub.h"
#include <QIcon>

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
