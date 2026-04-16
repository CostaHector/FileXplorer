#ifndef FAVORITEITEMDATA_H
#define FAVORITEITEMDATA_H

#include "TreeNodeBase.h"

#include <QString>
#include <QDataStream>
#include <memory>

struct FavoriteItemData final {
  friend QDataStream& operator<<(QDataStream& out, const FavoriteItemData& item);
  friend QDataStream& operator>>(QDataStream& in, FavoriteItemData& item);

  FavoriteItemData() = default;
  explicit FavoriteItemData(const QString& _name);
  FavoriteItemData(const QString& _name, const QString& path);

  enum Role { // -DEF_BEGIN_ROLE represent column
    DEF_BEGIN_ROLE = Qt::UserRole + 1,
    DEF_NAME_TEXT_ROLE = DEF_BEGIN_ROLE,
    FULL_PATH_ROLE,
    IS_GROUP_ROLE,
    LAST_ACCESS_ROLE,
    ACCESS_COUNT_ROLE,
    INVALID_BUTT_ROLE,
  };

  bool operator==(const FavoriteItemData& other) const { //
    return name == other.name && isGroup == other.isGroup && fullPath == other.fullPath;
  }

  bool operator!=(const FavoriteItemData& other) const { return !(*this == other); }

  bool isValid() const { return !name.isEmpty() && (isGroup || !fullPath.isEmpty()); }
  bool match(const QString& subStr) const;

  QString name;       // 显示名称
  QString fullPath;   // 完整路径（如果不是分组）
  bool isGroup{true}; // 是否为分组
  int lastAccess{0};  // 最后访问时间
  int accessCount{0}; // 访问次数

  static Role GetInitialSortRole();
  static void SaveInitialSortRole(Role sortRole);
  static bool GetInitialSortOrderReverse();
  static void SaveSortOrderReverse(bool bReverse);

  static constexpr int COLUMN_COUNT = INVALID_BUTT_ROLE - DEF_BEGIN_ROLE;
  static constexpr const char* HOR_HEADER_TITLES[COLUMN_COUNT]{"Name", "Full Path", "Is Group", "Last Access", "Access Time"};
  static constexpr int SORT_COLUMN = 0;
  static constexpr Role DEF_SORT_ROLE = DEF_NAME_TEXT_ROLE;
};

QDataStream& operator<<(QDataStream& out, const FavoriteItemData& item);
QDataStream& operator>>(QDataStream& in, FavoriteItemData& item);

struct FavTreeNode;
extern template class TreeNodeBase<FavTreeNode, FavoriteItemData>;

struct FavTreeNode final : public TreeNodeBase<FavTreeNode, FavoriteItemData> {
  friend QDataStream& operator<<(QDataStream& out, const FavTreeNode& item);
  friend QDataStream& operator>>(QDataStream& in, FavTreeNode& item);
  using TreeNodeBase<FavTreeNode, FavoriteItemData>::TreeNodeBase;
};

QDataStream& operator<<(QDataStream& out, const FavTreeNode& item);
QDataStream& operator>>(QDataStream& in, FavTreeNode& item);

#endif // FAVORITEITEMDATA_H
