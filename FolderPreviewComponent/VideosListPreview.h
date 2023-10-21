#ifndef VIDEOSLISTPREVIEW_H
#define VIDEOSLISTPREVIEW_H

#include "FolderListView.h"

class VideosListPreview : public FolderListView {
 public:
  explicit VideosListPreview();
  auto InitViewSettings() -> void override;
};

#endif  // VIDEOSLISTPREVIEW_H
