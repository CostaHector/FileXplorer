#ifndef FAVORITEITEMDATA_H
#define FAVORITEITEMDATA_H

#include <QString>
#include <QVector>
#include <QDataStream>
#include <memory>

struct FavoriteItemData final {
  friend QDataStream& operator<<(QDataStream& out, const FavoriteItemData& item);
  friend QDataStream& operator>>(QDataStream& in, FavoriteItemData& item);

  FavoriteItemData() = default;
  explicit FavoriteItemData(const QString& _name);
  FavoriteItemData(const QString& _name, const QString& path);

  enum Role {  // -DEF_BEGIN_ROLE represent column
    DEF_BEGIN_ROLE = Qt::UserRole + 1,
    DEF_NAME_TEXT_ROLE = DEF_BEGIN_ROLE,
    FULL_PATH_ROLE,
    IS_GROUP_ROLE,
    LAST_ACCESS_ROLE,
    ACCESS_COUNT_ROLE,
    INVALID_BUTT_ROLE,
  };

  bool operator==(const FavoriteItemData& other) const {  //
    return name == other.name && isGroup == other.isGroup && fullPath == other.fullPath;
  }

  bool operator!=(const FavoriteItemData& other) const { return !(*this == other); }

  bool isValid() const { return !name.isEmpty() && (isGroup || !fullPath.isEmpty()); }

  QString name;        // 显示名称
  QString fullPath;    // 完整路径（如果不是分组）
  bool isGroup{true};  // 是否为分组
  int lastAccess{0};   // 最后访问时间
  int accessCount{0};  // 访问次数

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

using TDataType = FavoriteItemData;
struct MyTreeNode final {
  friend QDataStream& operator<<(QDataStream& out, const MyTreeNode& item);
  friend QDataStream& operator>>(QDataStream& in, MyTreeNode& item);
  static std::unique_ptr<MyTreeNode> NewTreeNodeRoot() { return std::unique_ptr<MyTreeNode>{new MyTreeNode}; }

  MyTreeNode() = default;
  MyTreeNode(const TDataType& _val, const QList<MyTreeNode*>& _childs = {}, MyTreeNode* _parent = nullptr)
      : val{_val}, childs{_childs}, pParent{_parent} {}
  ~MyTreeNode() { releaseAndClearChilds(); }
  MyTreeNode* parent() const { return pParent; }
  bool isGroup() const { return val.isGroup; }
  QString name() const { return val.name; }
  bool setName(const QString& newName) {
    if (val.name == newName) {
      return false;  // unchanged
    }
    val.name = newName;
    return true;
  }
  bool isAncestorOf(const MyTreeNode* descendant) const;
  bool isDescendantOf(const MyTreeNode* ancestor) const;

  bool operator==(const MyTreeNode& rhs) const;
  bool operator!=(const MyTreeNode& rhs) const { return !(*this == rhs); }

  void releaseAndClearChilds() {
    qDeleteAll(childs);
    childs.clear();
  }

  TDataType val;
  QList<MyTreeNode*> childs;
  MyTreeNode* pParent{nullptr};

  int childsCount() const { return childs.size(); }
  int rowCount() const { return childsCount(); }

  MyTreeNode* child(int row, int column = 0) const {
    if (0 <= row && row < childsCount()) {
      return childs[row];
    }
    return nullptr;
  }

  MyTreeNode* appendRow(MyTreeNode* child) {
    if (child) {
      childs.append(child);
      child->pParent = this;
    }
    return child;
  }
  MyTreeNode* takeRow(int row) {
    if (0 <= row && row < childsCount()) {
      return childs.takeAt(row);
    }
    return nullptr;
  }

  void removeRow(int row) {
    if (0 <= row && row < childsCount()) {
      if (childs[row] != nullptr) {
        delete childs[row];
      }
      childs.erase(childs.begin() + row);
    }
  }

  int row() const {
    if (pParent) {
      return pParent->childs.indexOf(const_cast<MyTreeNode*>(this));
    }
    return 0;
  }
};

QDataStream& operator<<(QDataStream& out, const MyTreeNode& item);
QDataStream& operator>>(QDataStream& in, MyTreeNode& item);

#endif  // FAVORITEITEMDATA_H
