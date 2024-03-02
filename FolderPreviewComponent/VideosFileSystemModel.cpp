#include "VideosFileSystemModel.h"

VideosFileSystemModel::VideosFileSystemModel(QObject* parent, bool showThumbnails_) : MyQFileSystemModel(parent) {
  setNameFilters({"*.mp4", "*.mov", "*.avi", "*.flv", "*.wmv", "*.mkv", "*.divx", "*.m4v", "*.mpg", "*.ts", "*.vob"});
}
