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

  enum Role {
    DEF_BEGIN_ROLE = Qt::DisplayRole,
    DEF_NAME_TEXT_ROLE = DEF_BEGIN_ROLE,
    IS_GROUP_ROLE = Qt::UserRole + 1,
    FULL_PATH_ROLE,
    LAST_ACCESS_ROLE,
    ACCESS_COUNT_ROLE,
    INVALID_BUTT_ROLE,
  };

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

  static Role GetInitialSortRole();
  static void SaveInitialSortRole(Role sortRole);
  static bool GetInitialSortOrderReverse();
  static void SaveSortOrderReverse(bool bReverse);

  static constexpr int SORT_COLUMN = 0;
  static constexpr Role DEF_SORT_ROLE = DEF_NAME_TEXT_ROLE;
};

QDataStream& operator<<(QDataStream& out, const FavoriteItemData& item);
QDataStream& operator>>(QDataStream& in, FavoriteItemData& item);

#endif  // FAVORITEITEMDATA_H
