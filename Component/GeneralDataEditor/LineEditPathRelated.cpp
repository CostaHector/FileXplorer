#include "LineEditPathRelated.h"
#include "PathTool.h"

QVariant LineEditPathRelated::getSetDataEditRoleValue() const{
  return normalizePath(text());
}

QString LineEditPathRelated::normalizePath(const QString& rawPath) {
  return PathTool::normPath(rawPath);
}
