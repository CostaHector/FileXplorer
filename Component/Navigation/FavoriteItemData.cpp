// FavoriteItemData.cpp
#include "FavoriteItemData.h"
#include "FavoritesNavigationKey.h"
#include "Configuration.h"

constexpr int FavoriteItemData::COLUMN_COUNT;
constexpr const char* FavoriteItemData::HOR_HEADER_TITLES[];
constexpr int FavoriteItemData::SORT_COLUMN;
constexpr FavoriteItemData::Role FavoriteItemData::DEF_SORT_ROLE;

FavoriteItemData::FavoriteItemData(const QString& _name)
  : name{_name}
  , isGroup{true} {}
FavoriteItemData::FavoriteItemData(const QString& _name, const QString& path)
  : name{_name}
  , isGroup{false}
  , fullPath{path} {}

QDataStream& operator<<(QDataStream& out, const FavoriteItemData& item) {
  out << item.name;
  out << item.fullPath;
  out << item.isGroup;
  out << item.lastAccess;
  out << item.accessCount;
  return out;
}

QDataStream& operator>>(QDataStream& in, FavoriteItemData& item) {
  in >> item.name;
  in >> item.fullPath;
  in >> item.isGroup;
  in >> item.lastAccess;
  in >> item.accessCount;
  return in;
}

FavoriteItemData::Role FavoriteItemData::GetInitialSortRole() {
  const int role{getConfig(FavoritesNavigationKey::SORT_BY_ROLE).toInt()};
  if (role < DEF_BEGIN_ROLE || role > INVALID_BUTT_ROLE) {
    return FavoriteItemData::DEF_SORT_ROLE;
  }
  return static_cast<FavoriteItemData::Role>(role);
}

void FavoriteItemData::SaveInitialSortRole(Role sortRole) {
  setConfig(FavoritesNavigationKey::SORT_BY_ROLE, (int) sortRole);
}

bool FavoriteItemData::GetInitialSortOrderReverse() {
  return getConfig(FavoritesNavigationKey::SORT_ORDER_REVERSE).toBool();
}

void FavoriteItemData::SaveSortOrderReverse(bool bReverse) {
  setConfig(FavoritesNavigationKey::SORT_ORDER_REVERSE, bReverse);
}

bool FavoriteItemData::match(const QString& subStr, const Qt::CaseSensitivity caseMatter) const {
  if (subStr.isEmpty()) {
    return true;
  }
  if (isGroup) {
    return name.contains(subStr, caseMatter);
  }
  return name.contains(subStr, caseMatter) || fullPath.contains(subStr, caseMatter);
}

QDataStream& operator<<(QDataStream& out, const FavTreeNode& item) {
  out << item.val;
  int n = item.childsCount();
  out << n;
  for (FavTreeNode* pChild : item.childs) {
    out << *pChild;
  }
  return out;
}

QDataStream& operator>>(QDataStream& in, FavTreeNode& item) {
  item.releaseAndClearChilds();

  in >> item.val;

  int n{0};
  in >> n;

  item.childs.reserve(n);
  for (int i = 0; i < n; ++i) {
    FavTreeNode* node = new FavTreeNode;
    in >> *node;
    node->pParent = &item;
    item.childs.push_back(node);
  }
  return in;
}
