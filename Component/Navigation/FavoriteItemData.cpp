// FavoriteItemData.cpp
#include "FavoriteItemData.h"
#include "MemoryKey.h"

constexpr int FavoriteItemData::SORT_COLUMN;
constexpr FavoriteItemData::Role FavoriteItemData::DEF_SORT_ROLE;

FavoriteItemData::FavoriteItemData(const QString& _name) : name{_name}, isGroup{true} {}

FavoriteItemData::FavoriteItemData(const QString& _name, const QString& path) : name{_name}, isGroup{false}, fullPath{path} {}

QDataStream& operator<<(QDataStream& out, const FavoriteItemData& item) {
  out << item.name;
  out << item.isGroup;
  out << item.fullPath;
  out << item.lastAccess;
  out << item.accessCount;
  out << item.children;
  return out;
}

QDataStream& operator>>(QDataStream& in, FavoriteItemData& item) {
  in >> item.name;
  in >> item.isGroup;
  in >> item.fullPath;
  in >> item.lastAccess;
  in >> item.accessCount;
  in >> item.children;
  return in;
}

FavoriteItemData::Role FavoriteItemData::GetInitialSortRole() {
  const int role{Configuration().value(FavoritesNavigationKey::SORT_BY_ROLE.name, FavoritesNavigationKey::SORT_BY_ROLE.v).toInt()};
  if (role < DEF_BEGIN_ROLE || role > INVALID_BUTT_ROLE) {
    return DEF_BEGIN_ROLE;
  }
  return static_cast<FavoriteItemData::Role>(role);
}

void FavoriteItemData::SaveInitialSortRole(Role sortRole) {
  Configuration().setValue(FavoritesNavigationKey::SORT_BY_ROLE.name, (int)sortRole);
}

bool FavoriteItemData::GetInitialSortOrderReverse() {
  return Configuration().value(FavoritesNavigationKey::SORT_ORDER_REVERSE.name, FavoritesNavigationKey::SORT_ORDER_REVERSE.v).toBool();
}

void FavoriteItemData::SaveSortOrderReverse(bool bReverse) {
  Configuration().setValue(FavoritesNavigationKey::SORT_ORDER_REVERSE.name, bReverse);
}
