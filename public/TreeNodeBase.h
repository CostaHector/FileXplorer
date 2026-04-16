#ifndef TREENODEBASE_H
#define TREENODEBASE_H

#include <QList>
#include <memory>

// Requirement
// TDataType must have implement
// member function:
// 1. TDataType(QString);
// 2. bool operator==(const TDataType&rhs) const;
// 3. bool operator!=(const TDataType&rhs) const;
// 3. bool match(const QString& subStr) const;
// data member: bool isGroup; QString name;
// static data member: constexpr int COLUMN_COUNT; constexpr const char* HOR_HEADER_TITLES[COLUMN_COUNT];

template<typename Derived, typename TDataType>
class TreeNodeBase {
public:
  static Derived* create(const TDataType& _val,
                         const QList<Derived*>& _childs = {},
                         Derived* _parent = nullptr) {
    return new Derived(_val, _childs, _parent);
  }
  static std::unique_ptr<Derived> NewTreeNodeRoot() { return std::unique_ptr<Derived>{new Derived}; }
  using DataType = TDataType;

  virtual ~TreeNodeBase() { releaseAndClearChilds(); }
  Derived* derived() { return static_cast<Derived*>(this); }
  const Derived* derived() const { return static_cast<const Derived*>(this); }
  Derived* parent() const { return pParent; }
  int childsCount() const { return childs.size(); }
  int rowCount() const { return childsCount(); }

  void releaseAndClearChilds() {
    qDeleteAll(childs);
    childs.clear();
  }

  Derived* child(int row, int column = 0) const {
    if (0 <= row && row < childsCount()) {
      return childs[row];
    }
    return nullptr;
  }

  Derived* appendRow(Derived* child) {
    if (child) {
      childs.append(child);
      child->pParent = derived();
    }
    return child;
  }
  Derived* takeRow(int row) {
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
      return pParent->indexOfChild(derived());
    }
    return 0;
  }

  int indexOfChild(const Derived* child) const {
    return childs.indexOf(const_cast<Derived*>(child));  // 封装在这里
  }

  static int columnCount() {
    return TDataType::COLUMN_COUNT;
  }

  static constexpr auto horizontalHeaderTitles() -> decltype(TDataType::HOR_HEADER_TITLES)& {
    return TDataType::HOR_HEADER_TITLES;
  }

  bool isAncestorOf(const Derived* descendant) const;
  bool isDescendantOf(const Derived* ancestor) const;
  TDataType& value() {
    return val;
  }
  const TDataType& value() const {
    return val;
  }

  bool operator!=(const Derived& rhs) const { return !(*this == rhs); }
  bool operator==(const Derived& rhs) const;

  bool isGroup() const { return val.isGroup; }
  QString name() const { return val.name; }
  bool setName(const QString& newName) {
    if (val.name == newName) {
      return false; // unchanged
    }
    val.name = newName;
    return true;
  }
  bool filterAccept(const QString& text, QHash<const void*, bool>& passCache) const;

protected:
  TreeNodeBase() = default;
  TreeNodeBase(const TDataType& _val, const QList<Derived*>& _childs = {}, Derived* _parent = nullptr)
    : val{_val}
    , childs{_childs}
    , pParent{_parent} {}

  TDataType val;
  QList<Derived*> childs;
  Derived* pParent{nullptr};

  bool isCurrentNodeMatch(const QString& text, QHash<const void*, bool>& passCache) const;
  bool isChildNodeMatch(const QString& text, QHash<const void*, bool>& passCache) const;
  bool isParentNodeMatch(const QString& text, QHash<const void*, bool>& passCache) const;
};

#endif // TREENODEBASE_H
