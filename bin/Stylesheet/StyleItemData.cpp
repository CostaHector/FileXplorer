#include "StyleItemData.h"
#include "Logger.h"

constexpr int StyleItemData::COLUMN_COUNT;
constexpr int StyleItemData::EDITABLE_COLUMN;
constexpr const char* StyleItemData::HOR_HEADER_TITLES[];

StyleItemData::StyleItemData(const QString& _name)
  : name{_name}
  , isGroup{true} {}

StyleItemData::StyleItemData(const QString& _name, const QVariant& _defValue, const QVariant& _curValue, const DataTypeE& _dataType)
  : name{_name}
  , defValue{_defValue}
  , curValue{_curValue}
  , isGroup{false}
  , dataType{_dataType} {}

bool StyleItemData::setModifiedToValue(const QVariant& _newValue, bool& bNewValueValid) {
  // return value: changed or not
  // input value: valid or not
  if (_newValue == modifiedToValue) {
    bNewValueValid = true;
    return false;
  }
  const int srcType{_newValue.userType()};
  const int targetType{defValue.userType()};
  if (!_newValue.canConvert(targetType)) {
    LOG_W("_newValueType:%d, targetType:%d cannot convert", srcType, targetType);
    bNewValueValid = false;
    return false;
  }
  QVariant valueTestConverted = _newValue;
  if (!valueTestConverted.convert(targetType)) {
    LOG_W("_newValueType:%d, targetType:%d convert failed", srcType, targetType);
    bNewValueValid = false;
    return false;
  }
  bNewValueValid = true;
  modifiedToValue = valueTestConverted;
  return true;
}

bool StyleItemData::recoverToBackup() {
  // return value: need reset
  if (isGroup) {
    return false;
  }
  if (modifiedToValue == curValue) {
    return false;
  }
  modifiedToValue = curValue;
  return true;
}

bool StyleItemData::invalidateNewValue() {
  // return value: need invalidate
  if (isGroup) {
    return false;
  }
  modifiedToValue.clear();
  return true;
}

bool StyleItemData::match(const QString& subStr) const {
  if (subStr.isEmpty()) {
    return true;
  }
  if (isGroup) {
    return name.contains(subStr, Qt::CaseSensitivity::CaseInsensitive);
  }
  switch (dataType) {
    case NUMBER:
    case FONT_WEIGHT:
    case FONT_STYLE:
      return name.contains(subStr, Qt::CaseSensitivity::CaseInsensitive);
    case FONT_FAMILY:
    case COLOR:
      return name.contains(subStr, Qt::CaseSensitivity::CaseInsensitive)
             || defValue.toString().contains(subStr, Qt::CaseSensitivity::CaseInsensitive)
             || curValue.toString().contains(subStr, Qt::CaseSensitivity::CaseInsensitive)
             || modifiedToValue.toString().contains(subStr, Qt::CaseSensitivity::CaseInsensitive);
    default:
      return false;
  }
}
bool StyleItemData::match(const int& number) const {
  if (isGroup) {
    return false;
  }
  switch (dataType) {
    case NUMBER:
    case FONT_WEIGHT:
    case FONT_STYLE:
      return defValue.toInt() == number || curValue.toInt() == number || modifiedToValue.toInt() == number;
    case FONT_FAMILY:
    case COLOR:
    default:
      return false;
  }
}

QDataStream& operator<<(QDataStream& out, const StyleTreeNode& item) {
  LOG_E("StyleTreeNode does not support serialization. Please implement serialization functions.");
  return out;
}

QDataStream& operator>>(QDataStream& in, StyleTreeNode& item) {
  LOG_E("StyleTreeNode does not support serialization. Please implement serialization functions.");
  return in;
}

QString StyleTreeNode::GetConfigKey() const {
  QStringList keyPrefixListRev;
  keyPrefixListRev.reserve(5);
  const StyleTreeNode* node = this;
  while (node) {
    keyPrefixListRev.push_back(node->name());
    node = node->parent();
  }
  std::reverse(keyPrefixListRev.begin(), keyPrefixListRev.end());
  return keyPrefixListRev.join('/');
}
