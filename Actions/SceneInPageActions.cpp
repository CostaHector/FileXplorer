#include "SceneInPageActions.h"
#include "DropListToolButton.h"
#include "MemoryKey.h"
#include <QLineEdit>
#include <QToolBar>
#include <QLabel>
#include <QIntValidator>

SceneInPageActions& g_SceneInPageActions() {
  static SceneInPageActions ins;
  return ins;
}

SceneInPageActions::SceneInPageActions(QObject* parent) : QObject{parent} {
  _COMBINE_MEDIAINFOS_JSON = new (std::nothrow) QAction(QIcon(":img/UPDATE_JSON_THEN_SCN"), "Update infos", this);
  _COMBINE_MEDIAINFOS_JSON->setShortcut(QKeySequence(Qt::Key_F5));
  _COMBINE_MEDIAINFOS_JSON->setShortcutVisibleInContextMenu(true);
  _COMBINE_MEDIAINFOS_JSON->setToolTip(QString("<b>%1 (%2)</b><br/> Combine Videos/Images infos Into json files. Then generate scn file from valid "
                                               "json file(s). This operation may update json file contents")
                                           .arg(_COMBINE_MEDIAINFOS_JSON->text(), _COMBINE_MEDIAINFOS_JSON->shortcut().toString()));

  _UPDATE_SCN_ONLY = new (std::nothrow) QAction(QIcon(":img/UPDATE_SCN_FILE"), "Update Scn", this);
  _UPDATE_SCN_ONLY->setShortcutVisibleInContextMenu(true);
  _UPDATE_SCN_ONLY->setToolTip(
      QString("<b>%1 (%2)</b><br/> Only update scn file from valid json file(s). This operation will not update json file(s)").arg(_UPDATE_SCN_ONLY->text(), _UPDATE_SCN_ONLY->shortcut().toString()));

  _BY_MOVIE_NAME = new (std::nothrow) QAction(QIcon(":img/SORTING_FILE_FOLDER"), "Movie Name", this);
  _BY_MOVIE_NAME->setCheckable(true);
  _BY_MOVIE_SIZE = new (std::nothrow) QAction("Movie Size", this);
  _BY_MOVIE_SIZE->setCheckable(true);
  _BY_RATE = new (std::nothrow) QAction("Rate", this);
  _BY_RATE->setCheckable(true);
  _BY_UPLOADED_TIME = new (std::nothrow) QAction("Uploaded Time", this);
  _BY_UPLOADED_TIME->setCheckable(true);

  _REVERSE_SORT = new (std::nothrow) QAction("Reverse", this);
  _REVERSE_SORT->setCheckable(true);

  _ORDER_AG = new (std::nothrow) QActionGroup(this);
  _ORDER_AG->addAction(_BY_MOVIE_NAME);
  _ORDER_AG->addAction(_BY_MOVIE_SIZE);
  _ORDER_AG->addAction(_BY_RATE);
  _ORDER_AG->addAction(_BY_UPLOADED_TIME);
  _ORDER_AG->setExclusionPolicy(QActionGroup::ExclusionPolicy::ExclusiveOptional);

  _GROUP_BY_PAGE = new (std::nothrow) QAction("Enable Pagination display", this);
  _GROUP_BY_PAGE->setCheckable(true);
  _GROUP_BY_PAGE->setChecked(false);

  _THE_FIRST_PAGE = new (std::nothrow) QAction(QIcon(":img/PAGINATION_START"), "Start", this);
  _LAST_PAGE = new (std::nothrow) QAction(QIcon(":img/PAGINATION_LAST"), "Last", this);
  _NEXT_PAGE = new (std::nothrow) QAction(QIcon(":img/PAGINATION_NEXT"), "Next", this);
  _THE_LAST_PAGE = new (std::nothrow) QAction(QIcon(":img/PAGINATION_END"), "End", this);
}

bool SceneInPageActions::InitWidget() {
  if (mOrderTB != nullptr || mEnablePageTB != nullptr || mPagesSelectTB != nullptr) {
    return true;
  }
  mOrderTB = GetOrderToolBar();
  if (mOrderTB == nullptr) {
    qWarning("mOrderTB is nullptr");
    return false;
  }
  mEnablePageTB = GetPagesRowByColumnToolBar();
  if (mEnablePageTB == nullptr) {
    qWarning("mEnablePageTB is nullptr");
    return false;
  }
  mImageSizeTB = GetImageSizeToolBar();
  if (mImageSizeTB == nullptr) {
    qWarning("mImageSizeTB is nullptr");
    return false;
  }
  return true;
}

QToolBar* SceneInPageActions::GetOrderToolBar() {
  auto* orderToolButton = new (std::nothrow) DropdownToolButton(_ORDER_AG->actions(), QToolButton::InstantPopup, Qt::ToolButtonStyle::ToolButtonTextBesideIcon, IMAGE_SIZE::TABS_ICON_IN_MENU_24);
  orderToolButton->SetCaption(QIcon{":img/SORTING_FILE_FOLDER"}, "Sort");
  auto* orderTB = new (std::nothrow) QToolBar{"Scene Order"};
  orderTB->addWidget(orderToolButton);
  orderTB->addAction(_REVERSE_SORT);
  orderTB->setOrientation(Qt::Orientation::Vertical);
  orderTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  return orderTB;
}

QToolBar* SceneInPageActions::GetPagesRowByColumnToolBar() {
  int sceneCnt1Page = PreferenceSettings().value("SCENES_COUNT_EACH_PAGE", 0).toInt();
  mPageDimensionLE = new (std::nothrow) QLineEdit(QString::number(sceneCnt1Page));
  mPageDimensionLE->setAlignment(Qt::AlignmentFlag::AlignHCenter);
  mPageDimensionLE->setToolTip("Scenes count each page");
  mPageDimensionLE->setMaximumHeight(IMAGE_SIZE::TABS_ICON_IN_MENU_24);
  mPageDimensionLE->setEnabled(false);

  mPageIndexInputLE = new (std::nothrow) QLineEdit("0");
  mPageIndexInputLE->setValidator(new QIntValidator{-1, 10000});
  mPageIndexInputLE->setAlignment(Qt::AlignmentFlag::AlignHCenter);
  mPageIndexInputLE->setToolTip("Page index");
  mPageIndexInputLE->setMaximumHeight(IMAGE_SIZE::TABS_ICON_IN_MENU_16);
  mPageIndexInputLE->setEnabled(false);

  mPagesSelectTB = new (std::nothrow) QToolBar("Page Select");
  mPagesSelectTB->addActions({_THE_FIRST_PAGE, _LAST_PAGE});
  mPagesSelectTB->addSeparator();
  mPagesSelectTB->addWidget(mPageIndexInputLE);
  mPagesSelectTB->addSeparator();
  mPagesSelectTB->addActions({_NEXT_PAGE, _THE_LAST_PAGE});
  mPagesSelectTB->setStyleSheet("QToolBar { max-width: 512px; }");
  mPagesSelectTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  mPagesSelectTB->setEnabled(false);

  QToolBar* enableRowColTB = new (std::nothrow) QToolBar{"Pagination display"};
  enableRowColTB->addAction(_GROUP_BY_PAGE);
  enableRowColTB->addWidget(mPageDimensionLE);
  enableRowColTB->addWidget(mPagesSelectTB);
  enableRowColTB->setOrientation(Qt::Orientation::Vertical);
  return enableRowColTB;
}

void onImageSizeChanged(const QString& newImageSize) {
  int xIndex = newImageSize.indexOf('x');
  if (xIndex == -1) {
    return;
  }
  const QString wid = newImageSize.left(xIndex);
  bool isValueValid = false;
  int w = wid.toInt(&isValueValid);
  if (!isValueValid) {
    qWarning("width str[%s] invalid not a number", qPrintable(wid));
    return;
  }
  const QString height = newImageSize.mid(xIndex + 1);
  int h = height.toInt(&isValueValid);
  if (!isValueValid) {
    qWarning("height str[%s] invalid not a number", qPrintable(height));
    return;
  }
  qDebug("Image size changed to %dx%d pixels", w, h);
  PreferenceSettings().setValue("FLOATING_WINDOW_IMG_WIDTH", w);
  PreferenceSettings().setValue("FLOATING_WINDOW_IMG_HEIGHT", h);
  IMAGE_SIZE::IMG_WIDTH = w;
  IMAGE_SIZE::IMG_HEIGHT = h;
}

QToolBar* SceneInPageActions::GetImageSizeToolBar() {
  const int width = PreferenceSettings().value("FLOATING_WINDOW_IMG_WIDTH", 480).toInt();
  const int height = PreferenceSettings().value("FLOATING_WINDOW_IMG_HEIGHT", 280).toInt();
  IMAGE_SIZE::IMG_WIDTH = width;
  IMAGE_SIZE::IMG_HEIGHT = height;
  const QString& imageSize = QString::number(width) + "x" + QString::number(height);

  mImageSize = new (std::nothrow) QLineEdit(imageSize);
  mImageSize->setInputMask("0000x0000");
  mImageSize->setToolTip("Image Size Toolbar");
  mImageSize->setAlignment(Qt::AlignmentFlag::AlignHCenter);
  connect(mImageSize, &QLineEdit::textChanged, &onImageSizeChanged);

  QToolBar* imageSizeTB = new (std::nothrow) QToolBar{"Image size toolbar"};
  imageSizeTB->addWidget(new (std::nothrow) QLabel{"Image width-by-height"});
  imageSizeTB->addWidget(mImageSize);
  imageSizeTB->setStyleSheet("QToolBar { max-width: 512px; }");
  imageSizeTB->setOrientation(Qt::Orientation::Vertical);
  return imageSizeTB;
}
