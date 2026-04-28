#ifndef LINEEDITPATHRELATED_H
#define LINEEDITPATHRELATED_H

#include "LineEditGeneral.h"

class LineEditPathRelated : public LineEditGeneral {
public:
  using LineEditGeneral::LineEditGeneral;
  QVariant getSetDataEditRoleValue() const override;
  static QString normalizePath(const QString& rawPath);
};

#endif // LINEEDITPATHRELATED_H
