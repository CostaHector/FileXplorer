#include "SceneInPageActions.h"
#include "MenuToolButton.h"
#include "MemoryKey.h"
#include "StyleSheet.h"
#include <QToolBar>

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

  _BY_MOVIE_NAME = new (std::nothrow) QAction(QIcon(":img/SORTING_FILE_FOLDER"), "Movie Name", this);
  CHECK_NULLPTR_RETURN_VOID(_BY_MOVIE_NAME);
  _BY_MOVIE_NAME->setCheckable(true);

  _BY_MOVIE_SIZE = new (std::nothrow) QAction("Movie Size", this);
  CHECK_NULLPTR_RETURN_VOID(_BY_MOVIE_SIZE);
  _BY_MOVIE_SIZE->setCheckable(true);

  _BY_RATE = new (std::nothrow) QAction("Rate", this);
  CHECK_NULLPTR_RETURN_VOID(_BY_RATE);
  _BY_RATE->setCheckable(true);

  _BY_UPLOADED_TIME = new (std::nothrow) QAction("Uploaded Time", this);
  CHECK_NULLPTR_RETURN_VOID(_BY_UPLOADED_TIME);
  _BY_UPLOADED_TIME->setCheckable(true);

  _REVERSE_SORT = new (std::nothrow) QAction("Reverse", this);
  CHECK_NULLPTR_RETURN_VOID(_REVERSE_SORT);
  _REVERSE_SORT->setCheckable(true);

  _ORDER_AG = new (std::nothrow) QActionGroup(this);
  CHECK_NULLPTR_RETURN_VOID(_ORDER_AG);
  _ORDER_AG->addAction(_BY_MOVIE_NAME);
  _ORDER_AG->addAction(_BY_MOVIE_SIZE);
  _ORDER_AG->addAction(_BY_RATE);
  _ORDER_AG->addAction(_BY_UPLOADED_TIME);
  _ORDER_AG->setExclusionPolicy(QActionGroup::ExclusionPolicy::ExclusiveOptional);
}

QToolBar* SceneInPageActions::GetOrderToolBar(QWidget* parent) {
  auto* orderToolButton = new (std::nothrow) MenuToolButton(_ORDER_AG->actions(),                           //
                                                            QToolButton::InstantPopup,                      //
                                                            Qt::ToolButtonStyle::ToolButtonTextBesideIcon,  //
                                                            IMAGE_SIZE::TABS_ICON_IN_MENU_48,               //
                                                            parent);
  CHECK_NULLPTR_RETURN_NULLPTR(parent);
  orderToolButton->SetCaption(QIcon{":img/SORTING_FILE_FOLDER"}, "Sort");

  auto* orderTB = new (std::nothrow) QToolBar{"Scene Order", parent};
  CHECK_NULLPTR_RETURN_NULLPTR(orderTB);

  orderTB->addWidget(orderToolButton);
  orderTB->addAction(_REVERSE_SORT);
  orderTB->setOrientation(Qt::Orientation::Vertical);
  orderTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);

  SetLayoutAlightment(orderTB->layout(), Qt::AlignmentFlag::AlignLeft);
  return orderTB;
}
