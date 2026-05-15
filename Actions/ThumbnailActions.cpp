#include "ThumbnailActions.h"
#include "PublicMacro.h"
#include "NotificatorMacro.h"
#include "MenuToolButton.h"
#include "BehaviorKey.h"
#include "Configuration.h"
#include <QInputDialog>

ThumbnailActions& ThumbnailActions::GetInst() {
  static ThumbnailActions ins;
  return ins;
}

void setVideoStoryBoardDimension(QAction* pAct, int rowCnt, int colCnt, int widthPixel) {
  pAct->setProperty("rowCnt", rowCnt);
  pAct->setProperty("colCnt", colCnt);
  pAct->setProperty("widthPixel", widthPixel);
}

void setThumbnailExtractIndexRange(QAction* pAct, int startIndex, int endIndex) {
  pAct->setProperty("startIndex", startIndex);
  pAct->setProperty("endIndex", endIndex);
}

ThumbnailActions::ThumbnailActions(QObject* /*parent*/) {
  CREATE_THUMBNAIL_FOR_A_PATH = new (std::nothrow) QAction(QIcon(":img/THUMBNAIL"), tr("Create thumbnail"), this);
  CREATE_THUMBNAIL_FOR_A_PATH->setToolTip("Create thumbnails for all folder(s) in current view");

  _SKIP_IF_ALREADY_EXIST = new (std::nothrow) QAction(QIcon(""), tr("Skip extract if already exist"), this);
  _SKIP_IF_ALREADY_EXIST->setCheckable(true);
  _SKIP_IF_ALREADY_EXIST->setChecked(true);

  _CREATE_THUMBNAIL_AG = new (std::nothrow) QActionGroup{this};
  {
    _CREATE_1_BY_1_THUMBNAIL = new (std::nothrow) QAction(QIcon(":img/THUMBNAILS_1_BY_1"), tr("Create 1x1 thumbnail"), this);
    _CREATE_1_BY_1_THUMBNAIL->setToolTip("Create a thumbnail for video(s), each image width:720px");
    setVideoStoryBoardDimension(_CREATE_1_BY_1_THUMBNAIL, 1, 1, 720);
    _CREATE_THUMBNAIL_AG->addAction(_CREATE_1_BY_1_THUMBNAIL);
  }
  {
    _CREATE_2_BY_2_THUMBNAIL = new (std::nothrow) QAction(QIcon(":img/THUMBNAILS_2_BY_2"), tr("Create 2x2 thumbnail"), this);
    _CREATE_2_BY_2_THUMBNAIL->setToolTip("Create a 2x2 thumbnails for video(s), each image width:720px");
    setVideoStoryBoardDimension(_CREATE_2_BY_2_THUMBNAIL, 2, 2, 720);
    _CREATE_THUMBNAIL_AG->addAction(_CREATE_2_BY_2_THUMBNAIL);
  }
  {
    _CREATE_3_BY_3_THUMBNAIL = new (std::nothrow) QAction(QIcon(":img/THUMBNAILS_3_BY_3"), tr("Create 3x3 thumbnail"), this);
    _CREATE_3_BY_3_THUMBNAIL->setToolTip("Create a 3x3 thumbnails for video(s), each image width:720px");
    setVideoStoryBoardDimension(_CREATE_3_BY_3_THUMBNAIL, 3, 3, 720);
    _CREATE_THUMBNAIL_AG->addAction(_CREATE_3_BY_3_THUMBNAIL);
  }
  {
    _CREATE_4_BY_4_THUMBNAIL = new (std::nothrow) QAction(QIcon(":img/THUMBNAILS_4_BY_4"), tr("Create 4x4 thumbnail"), this);
    _CREATE_4_BY_4_THUMBNAIL->setToolTip("Create a 4x4 thumbnails for video(s), each image width:480px");
    setVideoStoryBoardDimension(_CREATE_4_BY_4_THUMBNAIL, 4, 4, 480);
    _CREATE_THUMBNAIL_AG->addAction(_CREATE_4_BY_4_THUMBNAIL);
  }

  _RENAME_THUMBNAILS_FROM_POT_PLAYER = new (std::nothrow) QAction(QIcon{":img/RENAME_THUMBNAILS"}, tr("Rename thumbnails"), this);
  _RENAME_THUMBNAILS_FROM_POT_PLAYER->setToolTip("Rename thumbnails generated from potplayer");

  _EXTRACT_THUMBNAIL_AG = new (std::nothrow) QActionGroup{this};
  {
    _EXTRACT_1ST_IMG = new (std::nothrow) QAction(QIcon(":img/THUMBNAIL_EXTRACTOR_0_1"), tr("Thumbnail Extractor"), this);
    _EXTRACT_1ST_IMG->setToolTip("Extract the 1st image from thumbnails under current view path");
    setThumbnailExtractIndexRange(_EXTRACT_1ST_IMG, 0, 1);
    _EXTRACT_THUMBNAIL_AG->addAction(_EXTRACT_1ST_IMG);
  }
  {
    _EXTRACT_2ND_IMGS = new (std::nothrow) QAction(QIcon(":img/THUMBNAIL_EXTRACTOR_0_2"), tr("Extract [0, 2) imgs"), this);
    _EXTRACT_2ND_IMGS->setToolTip("Extract the [0, 2) images from thumbnails under current view path");
    setThumbnailExtractIndexRange(_EXTRACT_2ND_IMGS, 0, 2);
    _EXTRACT_THUMBNAIL_AG->addAction(_EXTRACT_2ND_IMGS);
  }
  {
    _EXTRACT_4TH_IMGS = new (std::nothrow) QAction(QIcon(":img/THUMBNAIL_EXTRACTOR_0_4"), tr("Extract [0, 4) imgs"), this);
    _EXTRACT_4TH_IMGS->setToolTip("Extract the [0, 4) images from thumbnails under current view path");
    setThumbnailExtractIndexRange(_EXTRACT_4TH_IMGS, 0, 4);
    _EXTRACT_THUMBNAIL_AG->addAction(_EXTRACT_4TH_IMGS);
  }

  _CUSTOM_RANGE_IMGS = new (std::nothrow) QAction(QIcon(":img/THUMBNAIL_EXTRACTOR_B_E"), tr("Extract [b, e) imgs"), this);
  _CUSTOM_RANGE_IMGS->setToolTip("Extract the [b, e) images from thumbnails under current view path");

  subscribe();
}

void ThumbnailActions::subscribe() {
  connect(_CREATE_THUMBNAIL_AG, &QActionGroup::triggered, this, [this](QAction* pAct) {
    CHECK_NULLPTR_RETURN_VOID(pAct);
    const int dimensionX = pAct->property("rowCnt").toInt();
    const int dimensionY = pAct->property("colCnt").toInt();
    const int widthPixel = pAct->property("widthPixel").toInt();
    emit crtVideoStoryBoard(dimensionX, dimensionY, widthPixel, isSkipIfAlreadyExist());
  });

  connect(_EXTRACT_THUMBNAIL_AG, &QActionGroup::triggered, this, [this](QAction* pAct) {
    CHECK_NULLPTR_RETURN_VOID(pAct);
    const int startIndex = pAct->property("startIndex").toInt();
    const int endIndex = pAct->property("endIndex").toInt();
    emit extractFrames(startIndex, endIndex, isSkipIfAlreadyExist());
  });

  connect(_CUSTOM_RANGE_IMGS, &QAction::triggered, this, &ThumbnailActions::emitCustomFramesRange);
}

void ThumbnailActions::emitCustomFramesRange() {
  bool ok = false;
  const QString input = QInputDialog::getText(nullptr,                              //
                                              "Extract image range",                //
                                              "Enter range (e.g., 1,3; 1,4; 1,7):", //
                                              QLineEdit::Normal,
                                              "0,9",
                                              &ok); //
  if (!ok || input.isEmpty()) {
    LOG_INFO_P("[Skip] User canceled or invalid input", "input[%s]", qPrintable(input));
    return;
  }
  const int dotIndex = input.indexOf(',');
  if (dotIndex == -1 || input.count(',') != 1) {
    LOG_WARN_NP("Invalid Range Format", input);
    return;
  }
  const int startIndex = input.leftRef(dotIndex).toInt();
  const int endIndex = input.midRef(dotIndex + 1).toInt();
  if (startIndex < 0 || endIndex < startIndex) {
    LOG_WARN_P("Invalid range", "Ensure 0 < beg <= end but[%s]", qPrintable(input));
    return;
  }
  emit extractFrames(startIndex, endIndex, isSkipIfAlreadyExist());
}

QWidget* ThumbnailActions::GetCreateThumbnailToolbar(QWidget* parent) {
  CHECK_NULLPTR_RETURN_NULLPTR(parent);

  QToolBar* thumbnailTB = new (std::nothrow) QToolBar{"Thumbnail", parent};
  CHECK_NULLPTR_RETURN_NULLPTR(thumbnailTB);
  thumbnailTB->setOrientation(Qt::Orientation::Vertical);

  {
    QList<QAction*> thumbnailActs{CREATE_THUMBNAIL_FOR_A_PATH, nullptr, _SKIP_IF_ALREADY_EXIST};
    auto* crtThumbnail = new (std::nothrow) MenuToolButton{thumbnailActs,                                 //
                                                           QToolButton::MenuButtonPopup,                  //
                                                           Qt::ToolButtonStyle::ToolButtonTextBesideIcon, //
                                                           IMAGE_SIZE::TABS_ICON_IN_MENU_16,              //
                                                           thumbnailTB};
    CHECK_NULLPTR_RETURN_NULLPTR(crtThumbnail);
    crtThumbnail->setDefaultAction(CREATE_THUMBNAIL_FOR_A_PATH);
    thumbnailTB->addWidget(crtThumbnail);
  }

  {
    QList<QAction*> vidStoryboardActs;
    vidStoryboardActs += _CREATE_THUMBNAIL_AG->actions();
    vidStoryboardActs += nullptr;
    vidStoryboardActs += _RENAME_THUMBNAILS_FROM_POT_PLAYER;
    auto* crtVidStoryBoard = new (std::nothrow) MenuToolButton{vidStoryboardActs,                             //
                                                               QToolButton::MenuButtonPopup,                  //
                                                               Qt::ToolButtonStyle::ToolButtonTextBesideIcon, //
                                                               IMAGE_SIZE::TABS_ICON_IN_MENU_16,              //
                                                               thumbnailTB};                                  //
    CHECK_NULLPTR_RETURN_NULLPTR(crtVidStoryBoard);
    crtVidStoryBoard->InitDefaultActionFromQSetting(BehaviorKey::THUMBNAILS_DIMENSION, true);
    thumbnailTB->addWidget(crtVidStoryBoard);
  }

  {
    QList<QAction*> extractThumbnailActions;
    extractThumbnailActions += _EXTRACT_THUMBNAIL_AG->actions();
    extractThumbnailActions.push_back(nullptr);
    extractThumbnailActions.push_back(_CUSTOM_RANGE_IMGS);
    auto* extractTB = new (std::nothrow) MenuToolButton{extractThumbnailActions,                       //
                                                        QToolButton::MenuButtonPopup,                  //
                                                        Qt::ToolButtonStyle::ToolButtonTextBesideIcon, //
                                                        IMAGE_SIZE::TABS_ICON_IN_MENU_16,              //
                                                        thumbnailTB};                                  //
    CHECK_NULLPTR_RETURN_NULLPTR(extractTB);
    extractTB->InitDefaultActionFromQSetting(BehaviorKey::EXTRACT_CHOICE, true);
    thumbnailTB->addWidget(extractTB);
  }
  return thumbnailTB;
}
