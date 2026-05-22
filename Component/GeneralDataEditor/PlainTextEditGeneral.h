#ifndef PLAINTEXTEDITGENERAL_H
#define PLAINTEXTEDITGENERAL_H

#include "GeneralDataType.h"
#include <QPlainTextEdit>

class PlainTextEditGeneral : public QPlainTextEdit {
public:
  explicit PlainTextEditGeneral(GeneralDataType::Type gDataType, QWidget *parent = nullptr);
  GeneralDataType::Type type() const { return mDataType; }
  virtual QString getSetDataEditRoleValue() const { return toPlainText();}
  virtual void updateCurrentTextFromEditRole(const QVariant &editRoleData) { setPlainText(editRoleData.toString()); }

  static PlainTextEditGeneral *create(int generalDataType, QWidget *parent);
  static int linesCount(int generalDataType, const QVariant &displayRoleData);

private:
  GeneralDataType::Type mDataType;
};


#endif // PLAINTEXTEDITGENERAL_H
