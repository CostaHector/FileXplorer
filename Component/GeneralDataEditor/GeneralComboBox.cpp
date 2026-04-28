#include "GeneralComboBox.h"
#include "StringComboBox.h"
#include "EnumComboBox.h"
#include "Logger.h"

GeneralComboBox::GeneralComboBox(GeneralDataType::Type gDataType, QWidget *parent)
  : QComboBox{parent}
  , mDataType{gDataType} {
  setEditable(false);
}

GeneralComboBox *GeneralComboBox::create(int generalDataType, QWidget *parent) {
  GeneralDataType::Type typeE = static_cast<GeneralDataType::Type>(generalDataType);
  if (GeneralDataType::isComboBoxNeededInStringEditor(typeE)) {
    return new StringComboBox(typeE, parent);
  } else if (GeneralDataType::isComboBoxNeededInStringToEnumEditor(typeE)) {
    return new EnumComboBox(typeE, parent);
  } else {
    LOG_E("type[%d] cannot edit in QComboBox", typeE);
  }
  return nullptr;
}

QString GeneralComboBox::displayText(int generalDataType, const QVariant &displayRoleData) {
  GeneralDataType::Type typeE = static_cast<GeneralDataType::Type>(generalDataType);
  if (GeneralDataType::isComboBoxNeededInStringEditor(typeE)) {
    return StringComboBox::displayTextFromDisplayRole(typeE, displayRoleData);
  } else if (GeneralDataType::isComboBoxNeededInStringToEnumEditor(typeE)) {
    return EnumComboBox::displayTextFromDisplayRole(typeE, displayRoleData);
  } else {
    return displayRoleData.toString();
  }
}
