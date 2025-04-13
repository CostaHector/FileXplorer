#include "ViewActions.h"
#include "public/MemoryKey.h"
#include "public/PublicVariable.h"
ViewActions& g_viewActions() {
  static ViewActions ins;
  return ins;
}

ViewActions::ViewActions(QObject* parent)
    : QObject{parent},
      _ADVANCE_SEARCH_VIEW{new QAction(QIcon(":img/SEARCH"), "search")},
      _MOVIE_VIEW{new QAction(QIcon(":img/SHOW_DATABASE"), "movie")},
      _LIST_VIEW{new QAction(QIcon(":img/DISPLAY_LARGE_THUMBNAILS"), "list")},
      _TABLE_VIEW{new QAction(QIcon(":img/DISPLAY_DETAIL_INFOMATIONS"), "table")},
      _TREE_VIEW{new QAction(QIcon(":img/DISPLAY_TREE_VIEW"), "tree")},
      _SCENE_VIEW{new QAction(QIcon(":img/SCENE_TABLE_VIEW"), "scene")},
      _FLOATING_PREVIEW{new QAction(QIcon(":img/FLOATING_PREVIEW"), "floating preview")},
      _VIEWS_AG{GetViewsAG()},

      NAVIGATION_PANE{new QAction(QIcon(":img/NAVIGATION_PANE"), tr("Navigation Pane"))},
      _JSON_EDITOR_PANE{new QAction(QIcon(":img/JSON_EDITOR"), tr("Json Editor"))},
      _VIDEO_PLAYER_EMBEDDED{new QAction(QIcon(":img/VIDEO_PLAYER"), tr("Embedded Player"))},
      _VIEW_ACTIONS(Get_NAVIGATION_PANE_Actions()),
      _SYS_VIDEO_PLAYERS(new QAction(QIcon(":img/PLAY_BUTTON_TRIANGLE"), tr("Play"))),
      _VIDEO_PLAYERS(GetPlayersActions()) {
  _HAR_VIEW = new QAction{QIcon(":img/HAR_VIEW"), "Har View"};
}

QActionGroup* ViewActions::Get_NAVIGATION_PANE_Actions() {
  NAVIGATION_PANE->setToolTip(QString("<b>%1 (%2)</b><br/> Show or hide the navigation pane.").arg(NAVIGATION_PANE->text(), NAVIGATION_PANE->shortcut().toString()));
  NAVIGATION_PANE->setCheckable(true);

  _JSON_EDITOR_PANE->setCheckable(true);
  _JSON_EDITOR_PANE->setToolTip(QString("<b>%1 (%2)</b><br/>Show Json Edit Pane.").arg(_JSON_EDITOR_PANE->text(), _JSON_EDITOR_PANE->shortcut().toString()));

  _VIDEO_PLAYER_EMBEDDED->setShortcutVisibleInContextMenu(true);
  _VIDEO_PLAYER_EMBEDDED->setToolTip(
      QString("<b>%1 (%2)</b><br/> Open the selected item in embedded video player.").arg(_VIDEO_PLAYER_EMBEDDED->text(), _VIDEO_PLAYER_EMBEDDED->shortcut().toString()));

  auto* actionGroup = new QActionGroup(this);
  actionGroup->addAction(NAVIGATION_PANE);
  actionGroup->addAction(_JSON_EDITOR_PANE);
  actionGroup->addAction(_VIDEO_PLAYER_EMBEDDED);
  actionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  NAVIGATION_PANE->setChecked(PreferenceSettings().value(MemoryKey::SHOW_QUICK_NAVIGATION_TOOL_BAR.name).toBool());
  return actionGroup;
}

QActionGroup* ViewActions::GetPlayersActions() {
  _SYS_VIDEO_PLAYERS->setShortcut(QKeySequence(Qt::ShiftModifier | Qt::Key_Return));
  _SYS_VIDEO_PLAYERS->setShortcutVisibleInContextMenu(true);
  _SYS_VIDEO_PLAYERS->setToolTip(QString("<b>%1 (%2)</b><br/>"
                                         "Play the selected item(s) in default system player.")
                                     .arg(_SYS_VIDEO_PLAYERS->text(), _SYS_VIDEO_PLAYERS->shortcut().toString()));
  QActionGroup* actionGroup = new QActionGroup(this);
  actionGroup->addAction(_SYS_VIDEO_PLAYERS);
  actionGroup->addAction(_VIDEO_PLAYER_EMBEDDED);
  actionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  for (QAction* act : actionGroup->actions()) {
    act->setCheckable(false);
  }
  return actionGroup;
}

QActionGroup* ViewActions::GetViewsAG() {
  _LIST_VIEW->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_2));
  _LIST_VIEW->setShortcutVisibleInContextMenu(true);
  _LIST_VIEW->setToolTip(QString("Displays items by using large thumbnails. (%1)").arg(_LIST_VIEW->shortcut().toString()));
  _LIST_VIEW->setCheckable(true);

  _TABLE_VIEW->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_6));
  _TABLE_VIEW->setShortcutVisibleInContextMenu(true);
  _TABLE_VIEW->setToolTip(QString("Displays information about each in the window. (%1)").arg(_TABLE_VIEW->shortcut().toString()));
  _TABLE_VIEW->setCheckable(true);
  _TABLE_VIEW->setChecked(true);

  _TREE_VIEW->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_9));
  _TREE_VIEW->setShortcutVisibleInContextMenu(true);
  _TREE_VIEW->setToolTip(QString("Display files and folders achitecures. (%1)").arg(_TREE_VIEW->shortcut().toString()));
  _TREE_VIEW->setCheckable(true);

  _MOVIE_VIEW->setToolTip(QString("Movie dictionary view aka Database view. (%1)").arg(_MOVIE_VIEW->shortcut().toString()));
  _MOVIE_VIEW->setCheckable(true);

  _ADVANCE_SEARCH_VIEW->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_F));
  _ADVANCE_SEARCH_VIEW->setShortcutVisibleInContextMenu(true);
  _ADVANCE_SEARCH_VIEW->setToolTip(QString("Show advanced search window. (%1)").arg(_ADVANCE_SEARCH_VIEW->shortcut().toString()));
  _ADVANCE_SEARCH_VIEW->setCheckable(true);

  _SCENE_VIEW->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_8));
  _SCENE_VIEW->setShortcutVisibleInContextMenu(true);
  _SCENE_VIEW->setToolTip(QString("Show video scenes in page table. (%1)").arg(_SCENE_VIEW->shortcut().toString()));
  _SCENE_VIEW->setCheckable(true);

  _FLOATING_PREVIEW->setToolTip(QString("Show floating preview for a scene in scene view. (%1)").arg(_FLOATING_PREVIEW->shortcut().toString()));
  _FLOATING_PREVIEW->setCheckable(true);
  _FLOATING_PREVIEW->setChecked(PreferenceSettings().value(MemoryKey::SHOW_FLOATING_PREVIEW.name, MemoryKey::SHOW_FLOATING_PREVIEW.v).toBool());

  QActionGroup* actionGroup = new QActionGroup(this);
  actionGroup->addAction(_LIST_VIEW);
  actionGroup->addAction(_TABLE_VIEW);
  actionGroup->addAction(_TREE_VIEW);
  actionGroup->addAction(_MOVIE_VIEW);
  actionGroup->addAction(_ADVANCE_SEARCH_VIEW);
  actionGroup->addAction(_SCENE_VIEW);
  actionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);
  return actionGroup;
}
