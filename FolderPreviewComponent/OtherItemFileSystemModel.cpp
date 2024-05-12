#include "OtherItemFileSystemModel.h"
#include "PublicVariable.h"

OtherItemFileSystemModel::OtherItemFileSystemModel(QObject* parent) : MyQFileSystemModel(parent) {
  setFilter(QDir::Drives | QDir::Files);
  setNameFilters(TYPE_FILTER::TEXT_TYPE_SET);
  setNameFilterDisables(false);
}
