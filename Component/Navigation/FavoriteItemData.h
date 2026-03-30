#ifndef FAVORITEITEMDATA_H
#define FAVORITEITEMDATA_H

#include <QString>
#include <QVector>
#include <QDataStream>

struct FavoriteItemData {
  friend QDataStream& operator<<(QDataStream& out, const FavoriteItemData& item);
  friend QDataStream& operator>>(QDataStream& in, FavoriteItemData& item);

  FavoriteItemData() = default;
  explicit FavoriteItemData(const QString& _name);
  FavoriteItemData(const QString& _name, const QString& path);

  QString name;                        // 显示名称
  bool isGroup{true};                  // 是否为分组
  QString fullPath;                    // 完整路径（如果不是分组）
  int lastAccess{0};                   // 最后访问时间
  int accessCount{0};                  // 访问次数
  QVector<FavoriteItemData> children;  // 子项

  bool operator==(const FavoriteItemData& other) const {  //
    return name == other.name && isGroup == other.isGroup && fullPath == other.fullPath && children == other.children;
  }

  bool isValid() const { return !name.isEmpty() && (isGroup || !fullPath.isEmpty()); }
};

QDataStream& operator<<(QDataStream& out, const FavoriteItemData& item);
QDataStream& operator>>(QDataStream& in, FavoriteItemData& item);

#endif  // FAVORITEITEMDATA_H
