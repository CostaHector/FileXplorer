#ifndef VIDEOSFILESYSTEMMODEL_H
#define VIDEOSFILESYSTEMMODEL_H

#include <MyQFileSystemModel.h>

class VideosFileSystemModel : public MyQFileSystemModel {
 public:
  explicit VideosFileSystemModel(QObject* parent = nullptr);
};

#endif  // VIDEOSFILESYSTEMMODEL_H
