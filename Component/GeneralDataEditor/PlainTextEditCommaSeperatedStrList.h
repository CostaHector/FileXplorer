#ifndef PLAINTEXTEDITCOMMASEPERATEDSTRLIST_H
#define PLAINTEXTEDITCOMMASEPERATEDSTRLIST_H

#include "PlainTextEditGeneral.h"

class PlainTextEditCommaSeperatedStrList : public PlainTextEditGeneral {
public:
  using PlainTextEditGeneral::PlainTextEditGeneral;
  QString getSetDataEditRoleValue() const override;

  void updateCurrentTextFromEditRole(const QVariant &editRoleData) override;
  static QString displayTextFromDisplayRole(const QVariant &displayRoleData);

private:
  static QString getDisplayString(const QVariant &variantData);
};

#endif // PLAINTEXTEDITCOMMASEPERATEDSTRLIST_H
