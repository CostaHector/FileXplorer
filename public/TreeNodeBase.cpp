#include "TreeNodeBase.h"
#include <QHash>

template<typename Derived, typename TDataType>
bool TreeNodeBase<Derived, TDataType>::isAncestorOf(const Derived* descendant) const {
  if (descendant == nullptr) {
    return false;
  }
  return descendant->isDescendantOf(derived());
}

template<typename Derived, typename TDataType>
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

template<typename Derived, typename TDataType>
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

template<typename Derived, typename TDataType>
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

template<typename Derived, typename TDataType>
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

template<typename Derived, typename TDataType>
bool TreeNodeBase<Derived, TDataType>::isCurrentNodeMatch(const QString& text, QHash<const void*, bool>& passCache) const {
  const auto it = passCache.find(static_cast<const void*>(this));
  if (it != passCache.end()) {
    return it.value();
  }
  return passCache[static_cast<const void*>(this)] = value().match(text);
}

template<typename Derived, typename TDataType>
bool TreeNodeBase<Derived, TDataType>::isChildNodeMatch(const QString& text, QHash<const void*, bool>& passCache) const {
  for (const Derived* pDerived: childs) {
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

template<typename Derived, typename TDataType>
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
