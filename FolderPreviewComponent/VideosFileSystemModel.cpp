#include "VideosFileSystemModel.h"
#include "PublicVariable.h"

VideosFileSystemModel::VideosFileSystemModel(QObject* parent) : MyQFileSystemModel(parent) {
  setFilter(QDir::Drives|QDir::Files);
  setNameFilters(TYPE_FILTER::VIDEO_TYPE_SET);
  setNameFilterDisables(false);
}
