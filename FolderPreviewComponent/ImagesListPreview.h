#ifndef IMAGESLISTPREVIEW_H
#define IMAGESLISTPREVIEW_H

#include <QListView>
#include "FolderListView.h"
class ImagesListPreview : public FolderListView {
 public:
  explicit ImagesListPreview();
  void InitViewSettings() override;
};

#endif  // IMAGESLISTPREVIEW_H
