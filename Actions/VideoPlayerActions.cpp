#include "VideoPlayerActions.h"
#include "PublicVariable.h"

VideoPlayerActions::VideoPlayerActions(QObject* parent)
    : QObject{parent},
      _UPDATE_ITEM_PLAYABLE(new QAction(QIcon(":/themes/REFRESH_THIS_PATH"), "Update", this)),
      _MOVE_SELECTED_ITEMS_TO_TRASHBIN(new QAction(QIcon(":/themes/MOVE_TO_TRASH_BIN"), "Trashbin", this)),
      _SCROLL_TO_NEXT_FOLDER(new QAction(QIcon(":/themes/SCROLL_TO_NEXT_VIDEO_FOLDER"), "Nxt folder", this)),
      _SCROLL_TO_LAST_FOLDER(new QAction(QIcon(":/themes/SCROLL_TO_LAST_VIDEO_FOLDER"), "Lst folder", this)),
      _JUMP_LAST_HOT_SCENE(new QAction(QIcon(":/themes/JUMP_LAST_HOT_SCENE"), "last hot scene", this)),
      _JUMP_NEXT_HOT_SCENE(new QAction(QIcon(":/themes/JUMP_NEXT_HOT_SCENE"), "next hot scene", this)),
      _LAST_10_SECONDS(new QAction("-", this)),
      _NEXT_10_SECONDS(new QAction("+", this)),
      _AUTO_PLAY_NEXT_VIDEO(new QAction("auto", this)),
      _PLAY_PAUSE(new QAction(QIcon(":/themes/PLAY_VIDEO"), "play/pause", this)),
      _LAST_VIDEO(new QAction(QIcon(":/themes/LAST_VIDEO"), "last video", this)),
      _NEXT_VIDEO(new QAction(QIcon(":/themes/NEXT_VIDEO"), "next video", this)),
      _OPEN_A_VIDEO(new QAction(QIcon(":/themes/OPEN_A_VIDEO"), "open a video", this)),
      _LOAD_A_PATH(new QAction(QIcon(":/themes/OPEN_A_FOLDER"), "load a path", this)),
      _CLEAR_VIDEOS_LIST(new QAction(QIcon(":/themes/EMPTY_LISTWIDGET"), "clear playlist", this)),
      _PLAY_CURRENT_PATH(new QAction(QIcon(":/themes/OPEN_A_FOLDER"), "Play current path", this)),
      _PLAY_SELECTION(new QAction("Play selection", this)),
      _BATCH_VIDEO_ACTIONS(new QActionGroup(this)),
      _VIDEOS_LIST_MENU(new QAction(QIcon(":/themes/VIDEOS_LIST_MENU"), "show playlist", this)),
      _MARK_HOT_SCENE(new QAction(QIcon(":/themes/MARK_HOT_SCENE_POSITION"), "mark", this)),
      _GRAB_FRAME(new QAction(QIcon(":/themes/GRAB_FRAME"), "grab", this)),
      _RENAME_VIDEO(new QAction(QIcon(":/themes/RENAME_VIDEO"), "rename", this)),
      _MOD_PERFORMERS(new QAction(QIcon(":/themes/RENAME_PERFORMERS"), "mod performers", this)),
      _RATE_AG(GetRateActionGroups()),
      _RATE_LEVEL_COUNT(_RATE_AG->actions().size()),
      _REVEAL_IN_EXPLORER(new QAction(QIcon(), "reveal in explorer", this)) {
  _UPDATE_ITEM_PLAYABLE->setShortcut(QKeySequence(Qt::Key_F5));
  _UPDATE_ITEM_PLAYABLE->setShortcutVisibleInContextMenu(true);
  _UPDATE_ITEM_PLAYABLE->setToolTip(
      QString("<b>%1 (%2)</b><br/> Update playable item list").arg(_UPDATE_ITEM_PLAYABLE->text(), _UPDATE_ITEM_PLAYABLE->shortcut().toString()));

  _MOVE_SELECTED_ITEMS_TO_TRASHBIN->setShortcut(QKeySequence(Qt::Key_Delete));
  _MOVE_SELECTED_ITEMS_TO_TRASHBIN->setShortcutVisibleInContextMenu(true);
  _MOVE_SELECTED_ITEMS_TO_TRASHBIN->setToolTip(
      QString("<b>%1 (%2)</b><br/> Move selected items to trashbin")
          .arg(_MOVE_SELECTED_ITEMS_TO_TRASHBIN->text(), _MOVE_SELECTED_ITEMS_TO_TRASHBIN->shortcut().toString()));

  _SCROLL_TO_NEXT_FOLDER->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_Period));
  _SCROLL_TO_NEXT_FOLDER->setShortcutVisibleInContextMenu(true);
  _SCROLL_TO_NEXT_FOLDER->setToolTip(QString("<b>%1 (%2)</b><br/> Scroll to next folder first item and play it")
                                         .arg(_SCROLL_TO_NEXT_FOLDER->text(), _SCROLL_TO_NEXT_FOLDER->shortcut().toString()));

  _SCROLL_TO_LAST_FOLDER->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_Comma));
  _SCROLL_TO_LAST_FOLDER->setShortcutVisibleInContextMenu(true);
  _SCROLL_TO_LAST_FOLDER->setToolTip(QString("<b>%1 (%2)</b><br/> Scroll to last folder last item and play it")
                                         .arg(_SCROLL_TO_LAST_FOLDER->text(), _SCROLL_TO_LAST_FOLDER->shortcut().toString()));

  _LAST_10_SECONDS->setShortcut(QKeySequence(Qt::Key_Left));
  _LAST_10_SECONDS->setShortcutVisibleInContextMenu(true);
  _LAST_10_SECONDS->setToolTip(QString("<b>%1 (%2)</b><br/> -10s").arg(_LAST_10_SECONDS->text(), _LAST_10_SECONDS->shortcut().toString()));

  _NEXT_10_SECONDS->setShortcut(QKeySequence(Qt::Key_Right));
  _NEXT_10_SECONDS->setShortcutVisibleInContextMenu(true);
  _NEXT_10_SECONDS->setToolTip(QString("<b>%1 (%2)</b><br/> +10s").arg(_NEXT_10_SECONDS->text(), _NEXT_10_SECONDS->shortcut().toString()));

  _AUTO_PLAY_NEXT_VIDEO->setCheckable(true);
  _AUTO_PLAY_NEXT_VIDEO->setChecked(PreferenceSettings().value(MemoryKey::AUTO_PLAY_NEXT_VIDEO.name, MemoryKey::AUTO_PLAY_NEXT_VIDEO.v).toBool());

  _PLAY_PAUSE->setEnabled(false);
  _PLAY_PAUSE->setShortcutVisibleInContextMenu(true);
  _PLAY_PAUSE->setShortcut(QKeySequence(Qt::Key_Space));
  _PLAY_PAUSE->setToolTip(QString("<b>%1 (%2)</b><br/> Switch between pause/play").arg(_PLAY_PAUSE->text(), _PLAY_PAUSE->shortcut().toString()));

  _LOAD_A_PATH->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_O));
  _LOAD_A_PATH->setShortcutVisibleInContextMenu(true);
  _LOAD_A_PATH->setToolTip(QString("<b>%1 (%2)</b><br/> Load videos from a path").arg(_LOAD_A_PATH->text(), _LOAD_A_PATH->shortcut().toString()));

  _VIDEOS_LIST_MENU->setCheckable(true);
  _VIDEOS_LIST_MENU->setChecked(true);

  _MARK_HOT_SCENE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_P));
  _MARK_HOT_SCENE->setShortcutVisibleInContextMenu(true);
  _MARK_HOT_SCENE->setToolTip(QString("<b>%1 (%2)</b><br/> Mark this position").arg(_MARK_HOT_SCENE->text(), _MARK_HOT_SCENE->shortcut().toString()));

  _GRAB_FRAME->setShortcut(QKeySequence(Qt::AltModifier | Qt::Key_N));
  _GRAB_FRAME->setShortcutVisibleInContextMenu(true);
  _GRAB_FRAME->setToolTip(
      QString("<b>%1 (%2)</b><br/> Grab one frame and output as an image").arg(_GRAB_FRAME->text(), _GRAB_FRAME->shortcut().toString()));

  _RENAME_VIDEO->setShortcut(QKeySequence(Qt::Key_F2));
  _RENAME_VIDEO->setShortcutVisibleInContextMenu(true);
  _RENAME_VIDEO->setToolTip(
      QString("<b>%1 (%2)</b><br/> Rename both video and json file name if exists").arg(_RENAME_VIDEO->text(), _RENAME_VIDEO->shortcut().toString()));

  _MOD_PERFORMERS->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_F2));
  _MOD_PERFORMERS->setShortcutVisibleInContextMenu(true);
  _MOD_PERFORMERS->setToolTip(
      QString("<b>%1 (%2)</b><br/> Mod performers in json file").arg(_MOD_PERFORMERS->text(), _MOD_PERFORMERS->shortcut().toString()));

  _REVEAL_IN_EXPLORER->setShortcutVisibleInContextMenu(true);
  _REVEAL_IN_EXPLORER->setToolTip(
      QString("<b>%1 (%2)</b><br/> Reveal file in explorer").arg(_REVEAL_IN_EXPLORER->text(), _REVEAL_IN_EXPLORER->shortcut().toString()));

  _BATCH_VIDEO_ACTIONS->addAction(_PLAY_CURRENT_PATH);
  _BATCH_VIDEO_ACTIONS->addAction(_PLAY_SELECTION);
}

QActionGroup* VideoPlayerActions::GetRateActionGroups() {
  QActionGroup* rateAg = new QActionGroup(this);
  for (int i = 0; i != 10; ++i) {
    auto* rateI = new QAction(QString::number(i), this);
    rateI->setCheckable(true);
    rateAg->addAction(rateI);
  }
  rateAg->setExclusionPolicy(QActionGroup::ExclusionPolicy::ExclusiveOptional);
  return rateAg;
}

VideoPlayerActions& g_videoPlayerActions() {
  static VideoPlayerActions ins;
  return ins;
}
