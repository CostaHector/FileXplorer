#include "ViewActions.h"
#include "MemoryKey.h"
#include "PublicMacro.h"
#include "PublicTool.h"
#include "PublicVariable.h"
#include "StyleSheet.h"
#include "ViewTypeTool.h"

ViewActions& g_viewActions() {
  static ViewActions ins;
  return ins;
}

ViewActions::ViewActions(QObject* parent) : QObject{parent} {
  using namespace ViewTypeTool;

  _ADVANCE_SEARCH_VIEW = new (std::nothrow) QAction(QIcon(":img/SEARCH"), ENUM_2_STR(SEARCH));
  _MOVIE_VIEW = new (std::nothrow) QAction(QIcon(":img/MOVIES_VIEW"), ENUM_2_STR(MOVIE));
  _LIST_VIEW = new (std::nothrow) QAction(QIcon(":img/DISPLAY_LARGE_THUMBNAILS"), ENUM_2_STR(LIST));
  _TABLE_VIEW = new (std::nothrow) QAction(QIcon(":img/DISPLAY_DETAIL_INFOMATIONS"), ENUM_2_STR(TABLE));
  _TREE_VIEW = new (std::nothrow) QAction(QIcon(":img/DISPLAY_TREE_VIEW"), ENUM_2_STR(TREE));
  _SCENE_VIEW = new (std::nothrow) QAction(QIcon(":img/SCENES_VIEW"), ENUM_2_STR(SCENE));
  _CAST_VIEW = new (std::nothrow) QAction(QIcon(":img/CAST_VIEW"), ENUM_2_STR(CAST));
  _JSON_VIEW = new (std::nothrow) QAction(QIcon(":img/JSON_EDITOR"), ENUM_2_STR(JSON));
  _VIEWS_AG = GetViewsAG();

  NAVIGATION_PANE = new (std::nothrow) QAction(QIcon(":img/NAVIGATION_PANE"), tr("Navigation Pane"));
  _VIEW_ACTIONS = Get_NAVIGATION_PANE_Actions();
  _SYS_VIDEO_PLAYERS = new (std::nothrow) QAction(QIcon(":img/PLAY_BUTTON_ROUND"), tr("Play"));
  _SYS_VIDEO_PLAYERS->setShortcut(QKeySequence(Qt::ShiftModifier | Qt::Key_Return));
  _SYS_VIDEO_PLAYERS->setShortcutVisibleInContextMenu(true);
  _SYS_VIDEO_PLAYERS->setToolTip(QString("<b>%1 (%2)</b><br/>"
                                         "Play the selected item(s) in default system player.")
                                     .arg(_SYS_VIDEO_PLAYERS->text(), _SYS_VIDEO_PLAYERS->shortcut().toString()));

  _HAR_VIEW = new (std::nothrow) QAction{QIcon(":img/HAR_VIEW"), "Har View"};
}

QActionGroup* ViewActions::Get_NAVIGATION_PANE_Actions() {
  NAVIGATION_PANE->setToolTip(QString("<b>%1 (%2)</b><br/> Show or hide the navigation pane.").arg(NAVIGATION_PANE->text(), NAVIGATION_PANE->shortcut().toString()));
  NAVIGATION_PANE->setCheckable(true);

  auto* actionGroup = new (std::nothrow) QActionGroup(this);
  CHECK_NULLPTR_RETURN_NULLPTR(actionGroup);
  actionGroup->addAction(NAVIGATION_PANE);
  actionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  NAVIGATION_PANE->setChecked(Configuration().value(MemoryKey::SHOW_QUICK_NAVIGATION_TOOL_BAR.name).toBool());
  return actionGroup;
}

QActionGroup* ViewActions::GetViewsAG() {
  _LIST_VIEW->setShortcutVisibleInContextMenu(true);
  _LIST_VIEW->setToolTip(QString("Displays items by using large thumbnails. (%1)").arg(_LIST_VIEW->shortcut().toString()));
  _LIST_VIEW->setCheckable(true);

  _TABLE_VIEW->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_6));
  _TABLE_VIEW->setShortcutVisibleInContextMenu(true);
  _TABLE_VIEW->setToolTip(QString("Displays information about each in the window. (%1)").arg(_TABLE_VIEW->shortcut().toString()));
  _TABLE_VIEW->setCheckable(true);
  _TABLE_VIEW->setChecked(true);

  _TREE_VIEW->setShortcutVisibleInContextMenu(true);
  _TREE_VIEW->setToolTip(QString("Display files and folders achitecures. (%1)").arg(_TREE_VIEW->shortcut().toString()));
  _TREE_VIEW->setCheckable(true);

  _MOVIE_VIEW->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_7));
  _MOVIE_VIEW->setShortcutVisibleInContextMenu(true);
  _MOVIE_VIEW->setToolTip(QString("Movie dictionary view aka Database view. (%1)").arg(_MOVIE_VIEW->shortcut().toString()));
  _MOVIE_VIEW->setCheckable(true);

  _CAST_VIEW->setShortcutVisibleInContextMenu(true);
  _CAST_VIEW->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_8));
  _CAST_VIEW->setToolTip(QString("Show Cast database. (%1)").arg(_CAST_VIEW->shortcut().toString()));
  _CAST_VIEW->setCheckable(true);

  _SCENE_VIEW->setShortcutVisibleInContextMenu(true);
  _SCENE_VIEW->setToolTip(QString("Show video scenes in page table. (%1)").arg(_SCENE_VIEW->shortcut().toString()));
  _SCENE_VIEW->setCheckable(true);

  _JSON_VIEW->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_9));
  _JSON_VIEW->setShortcutVisibleInContextMenu(true);
  _JSON_VIEW->setToolTip(QString("Show Json editor tableview. (%1)").arg(_JSON_VIEW->shortcut().toString()));
  _JSON_VIEW->setCheckable(true);

  _ADVANCE_SEARCH_VIEW->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_F));
  _ADVANCE_SEARCH_VIEW->setShortcutVisibleInContextMenu(true);
  _ADVANCE_SEARCH_VIEW->setToolTip(QString("Show advanced search window. (%1)").arg(_ADVANCE_SEARCH_VIEW->shortcut().toString()));
  _ADVANCE_SEARCH_VIEW->setCheckable(true);

  QActionGroup* actionGroup = new (std::nothrow) QActionGroup(this);
  CHECK_NULLPTR_RETURN_NULLPTR(actionGroup);
  actionGroup->addAction(_LIST_VIEW);
  actionGroup->addAction(_TABLE_VIEW);
  actionGroup->addAction(_TREE_VIEW);
  actionGroup->addAction(_MOVIE_VIEW);
  actionGroup->addAction(_CAST_VIEW);
  actionGroup->addAction(_SCENE_VIEW);
  actionGroup->addAction(_JSON_VIEW);
  actionGroup->addAction(_ADVANCE_SEARCH_VIEW);
  actionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);
  return actionGroup;
}

QToolBar* ViewActions::GetFileSystemViewTB(QWidget* parent) {
  auto* fileSystemViewTb = new (std::nothrow) QToolBar("Navigation Preview Switch", parent);
  CHECK_NULLPTR_RETURN_NULLPTR(fileSystemViewTb);
  fileSystemViewTb->setOrientation(Qt::Orientation::Vertical);
  fileSystemViewTb->addAction(_LIST_VIEW);
  fileSystemViewTb->addAction(_TABLE_VIEW);
  fileSystemViewTb->addAction(_TREE_VIEW);
  fileSystemViewTb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  fileSystemViewTb->setStyleSheet("QToolBar { max-width: 256px; }");
  fileSystemViewTb->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  SetLayoutAlightment(fileSystemViewTb->layout(), Qt::AlignmentFlag::AlignLeft);
  return fileSystemViewTb;
}

QToolBar* ViewActions::GetViewTB(QWidget* parent) {
  auto* pTb = new (std::nothrow) QToolBar{"views switch", parent};
  CHECK_NULLPTR_RETURN_NULLPTR(pTb);
  pTb->addAction(_TABLE_VIEW);
  pTb->addAction(_MOVIE_VIEW);
  pTb->addAction(_CAST_VIEW);
  pTb->addAction(_SCENE_VIEW);
  pTb->addAction(_JSON_VIEW);
  pTb->addAction(_ADVANCE_SEARCH_VIEW);
  pTb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
  pTb->setOrientation(Qt::Orientation::Horizontal);
  pTb->setStyleSheet("QToolBar { max-width: 256px; }");
  pTb->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  return pTb;
}
