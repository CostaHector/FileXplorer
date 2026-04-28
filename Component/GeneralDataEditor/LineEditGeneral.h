#ifndef LINEEDITGENERAL_H
#define LINEEDITGENERAL_H

#include "GeneralDataType.h"
#include <QLineEdit>

class LineEditGeneral : public QLineEdit {
public:
  explicit LineEditGeneral(GeneralDataType::Type gDataType, const char *iconUrl = nullptr, QWidget *parent = nullptr);
  GeneralDataType::Type type() const { return mDataType; }
  virtual QVariant getSetDataEditRoleValue() const { return text(); }

  static LineEditGeneral *create(int generalDataType, QWidget *parent);

protected:
  virtual void onActionTriggered() = 0;

private:
  GeneralDataType::Type mDataType;
};

#endif // LINEEDITGENERAL_H
