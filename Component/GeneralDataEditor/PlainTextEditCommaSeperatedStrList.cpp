#include "PlainTextEditCommaSeperatedStrList.h"
#include <QKeyEvent>

QString PlainTextEditCommaSeperatedStrList::getSetDataEditRoleValue() const {
  return toPlainText().replace('\n', ',');
}

void PlainTextEditCommaSeperatedStrList::updateCurrentTextFromEditRole(const QVariant &editRoleData) {
  const QString editModeDisplayText = getDisplayString(editRoleData);
  PlainTextEditGeneral::updateCurrentTextFromEditRole(editModeDisplayText);
}

QString PlainTextEditCommaSeperatedStrList::displayTextFromDisplayRole(const QVariant &displayRoleData) {
  return getDisplayString(displayRoleData);
}

QString PlainTextEditCommaSeperatedStrList::getDisplayString(const QVariant &variantData) {
  return variantData.toString().replace(',', '\n');
}
