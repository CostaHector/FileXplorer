#include "ThumbnailProcessActions.h"

ThumbnailProcessActions::ThumbnailProcessActions(QObject* parent) {
  _EXTRACT_1ST_IMG = new QAction(QIcon(":img/THUMBNAIL_EXTRACTOR"), "Thumbnail Extractor");
  _EXTRACT_1ST_IMG->setToolTip("Extract [0, 1) image from thumbnails under current view path");
  _EXTRACT_2ND_IMGS = new QAction("Extract [0, 2) imgs");
  _EXTRACT_2ND_IMGS->setToolTip("Extract [0, 2) images from thumbnails under current view path");
  _EXTRACT_4TH_IMGS = new QAction("Extract [0, 4) imgs");
  _EXTRACT_4TH_IMGS->setToolTip("Extract [0, 4) images from thumbnails under current view path");
  _CUSTOM_RANGE_IMGS = new QAction("Extract [b, e) imgs");
  _CUSTOM_RANGE_IMGS->setToolTip("Extract [b, e) images from thumbnails under current view path");

  _SKIP_IF_ALREADY_EXIST = new QAction(QIcon(""), "Skip extract if already exist");
  _SKIP_IF_ALREADY_EXIST->setCheckable(true);
  _SKIP_IF_ALREADY_EXIST->setChecked(true);
}

ThumbnailProcessActions& g_ThumbnailProcessActions() {
  static ThumbnailProcessActions ins;
  return ins;
}
