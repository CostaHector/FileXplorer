#include "StyleItemData.h"
#include "Logger.h"
#include <QColor>
#include <QFile>

constexpr int StyleItemData::NAME_COLUMN;
constexpr int StyleItemData::DEF_COLUMN;
constexpr int StyleItemData::CUR_COLUMN;
constexpr int StyleItemData::COLUMN_COUNT;
constexpr int StyleItemData::EDITABLE_COLUMN;
constexpr const char* StyleItemData::HOR_HEADER_TITLES[];
constexpr int StyleItemData::SORT_COLUMN;

StyleItemData::StyleItemData(const QString& _name, const QVariant& _defValue, const QVariant& _curValue, const DataTypeE& _dataType)
  : name{_name}
  , defValue{_defValue}
  , curValue{_curValue}
  , dataType{_dataType}
  , isGroup{_dataType == DataTypeE::GROUP} {}

bool StyleItemData::modifyValueTo(const QVariant& _newValue, bool& bNewValueAccept) {
  // return value: changed or not
  // input value: accept or not
  if (isGroup) {
    bNewValueAccept = false;
    return false;
  }

  if (_newValue == modifiedToValue) {
    bNewValueAccept = true;
    return false;
  }

  if (dataType == DataTypeE::COLOR) {
    if (_newValue.toString().isEmpty()) {
      bNewValueAccept = true;
      modifiedToValue.clear();
      return true;
    }
    if (bNewValueAccept = QColor::isValidColor(_newValue.toString())) {
      modifiedToValue = _newValue;
      return true;
    }
    return false;
  } else if (dataType == DataTypeE::FONT_FAMILY) {
    if (_newValue.toString().isEmpty()) {
      bNewValueAccept = true;
      modifiedToValue.clear();
      return true;
    }
    bNewValueAccept = true;
    modifiedToValue = _newValue;
    return true;
  } else if (dataType == DataTypeE::FILE_PATH) {
    const QString& newFilePath = _newValue.toString();
    if (bNewValueAccept = QFile::exists(newFilePath)) {
      modifiedToValue = newFilePath;
    }
    return bNewValueAccept;
  }
  // NUMBER类型的数值字符串{"123"}, 或者FONT_WEIGHT, FONT_STYLE下拉框提供的QVariant{123};
  const int newValueInt = _newValue.toInt(&bNewValueAccept);
  if (bNewValueAccept) {
    modifiedToValue = newValueInt;
    return true;
  }
  return false;
}

bool StyleItemData::modifiedColorTo(const QString& newColor) {
  // return value: changed
  if (isGroup) {
    return false;
  }
  if (dataType != DataTypeE::COLOR) {
    return false;
  }
  if (modifiedToValue.toString() == newColor) {
    return false;
  }
  modifiedToValue = newColor;
  return true;
}

bool StyleItemData::recoverToDefault() {
  // return value: need reset
  if (isGroup) {
    return false;
  }
  if (modifiedToValue == defValue) {
    return false;
  }
  modifiedToValue = defValue;
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
  if (!modifiedToValue.isValid()) {
    return false;
  }
  modifiedToValue.clear();
  return true;
}

bool StyleItemData::match(const QString& subStr, const Qt::CaseSensitivity caseMatter) const {
  if (subStr.isEmpty()) {
    return true;
  }
  if (isGroup) {
    return name.contains(subStr, caseMatter);
  }
  switch (dataType) {
    case NUMBER:
    case FONT_WEIGHT:
    case FONT_STYLE:
      return name.contains(subStr, caseMatter);
    case FONT_FAMILY:
    case COLOR:
    case FILE_PATH:
      return name.contains(subStr, caseMatter)                   //
             || defValue.toString().contains(subStr, caseMatter) //
             || curValue.toString().contains(subStr, caseMatter) //
             || modifiedToValue.toString().contains(subStr, caseMatter);
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
    case FONT_STYLE: {
      bool bIsInt{false};
      return (defValue.toInt(&bIsInt) == number && bIsInt)    //
             || (curValue.toInt(&bIsInt) == number && bIsInt) //
             || (modifiedToValue.toInt(&bIsInt) == number && bIsInt);
    }
    case FONT_FAMILY:
    case COLOR:
    case FILE_PATH:
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
