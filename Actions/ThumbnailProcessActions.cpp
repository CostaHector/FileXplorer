#include "ThumbnailProcessActions.h"
#include "MenuToolButton.h"
#include "MemoryKey.h"

ThumbnailProcessActions::ThumbnailProcessActions(QObject* /*parent*/) {
  _CREATE_1_BY_1_THUMBNAIL = new (std::nothrow) QAction(QIcon(":img/THUMBNAILS_1_BY_1"), "Create 1x1 thumbnail");
  _CREATE_1_BY_1_THUMBNAIL->setToolTip("Create a thumbnail for video(s), each image width:720px");
  _CREATE_2_BY_2_THUMBNAIL = new (std::nothrow) QAction(QIcon(":img/THUMBNAILS_2_BY_2"), "Create 2x2 thumbnail");
  _CREATE_2_BY_2_THUMBNAIL->setToolTip("Create a 2x2 thumbnails for video(s), each image width:720px");
  _CREATE_3_BY_3_THUMBNAIL = new (std::nothrow) QAction(QIcon(":img/THUMBNAILS_3_BY_3"), "Create 3x3 thumbnail");
  _CREATE_3_BY_3_THUMBNAIL->setToolTip("Create a 3x3 thumbnails for video(s), each image width:720px");
  _CREATE_4_BY_4_THUMBNAIL = new (std::nothrow) QAction("Create 4x4 thumbnail");
  _CREATE_4_BY_4_THUMBNAIL->setToolTip("Create a 4x4 thumbnails for video(s), each image width:480px");

  _CREATE_THUMBNAIL_AG = new (std::nothrow) QActionGroup{this};
  _CREATE_THUMBNAIL_AG->addAction(_CREATE_1_BY_1_THUMBNAIL);
  _CREATE_THUMBNAIL_AG->addAction(_CREATE_2_BY_2_THUMBNAIL);
  _CREATE_THUMBNAIL_AG->addAction(_CREATE_3_BY_3_THUMBNAIL);
  _CREATE_THUMBNAIL_AG->addAction(_CREATE_4_BY_4_THUMBNAIL);
  mCreateThumbnailDimension[_CREATE_1_BY_1_THUMBNAIL] = {1, 1, 720};
  mCreateThumbnailDimension[_CREATE_2_BY_2_THUMBNAIL] = {2, 2, 720};
  mCreateThumbnailDimension[_CREATE_3_BY_3_THUMBNAIL] = {3, 3, 720};
  mCreateThumbnailDimension[_CREATE_3_BY_3_THUMBNAIL] = {4, 4, 480};

  _THUMBNAIL_SAMPLE_PERIOD = new (std::nothrow) QAction("Sample period");
  _THUMBNAIL_SAMPLE_PERIOD->setToolTip("Set thumbnail image sample period(20 second by default)");

  _EXTRACT_1ST_IMG = new (std::nothrow) QAction(QIcon(":img/THUMBNAIL_EXTRACTOR_0_1"), "Thumbnail Extractor");
  _EXTRACT_1ST_IMG->setToolTip("Extract the 1st image from thumbnails under current view path");
  _EXTRACT_2ND_IMGS = new (std::nothrow) QAction(QIcon(":img/THUMBNAIL_EXTRACTOR_0_2"), "Extract [0, 2) imgs");
  _EXTRACT_2ND_IMGS->setToolTip("Extract the [0, 2) images from thumbnails under current view path");
  _EXTRACT_4TH_IMGS = new (std::nothrow) QAction(QIcon(":img/THUMBNAIL_EXTRACTOR_0_4"), "Extract [0, 4) imgs");
  _EXTRACT_4TH_IMGS->setToolTip("Extract the [0, 4) images from thumbnails under current view path");
  _EXTRACT_THUMBNAIL_AG = new (std::nothrow) QActionGroup{this};
  _EXTRACT_THUMBNAIL_AG->addAction(_EXTRACT_1ST_IMG);
  _EXTRACT_THUMBNAIL_AG->addAction(_EXTRACT_2ND_IMGS);
  _EXTRACT_THUMBNAIL_AG->addAction(_EXTRACT_4TH_IMGS);
  mExtractThumbnailRange[_EXTRACT_1ST_IMG] = {0, 1};
  mExtractThumbnailRange[_EXTRACT_2ND_IMGS] = {0, 2};
  mExtractThumbnailRange[_EXTRACT_4TH_IMGS] = {0, 4};

  _CUSTOM_RANGE_IMGS = new (std::nothrow) QAction(QIcon(":img/THUMBNAIL_EXTRACTOR_B_E"), "Extract [b, e) imgs");
  _CUSTOM_RANGE_IMGS->setToolTip("Extract the [b, e) images from thumbnails under current view path");

  _SKIP_IF_ALREADY_EXIST = new (std::nothrow) QAction(QIcon(""), "Skip extract if already exist");
  _SKIP_IF_ALREADY_EXIST->setCheckable(true);
  _SKIP_IF_ALREADY_EXIST->setChecked(true);
}

QToolBar* ThumbnailProcessActions::GetThumbnailToolbar(QWidget* parent) {
  QList<QAction*> crtThumbnailActions;
  crtThumbnailActions += _CREATE_THUMBNAIL_AG->actions();
  crtThumbnailActions.push_back(nullptr);
  crtThumbnailActions.push_back(_THUMBNAIL_SAMPLE_PERIOD);
  auto* createTB = new (std::nothrow) MenuToolButton{crtThumbnailActions,//
                                                         QToolButton::MenuButtonPopup,//
                                                         Qt::ToolButtonStyle::ToolButtonTextBesideIcon,//
                                                         IMAGE_SIZE::TABS_ICON_IN_MENU_16,//
                                                         parent};//
  CHECK_NULLPTR_RETURN_NULLPTR(createTB);
  createTB->FindAndSetDefaultAction(Configuration().value(MemoryKey::DEFAULT_THUMBNAILS_DIMENSION.name, MemoryKey::DEFAULT_THUMBNAILS_DIMENSION.v).toString());
  createTB->MemorizeCurrentAction(MemoryKey::DEFAULT_THUMBNAILS_DIMENSION.name);

  QList<QAction*> extractThumbnailActions;
  extractThumbnailActions += _EXTRACT_THUMBNAIL_AG->actions();
  extractThumbnailActions.push_back(nullptr);
  extractThumbnailActions.push_back(_CUSTOM_RANGE_IMGS);
  extractThumbnailActions.push_back(nullptr);
  extractThumbnailActions.push_back(_SKIP_IF_ALREADY_EXIST);
  auto* extractTB = new (std::nothrow) MenuToolButton{extractThumbnailActions,//
                                                          QToolButton::MenuButtonPopup,//
                                                          Qt::ToolButtonStyle::ToolButtonTextBesideIcon,//
                                                          IMAGE_SIZE::TABS_ICON_IN_MENU_16,//
                                                          parent};//
  CHECK_NULLPTR_RETURN_NULLPTR(extractTB);
  extractTB->FindAndSetDefaultAction(Configuration().value(MemoryKey::DEFAULT_EXTRACT_CHOICE.name, MemoryKey::DEFAULT_EXTRACT_CHOICE.v).toString());
  extractTB->MemorizeCurrentAction(MemoryKey::DEFAULT_EXTRACT_CHOICE.name);

  QToolBar* thumbnailTB = new (std::nothrow) QToolBar{"Thumbnail", parent};
  CHECK_NULLPTR_RETURN_NULLPTR(thumbnailTB);
  thumbnailTB->addWidget(createTB);
  thumbnailTB->addWidget(extractTB);
  thumbnailTB->setOrientation(Qt::Orientation::Vertical);
  return thumbnailTB;
}

ThumbnailProcessActions& g_ThumbnailProcessActions() {
  static ThumbnailProcessActions ins;
  return ins;
}
