#include "TreeNodeBase.h"
#include "Logger.h"
#include <QHash>

template <typename Derived, typename TDataType>
const Derived* TreeNodeBase<Derived, TDataType>::FindNode(const QString& fullPath) const {
  const Derived* pCurDerived = derived();
  if (fullPath.isEmpty()) {
    return pCurDerived;
  }
  // 分割路径
  const QStringList& pathParts = fullPath.split('/', Qt::KeepEmptyParts);

  if (pCurDerived->name() != pathParts[0]) {
    return nullptr;  // rootName not match
  }

  // find parts[1], ..., parts[n-1] in childs
  for (int i = 1; i < pathParts.size(); ++i) {
    const QString& pathPart = pathParts[i];

    bool bFindInChilds{false};
    for (const Derived* child : pCurDerived->childs) {
      if (child->name() == pathPart) {
        pCurDerived = child;
        bFindInChilds = true;
        break;
      }
    }

    if (!bFindInChilds) {
      return nullptr;
    }
  }
  return pCurDerived;
}

/* pairList with element {prepath, data} likes
QList{
{"p1/p10", data0},
{"p1", data1},
{"p2", data2},
};
note that: prepath must not endswith slash!

rootNodeName likes
QString{"root"}
 */
template <typename Derived, typename TDataType>
std::unique_ptr<Derived> TreeNodeBase<Derived, TDataType>::fromPairList(const QList<std::pair<QString, TDataType>>& pairList, const QString& rootNodeName) {
  // 创建一个虚拟根节点
  std::unique_ptr<Derived> root = NewTreeNodeRoot(rootNodeName);
  if (pairList.isEmpty()) {
    return root;
  }

  QHash<QString, Derived*> path2Node;
  path2Node.insert("", root.get());

  static constexpr QChar PATH_SPLITTER = '/';
  for (auto& pr : pairList) {
    QString path = pr.first;
    if (path.isEmpty()) {
      LOG_W("cannot contains empty path here");  //
      continue;
    }
    path += PATH_SPLITTER;
    Derived* childNodeBelongTo = root.get();

    const QStringList& pathParts = path.split(PATH_SPLITTER, Qt::SplitBehaviorFlags::KeepEmptyParts);
    QString prePathJoin;
    for (const QString& pathPart : pathParts) {
      if (!prePathJoin.isEmpty()) {
        prePathJoin += PATH_SPLITTER;
      }
      prePathJoin += pathPart;
      auto nodeIt = path2Node.find(prePathJoin);
      if (nodeIt != path2Node.end()) {
        childNodeBelongTo = nodeIt.value();
        continue;
      }

      Derived* childNode{nullptr};
      if (prePathJoin.size() < path.size()) {
        // 到达分支group
        childNode = new Derived(TDataType{pathPart});
        path2Node.insert(prePathJoin, childNode);
      } else {
        // 到达最底部叶子, 不能把叶子加入缓存
        childNode = new Derived(pr.second);
      }
      childNodeBelongTo->appendRow(childNode);
      childNodeBelongTo = childNode;
    }
  }

  return root;
}

template <typename Derived, typename TDataType>
bool TreeNodeBase<Derived, TDataType>::isAncestorOf(const Derived* descendant) const {
  if (descendant == nullptr) {
    return false;
  }
  return descendant->isDescendantOf(derived());
}

template <typename Derived, typename TDataType>
bool TreeNodeBase<Derived, TDataType>::isDescendantOf(const Derived* ancestor) const {
  if (ancestor == nullptr) {
    return false;
  }

  for (const Derived* parentNode = parent(); parentNode != nullptr; parentNode = parentNode->parent()) {
    if (parentNode == ancestor) {
      return true;
    }
  }

  return false;
}

template <typename Derived, typename TDataType>
bool TreeNodeBase<Derived, TDataType>::operator==(const Derived& rhs) const {
  if (this == &rhs) {
    return true;
  }
  if (val != rhs.val) {
    return false;
  }
  if (childsCount() != rhs.childsCount()) {
    return false;
  }
  for (int i = 0; i < childsCount(); ++i) {
    const Derived* leftChild = child(i);
    const Derived* rightChild = rhs.child(i);
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

template <typename Derived, typename TDataType>
QString TreeNodeBase<Derived, TDataType>::GetConfigKey() const {
  QStringList keyPrefixListRev;
  keyPrefixListRev.reserve(5);
  const Derived* node = derived();
  while (node) {
    keyPrefixListRev.push_back(node->name());
    node = node->parent();
  }
  std::reverse(keyPrefixListRev.begin(), keyPrefixListRev.end());
  return keyPrefixListRev.join('/');
}

template <typename Derived, typename TDataType>
bool TreeNodeBase<Derived, TDataType>::filterAccept(const QString& text, QHash<const void*, bool>& passCache) const {
  if (text.isEmpty()) {
    return true;
  }
  if (isCurrentNodeMatch(text, passCache)) {
    return true;
  }
  if (isChildNodeMatch(text, passCache)) {
    return true;
  }
  if (isParentNodeMatch(text, passCache)) {
    return true;
  }
  return false;
}

template <typename Derived, typename TDataType>
bool TreeNodeBase<Derived, TDataType>::isCurrentNodeMatch(const QString& text, QHash<const void*, bool>& passCache) const {
  const auto it = passCache.find(static_cast<const void*>(this));
  if (it != passCache.end()) {
    return it.value();
  }
  return passCache[static_cast<const void*>(this)] = value().match(text);
}

template <typename Derived, typename TDataType>
bool TreeNodeBase<Derived, TDataType>::isChildNodeMatch(const QString& text, QHash<const void*, bool>& passCache) const {
  for (const Derived* pDerived : childs) {
    if (pDerived == nullptr) {
      continue;
    }
    if (pDerived->isCurrentNodeMatch(text, passCache)) {
      return true;
    }
    if (pDerived->isChildNodeMatch(text, passCache)) {
      return true;
    }
  }
  return false;
}

template <typename Derived, typename TDataType>
bool TreeNodeBase<Derived, TDataType>::isParentNodeMatch(const QString& text, QHash<const void*, bool>& passCache) const {
  const Derived* pDerived = derived();
  while (pDerived = pDerived->parent()) {
    if (pDerived->isCurrentNodeMatch(text, passCache)) {
      return true;
    }
  }
  return false;
}

#include "FavoriteItemData.h"
template class TreeNodeBase<FavTreeNode, FavoriteItemData>;

#include "StyleItemData.h"
template class TreeNodeBase<StyleTreeNode, StyleItemData>;
