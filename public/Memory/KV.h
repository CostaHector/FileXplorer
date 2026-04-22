#ifndef KV_H
#define KV_H

#include <QString>
#include "ValueChecker.h"

class ConfigsModel;
class KV {
public:
  friend class ConfigsModel;
  KV(const QString& name_, const QVariant& v_, const ValueChecker& checker_, bool bUserEditable = false);

  QString InitialValueToString() const;
  QString valueToString(const QVariant& v_) const;
  bool isPath() const { return checker.isPath(); }

  QString name;
  QVariant v;

private:
  static QList<const KV*>& GetEditableKVs();
  ValueChecker checker;
};

#endif // KV_H
