#ifndef STYLEITEMDATA_H
#define STYLEITEMDATA_H

#include "TreeNodeBase.h"
#include <QString>
#include <QVariant>

struct StyleItemData final {
  enum DataTypeE {
    NUMBER,
    FONT_FAMILY,
    FONT_WEIGHT,
    FONT_STYLE,
    COLOR,
  };

  enum Role { // -DEF_BEGIN_ROLE represent column
    DEF_BEGIN_ROLE = Qt::UserRole + 1,
    DEF_NAME_TEXT_ROLE = DEF_BEGIN_ROLE,
    DEFAULT_DATA_ROLE,
    CURRENT_DATA_ROLE,
    NEW_DATA_ROLE,
    IS_GROUP_ROLE,
    DATA_TYPE_ROLE,
    INVALID_BUTT_ROLE,
  };

  StyleItemData() = default;
  explicit StyleItemData(const QString& _name);
  StyleItemData(const QString& _name, const QVariant& _defValue, const QVariant& _curValue, const DataTypeE& _dataType);

  bool operator==(const StyleItemData& other) const { //
    return name == other.name && isGroup == other.isGroup && defValue == other.defValue && curValue == other.curValue;
  }

  bool operator!=(const StyleItemData& other) const { return !(*this == other); }

  bool isValid() const { return !name.isEmpty() && (isGroup || (defValue.isValid() && curValue.isValid())); }
  bool isModifiedToValueValid() const { return modifiedToValue.isValid(); }
  bool setModifiedToValue(const QVariant& _newValue, bool& bNewValueValid);
  bool recoverToBackup();
  bool invalidateNewValue();
  bool match(const QString& subStr) const;
  bool match(const int& number) const;

  QString name;                                 // 显示名称
  QVariant defValue, curValue, modifiedToValue; // def, backup, modified to
  DataTypeE dataType{NUMBER};
  bool isGroup{true}; // 是否为分组

  static constexpr int EDITABLE_COLUMN = NEW_DATA_ROLE - DEF_BEGIN_ROLE;
  static constexpr int COLUMN_COUNT = IS_GROUP_ROLE - DEF_BEGIN_ROLE;
  static constexpr const char* HOR_HEADER_TITLES[COLUMN_COUNT]{"Name", "Default Value", "Backup Value", "Modified To Value"};
};

struct StyleTreeNode;
extern template class TreeNodeBase<StyleTreeNode, StyleItemData>;

struct StyleTreeNode final : public TreeNodeBase<StyleTreeNode, StyleItemData> {
  using TreeNodeBase<StyleTreeNode, StyleItemData>::TreeNodeBase;
  QString GetConfigKey() const;
};

QDataStream& operator<<(QDataStream& out, const StyleTreeNode& item);
QDataStream& operator>>(QDataStream& in, StyleTreeNode& item);

#endif // STYLEITEMDATA_H
