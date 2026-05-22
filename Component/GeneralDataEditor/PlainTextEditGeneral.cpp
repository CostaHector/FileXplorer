#include "PlainTextEditGeneral.h"
#include "PlainTextEditCommaSeperatedStrList.h"
#include "Logger.h"

PlainTextEditGeneral::PlainTextEditGeneral(GeneralDataType::Type gDataType, QWidget *parent)
  : QPlainTextEdit{parent}
  , mDataType{gDataType} {
}

PlainTextEditGeneral *PlainTextEditGeneral::create(int generalDataType, QWidget *parent) {
  GeneralDataType::Type typeE = static_cast<GeneralDataType::Type>(generalDataType);
  if (!GeneralDataType::isPlainTextEditNeededInEditor(typeE)) {
    LOG_E("type[%d] cannot edit in QPlainTextEdit", typeE);
    return nullptr;
  }
  if (typeE == GeneralDataType::COMMA_SEPERATED_STR_LIST) {
    return new PlainTextEditCommaSeperatedStrList{typeE, parent};
  }
  return new PlainTextEditGeneral(typeE, parent);
}

int PlainTextEditGeneral::linesCount(int generalDataType, const QVariant &displayRoleData) {
  GeneralDataType::Type typeE = static_cast<GeneralDataType::Type>(generalDataType);
  if (!GeneralDataType::isPlainTextEditNeededInEditor(typeE)) {
    return -1;
  }
  return displayRoleData.isNull() ? 0 : displayRoleData.toString().count('\n') + 1;
}
