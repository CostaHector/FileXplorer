#ifndef IMAGESLISTPREVIEW_H
#define IMAGESLISTPREVIEW_H

#include <QListView>
#include "FolderListView.h"
class ImagesListPreview : public FolderListView {
 public:
  explicit ImagesListPreview(QWidget* parent = nullptr);
  void InitViewSettings() override;
};

#endif  // IMAGESLISTPREVIEW_H
