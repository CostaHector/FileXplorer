#include "ComboBoxGeneral.h"
#include "ComboBoxString.h"
#include "ComboBoxEnum.h"
#include "Logger.h"

ComboBoxGeneral::ComboBoxGeneral(GeneralDataType::Type gDataType, QWidget *parent)
  : QComboBox{parent}
  , mDataType{gDataType} {
  setEditable(false);
}

ComboBoxGeneral *ComboBoxGeneral::create(int generalDataType, QWidget *parent) {
  GeneralDataType::Type typeE = static_cast<GeneralDataType::Type>(generalDataType);
  if (GeneralDataType::isComboBoxNeededInStringEditor(typeE)) {
    return new ComboBoxString(typeE, parent);
  } else if (GeneralDataType::isComboBoxNeededInStringToEnumEditor(typeE)) {
    return new ComboBoxEnum(typeE, parent);
  } else {
    LOG_E("type[%d] cannot edit in QComboBox", typeE);
  }
  return nullptr;
}

QString ComboBoxGeneral::displayText(int generalDataType, const QVariant &displayRoleData) {
  GeneralDataType::Type typeE = static_cast<GeneralDataType::Type>(generalDataType);
  if (GeneralDataType::isComboBoxNeededInStringEditor(typeE)) {
    return ComboBoxString::displayTextFromDisplayRole(typeE, displayRoleData);
  } else if (GeneralDataType::isComboBoxNeededInStringToEnumEditor(typeE)) {
    return ComboBoxEnum::displayTextFromDisplayRole(typeE, displayRoleData);
  } else {
    return displayRoleData.toString();
  }
}
