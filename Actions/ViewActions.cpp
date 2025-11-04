#include "ViewActions.h"
#include "MemoryKey.h"
#include "PublicMacro.h"
#include "PublicTool.h"
#include "PublicVariable.h"
#include "StyleSheet.h"
#include "ViewTypeTool.h"
#include "ViewSwitchToolBar.h"

ViewActions::ViewActions(QObject* parent) : QObject{parent} {
  using namespace ViewTypeTool;

  _LIST_VIEW = new (std::nothrow) QAction(QIcon(":img/DISPLAY_LARGE_THUMBNAILS"), ENUM_2_STR(LIST), this);
  _TABLE_VIEW = new (std::nothrow) QAction(QIcon(":img/DISPLAY_DETAIL_INFOMATIONS"), ENUM_2_STR(TABLE), this);
  _TREE_VIEW = new (std::nothrow) QAction(QIcon(":img/DISPLAY_TREE_VIEW"), ENUM_2_STR(TREE), this);
  _ADVANCE_SEARCH_VIEW = new (std::nothrow) QAction(QIcon(":img/SEARCH"), ENUM_2_STR(SEARCH), this);
  _MOVIE_VIEW = new (std::nothrow) QAction(QIcon(":img/MOVIES_VIEW"), ENUM_2_STR(MOVIE), this);
  _SCENE_VIEW = new (std::nothrow) QAction(QIcon(":img/SCENES_VIEW"), ENUM_2_STR(SCENE), this);
  _CAST_VIEW = new (std::nothrow) QAction(QIcon(":img/CAST_VIEW"), ENUM_2_STR(CAST), this);
  _JSON_VIEW = new (std::nothrow) QAction(QIcon(":img/JSON_EDITOR"), ENUM_2_STR(JSON), this);
  _ALL_VIEWS += _LIST_VIEW;
  _ALL_VIEWS += _TABLE_VIEW;
  _ALL_VIEWS += _TREE_VIEW;
  _ALL_VIEWS += _MOVIE_VIEW;
  _ALL_VIEWS += _CAST_VIEW;
  _ALL_VIEWS += _SCENE_VIEW;
  _ALL_VIEWS += _JSON_VIEW;
  _ALL_VIEWS += _ADVANCE_SEARCH_VIEW;

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

  _ADVANCE_SEARCH_VIEW->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_F));
  _ADVANCE_SEARCH_VIEW->setShortcutVisibleInContextMenu(true);
  _ADVANCE_SEARCH_VIEW->setToolTip(QString("Show advanced search window. (%1)").arg(_ADVANCE_SEARCH_VIEW->shortcut().toString()));
  _ADVANCE_SEARCH_VIEW->setCheckable(true);

  _MOVIE_VIEW->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_7));
  _MOVIE_VIEW->setShortcutVisibleInContextMenu(true);
  _MOVIE_VIEW->setToolTip(QString("Movie dictionary view aka Database view. (%1)").arg(_MOVIE_VIEW->shortcut().toString()));
  _MOVIE_VIEW->setCheckable(true);

  _CAST_VIEW->setShortcutVisibleInContextMenu(true);
  _CAST_VIEW->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_8));
  _CAST_VIEW->setToolTip(QString("Show Cast database. (%1)").arg(_CAST_VIEW->shortcut().toString()));
  _CAST_VIEW->setCheckable(true);

  _SCENE_VIEW->setShortcutVisibleInContextMenu(true);
  _SCENE_VIEW->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_9));
  _SCENE_VIEW->setToolTip(QString("Show video scenes in page table. (%1)").arg(_SCENE_VIEW->shortcut().toString()));
  _SCENE_VIEW->setCheckable(true);

  _JSON_VIEW->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_0));
  _JSON_VIEW->setShortcutVisibleInContextMenu(true);
  _JSON_VIEW->setToolTip(QString("Show Json editor tableview. (%1)").arg(_JSON_VIEW->shortcut().toString()));
  _JSON_VIEW->setCheckable(true);

  _VIEW_BACK_TO = new (std::nothrow) QAction(QIcon{":img/_VIEW_BACK_TO"}, "View back", this);
  _VIEW_BACK_TO->setToolTip(QString("<b>%1 (Ctrl+Mouse BackButton)</b><br/> back to last view type.").arg(_VIEW_BACK_TO->text()));
  _VIEW_FORWARD_TO = new (std::nothrow) QAction(QIcon{":img/_VIEW_FORWARD_TO"}, "View forward", this);
  _VIEW_FORWARD_TO->setToolTip(QString("<b>%1 (Ctrl+Mouse ForwardButton)</b><br/> forward to next view type.").arg(_VIEW_FORWARD_TO->text()));
  _VIEWS_NAVIGATE += _VIEW_BACK_TO;
  _VIEWS_NAVIGATE += _VIEW_FORWARD_TO;

  NAVIGATION_PANE = new (std::nothrow) QAction(QIcon(":img/NAVIGATION_PANE"), tr("Navigation Pane"), this);
  NAVIGATION_PANE->setToolTip(QString("<b>%1 (%2)</b><br/> Show or hide the navigation pane.").arg(NAVIGATION_PANE->text(), NAVIGATION_PANE->shortcut().toString()));
  NAVIGATION_PANE->setCheckable(true);
  NAVIGATION_PANE->setChecked(Configuration().value(MemoryKey::SHOW_QUICK_NAVIGATION_TOOL_BAR.name).toBool());

  _SYS_VIDEO_PLAYERS = new (std::nothrow) QAction(QIcon(":img/PLAY_BUTTON_ROUND"), tr("Play"), this);
  _SYS_VIDEO_PLAYERS->setShortcut(QKeySequence(Qt::ShiftModifier | Qt::Key_Return));
  _SYS_VIDEO_PLAYERS->setShortcutVisibleInContextMenu(true);
  _SYS_VIDEO_PLAYERS->setToolTip(QString("<b>%1 (%2)</b><br/>"
                                         "Play the selected item(s) in default system player.")
                                     .arg(_SYS_VIDEO_PLAYERS->text(), _SYS_VIDEO_PLAYERS->shortcut().toString()));

  _HAR_VIEW = new (std::nothrow) QAction{QIcon(":img/HAR_VIEW"), tr("Har View"), this};
  _HAR_VIEW->setCheckable(true);
  _HAR_VIEW->setChecked(false);
  _HAR_VIEW->setToolTip(QString("<b>%1 (%2)</b><br/>"
                                "Double click to open an har file")
                            .arg(_HAR_VIEW->text(), _HAR_VIEW->shortcut().toString()));
}

QToolBar* ViewActions::GetViewTB(QWidget* parent) {
  auto* pTb = new (std::nothrow) ViewSwitchToolBar{"views switch", parent};
  CHECK_NULLPTR_RETURN_NULLPTR(pTb);
  pTb->mViewTypeIntAction.init({{_LIST_VIEW, ViewTypeTool::ViewType::LIST},
                                {_TABLE_VIEW, ViewTypeTool::ViewType::TABLE},
                                {_TREE_VIEW, ViewTypeTool::ViewType::TREE},
                                {_MOVIE_VIEW, ViewTypeTool::ViewType::MOVIE},
                                {_CAST_VIEW, ViewTypeTool::ViewType::CAST},
                                {_SCENE_VIEW, ViewTypeTool::ViewType::SCENE},
                                {_JSON_VIEW, ViewTypeTool::ViewType::JSON},
                                {_ADVANCE_SEARCH_VIEW, ViewTypeTool::ViewType::SEARCH}},
                               ViewTypeTool::DEFAULT_VIEW_TYPE, QActionGroup::ExclusionPolicy::Exclusive);
  pTb->subscribe();
  pTb->addActions(_ALL_VIEWS); // action sorted in user specified sequence
  pTb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
  pTb->setOrientation(Qt::Orientation::Horizontal);
  pTb->setStyleSheet("QToolBar { max-width: 256px; }");
  pTb->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  return pTb;
}

ViewActions& g_viewActions() {
  static ViewActions ins;
  return ins;
}
