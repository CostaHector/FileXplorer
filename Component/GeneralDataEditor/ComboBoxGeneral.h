#ifndef COMBOBOXGENERAL_H
#define COMBOBOXGENERAL_H

#include "GeneralDataType.h"
#include <QComboBox>

class ComboBoxGeneral : public QComboBox {
public:
  explicit ComboBoxGeneral(GeneralDataType::Type gDataType, QWidget *parent = nullptr);
  GeneralDataType::Type type() const { return mDataType; }
  virtual QVariant getSetDataEditRoleValue() const = 0;
  virtual void updateCurrentTextFromEditRole(const QVariant &editRoleData) { setCurrentText(editRoleData.toString()); }

  static ComboBoxGeneral *create(int generalDataType, QWidget *parent);
  static QString displayText(int generalDataType, const QVariant &displayRoleData);

private:
  GeneralDataType::Type mDataType;
};

#endif // COMBOBOXGENERAL_H
