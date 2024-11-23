#include "ThumbnailProcessActions.h"

ThumbnailProcessActions::ThumbnailProcessActions(QObject* parent) {
  _EXTRACT_1ST_IMG = new QAction(QIcon(":img/THUMBNAIL_EXTRACTOR"), "1st img");
  _EXTRACT_2ND_IMGS = new QAction("first 2 imgs");
  _EXTRACT_4TH_IMGS = new QAction("first 4 imgs");
  _CUSTOM_RANGE_IMGS = new QAction("custom range imgs [b, e)");
  _SKIP_IF_ALREADY_EXIST = new QAction(QIcon(""), "Skip extract if already exist");
  _SKIP_IF_ALREADY_EXIST->setCheckable(true);
  _SKIP_IF_ALREADY_EXIST->setChecked(true);
}

ThumbnailProcessActions& g_ThumbnailProcessActions() {
  static ThumbnailProcessActions ins;
  return ins;
}
