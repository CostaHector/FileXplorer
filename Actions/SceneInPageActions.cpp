#include "SceneInPageActions.h"
#include <QLineEdit>
#include <QToolBar>
#include <QLabel>
#include <QIntValidator>
#include "Component/DropListToolButton.h"

constexpr int SceneInPageActions::ROW_COLUMN_LINEDIT_MAX_WIDTH;

SceneInPageActions& g_SceneInPageActions() {
  static SceneInPageActions ins;
  return ins;
}

SceneInPageActions::SceneInPageActions(QObject* parent) : QObject{parent} {
  _COMBINE_MEDIAINFOS_JSON = new QAction(QIcon(":/themes/COMBINE_MEDIAS_INFO"), "Update infos", this);
  _COMBINE_MEDIAINFOS_JSON->setShortcut(QKeySequence(Qt::Key_F5));
  _COMBINE_MEDIAINFOS_JSON->setShortcutVisibleInContextMenu(true);
  _COMBINE_MEDIAINFOS_JSON->setToolTip(QString("<b>%1 (%2)</b><br/> Combine Videos/Images infos Into json files. Then generate scn file from valid "
                                               "json file(s). This operation may update json file contents")
                                           .arg(_COMBINE_MEDIAINFOS_JSON->text(), _COMBINE_MEDIAINFOS_JSON->shortcut().toString()));

  _UPDATE_SCN_ONLY = new QAction(QIcon(), "Update Scn", this);
  _UPDATE_SCN_ONLY->setShortcutVisibleInContextMenu(true);
  _UPDATE_SCN_ONLY->setToolTip(QString("<b>%1 (%2)</b><br/> Only update scn file from valid json file(s).")
                                           .arg(_UPDATE_SCN_ONLY->text(), _UPDATE_SCN_ONLY->shortcut().toString()));

  _BY_MOVIE_NAME = new QAction(QIcon(":/themes/SORTING_FILE_FOLDER"), "Movie Name", this);
  _BY_MOVIE_NAME->setCheckable(true);
  _BY_MOVIE_SIZE = new QAction("Movie Size", this);
  _BY_MOVIE_SIZE->setCheckable(true);
  _BY_RATE = new QAction("Rate", this);
  _BY_RATE->setCheckable(true);
  _BY_UPLOADED_TIME = new QAction("Uploaded Time", this);
  _BY_UPLOADED_TIME->setCheckable(true);

  _REVERSE_SORT = new QAction("Reverse", this);
  _REVERSE_SORT->setCheckable(true);

  _ORDER_AG = new QActionGroup(this);
  _ORDER_AG->addAction(_BY_MOVIE_NAME);
  _ORDER_AG->addAction(_BY_MOVIE_SIZE);
  _ORDER_AG->addAction(_BY_RATE);
  _ORDER_AG->addAction(_BY_UPLOADED_TIME);
  _ORDER_AG->setExclusionPolicy(QActionGroup::ExclusionPolicy::ExclusiveOptional);

  _GROUP_BY_PAGE = new QAction("Enable Group by page:\nrow x column", this);
  _GROUP_BY_PAGE->setCheckable(true);
  _GROUP_BY_PAGE->setChecked(false);

  _THE_LAST_PAGE = new QAction("The Last>>", this);
  _LAST_PAGE = new QAction("Last<", this);
  _NEXT_PAGE = new QAction("Next>", this);
  _THE_FIRST_PAGE = new QAction("The First<<", this);
}

bool SceneInPageActions::InitWidget() {
  if (mOrderTB != nullptr || mEnablePageTB != nullptr || mPagesSelectTB != nullptr) {
    return true;
  }
  mOrderTB = GetOrderToolBar();
  mEnablePageTB = GetPagesRowByColumnToolBar();
  mPagesSelectTB = GetPageIndexSelectionToolBar();
  if (mOrderTB == nullptr || mEnablePageTB == nullptr || mPagesSelectTB == nullptr) {
    qWarning("Init Scene Order/Row-by-Column/PageIndex Failed");
    return false;
  }
  mPagesSelectTB->setEnabled(false);
  return true;
}

QToolBar* SceneInPageActions::GetSceneToolbar() {
  if (!InitWidget()) {
    return nullptr;
  }
  auto* sceneTB = new (std::nothrow) QToolBar("scene toolbar");
  sceneTB->addWidget(mOrderTB);
  sceneTB->addSeparator();
  sceneTB->addWidget(mEnablePageTB);
  sceneTB->addSeparator();
  sceneTB->addWidget(mPagesSelectTB);
  return sceneTB;
}

QToolBar* SceneInPageActions::GetOrderToolBar() {
  auto* defaultDisp = new QAction{QIcon(":/themes/SORTING_FILE_FOLDER"), "Sort"};
  defaultDisp->setToolTip("Choose sort option");
  auto* orderToolButton = DropListToolButton(defaultDisp, _ORDER_AG->actions(), QToolButton::InstantPopup, "Choose sort option",
                                             Qt::ToolButtonStyle::ToolButtonTextBesideIcon, TABS_ICON_IN_MENU_2x1);

  auto* orderTB = new (std::nothrow) QToolBar("Scene Order");
  orderTB->addWidget(orderToolButton);
  orderTB->addAction(_REVERSE_SORT);
  orderTB->setOrientation(Qt::Orientation::Vertical);
  orderTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  return orderTB;
}

QToolBar* SceneInPageActions::GetPagesRowByColumnToolBar() {
  mRowsInputLE = new (std::nothrow) QLineEdit("5");
  mColumnsInputLE = new (std::nothrow) QLineEdit("4");
  mRowsInputLE->setMaximumWidth(ROW_COLUMN_LINEDIT_MAX_WIDTH);
  mColumnsInputLE->setMaximumWidth(ROW_COLUMN_LINEDIT_MAX_WIDTH);
  mRowsInputLE->setValidator(new QIntValidator{1, 10000});
  mColumnsInputLE->setValidator(new QIntValidator{1, 10000});

  auto* rowByColumnTB = new (std::nothrow) QToolBar("Page Row-by-Column");
  rowByColumnTB->addWidget(mRowsInputLE);
  rowByColumnTB->addWidget(new QLabel("-by-"));
  rowByColumnTB->addWidget(mColumnsInputLE);

  auto* enableRowColTB = new (std::nothrow) QToolBar{"Enable Page"};
  enableRowColTB->addAction(_GROUP_BY_PAGE);
  enableRowColTB->addWidget(rowByColumnTB);
  enableRowColTB->setOrientation(Qt::Orientation::Vertical);
  return enableRowColTB;
}

QToolBar* SceneInPageActions::GetPageIndexSelectionToolBar() {
  mPageIndexInputLE = new (std::nothrow) QLineEdit("0");
  mPageIndexInputLE->setMaximumWidth(ROW_COLUMN_LINEDIT_MAX_WIDTH);
  mPageIndexInputLE->setValidator(new QIntValidator{-1, 10000});

  auto* pagesSelectTB = new (std::nothrow) QToolBar("Page Select");
  pagesSelectTB->addSeparator();
  pagesSelectTB->addActions({_THE_FIRST_PAGE, _LAST_PAGE});
  pagesSelectTB->addSeparator();
  pagesSelectTB->addWidget(mPageIndexInputLE);
  pagesSelectTB->addSeparator();
  pagesSelectTB->addActions({_NEXT_PAGE, _THE_LAST_PAGE});
  return pagesSelectTB;
}
