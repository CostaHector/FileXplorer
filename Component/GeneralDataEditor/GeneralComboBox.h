#ifndef GENERALCOMBOBOX_H
#define GENERALCOMBOBOX_H

#include "GeneralDataType.h"
#include <QComboBox>

class GeneralComboBox : public QComboBox {
public:
  explicit GeneralComboBox(GeneralDataType::Type gDataType, QWidget *parent = nullptr);
  GeneralDataType::Type type() const { return mDataType; }
  virtual QVariant getSetDataEditRoleValue() const = 0;
  virtual void updateCurrentTextFromEditRole(const QVariant &editRoleData) { setCurrentText(editRoleData.toString()); }

  static GeneralComboBox *create(int generalDataType, QWidget *parent);
  static QString displayText(int generalDataType, const QVariant &displayRoleData);

private:
  GeneralDataType::Type mDataType;
};

#endif // GENERALCOMBOBOX_H
