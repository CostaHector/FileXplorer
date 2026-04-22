#include "KV.h"

QList<const KV*>& KV::GetEditableKVs() {
  static QList<const KV*> editableKVs;
  return editableKVs;
}

KV::KV(const QString& name_, const QVariant& v_, const ValueChecker& checker_, bool bUserEditable)  //
  : name{name_}, v{v_}, checker{checker_} {
  if (bUserEditable) {
    GetEditableKVs().push_back(this);
  }
}

QString KV::InitialValueToString() const {
  return checker.valueToString(v);
}

QString KV::valueToString(const QVariant& v_) const {
  return checker.valueToString(v_);
}
