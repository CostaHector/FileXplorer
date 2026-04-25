#include "StyleItemData.h"
#include "ValueChecker.h"
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

StyleItemData::StyleItemData(const QString& _name, const QVariant& _defValue, const QVariant& _curValue, const GeneralDataType::Type& _dataType)
  : name{_name}
  , defValue{_defValue}
  , curValue{_curValue}
  , dataType{_dataType}
  , isGroup{_dataType == GeneralDataType::Type::GROUP} {}

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

  using StringValidChecker = bool (*)(const QString& s);
  static const auto emptyStringWillClear = [](const QVariant& newValue, QVariant& modifiedTo, bool& acceptInput, StringValidChecker checker) -> bool{
    // return value: changed
    if (newValue.toString().isEmpty()) { // 空字符串视为重置为QVariant()
      modifiedTo.clear();
      return acceptInput = true;
    }
    if (checker == nullptr || checker(newValue.toString())) {
      modifiedTo = newValue;
      return acceptInput = true;
    }
    return acceptInput = false;
  };

  switch (dataType) {
    case GeneralDataType::Type::PLAIN_STR:
    case GeneralDataType::Type::MULTI_LINE_STR:
    case GeneralDataType::Type::FONT_FAMILY: {
      return emptyStringWillClear(_newValue, modifiedToValue, bNewValueAccept, nullptr);
    }
    case GeneralDataType::Type::PLAIN_BOOL: {
      modifiedToValue = _newValue.toBool();
      return bNewValueAccept = true;
    }
    case GeneralDataType::Type::PLAIN_INT: {
      // NUMBER类型的数值字符串{"123"}, 或者FONT_WEIGHT, FONT_STYLE下拉框提供的QVariant{123};
      const int newValueInt = _newValue.toInt(&bNewValueAccept);
      if (bNewValueAccept) {
        modifiedToValue = newValueInt;
      }
      return bNewValueAccept;
    }
    case GeneralDataType::Type::COLOR: {
      return emptyStringWillClear(_newValue, modifiedToValue, bNewValueAccept, QColor::isValidColor);
    }
    case GeneralDataType::Type::FILE_PATH: {
      return emptyStringWillClear(_newValue, modifiedToValue, bNewValueAccept, ValueChecker::GeneralFilePathStrChecker<false>);
    }
    case GeneralDataType::Type::IMAGE_PATH_OPTIONAL: {
      return emptyStringWillClear(_newValue, modifiedToValue, bNewValueAccept, ValueChecker::GeneralFilePathStrChecker<true>);
    }
    case GeneralDataType::Type::FOLDER_PATH: {
      return emptyStringWillClear(_newValue, modifiedToValue, bNewValueAccept, ValueChecker::GeneralFolderPathStrChecker);
    }
    default: {
      modifiedToValue = _newValue;
      return bNewValueAccept = true;
    }
  }
}

bool StyleItemData::modifiedColorTo(const QString& newColor) {
  // return value: changed
  if (isGroup) {
    return false;
  }
  if (dataType != GeneralDataType::Type::COLOR) {
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
    case GeneralDataType::Type::PLAIN_INT:
    case GeneralDataType::Type::FONT_WEIGHT:
    case GeneralDataType::Type::FONT_STYLE:
      return name.contains(subStr, caseMatter);
    case GeneralDataType::Type::FONT_FAMILY:
    case GeneralDataType::Type::COLOR:
    case GeneralDataType::Type::FILE_PATH:
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
    case GeneralDataType::Type::PLAIN_INT:
    case GeneralDataType::Type::FONT_WEIGHT:
    case GeneralDataType::Type::FONT_STYLE: {
      bool bIsInt{false};
      return (defValue.toInt(&bIsInt) == number && bIsInt)    //
             || (curValue.toInt(&bIsInt) == number && bIsInt) //
             || (modifiedToValue.toInt(&bIsInt) == number && bIsInt);
    }
    case GeneralDataType::Type::FONT_FAMILY:
    case GeneralDataType::Type::COLOR:
    case GeneralDataType::Type::FILE_PATH:
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
