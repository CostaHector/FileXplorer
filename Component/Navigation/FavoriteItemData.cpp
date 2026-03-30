// FavoriteItemData.cpp
#include "FavoriteItemData.h"

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
