#include "VideoPlayerActions.h"

VideoPlayerActions::VideoPlayerActions(QObject* parent)
    : QObject{parent},
      _JUMP_LAST_HOT_SCENE(new QAction(QIcon(":/themes/JUMP_LAST_HOT_SCENE"), "last hot scene", this)),
      _JUMP_NEXT_HOT_SCENE(new QAction(QIcon(":/themes/JUMP_NEXT_HOT_SCENE"), "next hot scene", this)),
      _PLAY_PAUSE(new QAction(QIcon(":/themes/PLAY_VIDEO"), "play/pause", this)),
      _LAST_VIDEO(new QAction(QIcon(":/themes/LAST_VIDEO"), "last video", this)),
      _NEXT_VIDEO(new QAction(QIcon(":/themes/NEXT_VIDEO"), "next video", this)),
      _OPEN_A_VIDEO(new QAction(QIcon(":/themes/OPEN_A_VIDEO"), "open a video", this)),
      _LOAD_A_PATH(new QAction(QIcon(":/themes/OPEN_A_FOLDER"), "load a path", this)),
      _CLEAR_VIDEOS_LIST(new QAction(QIcon(":/themes/CLEAR_VIDEOS_LIST"), "clear playlist", this)),
      _VIDEOS_LIST_MENU(new QAction(QIcon(":/themes/VIDEOS_LIST_MENU"), "show playlist", this)),
      _MARK_HOT_SCENE(new QAction(QIcon(":/themes/MARK_HOT_SCENE_POSITION"), "mark", this)),
      _GRAB_FRAME(new QAction(QIcon(":/themes/GRAB_FRAME"), "grab", this)),
      _RENAME_VIDEO(new QAction(QIcon(":/themes/RENAME_VIDEO"), "rename", this)),
      _MOD_PERFORMERS(new QAction(QIcon(":/themes/RENAME_PERFORMERS"), "mod performers", this)),
      _RATE_AG(GetRateActionGroups()),
      _RATE_LEVEL_COUNT(_RATE_AG->actions().size()),
      _REVEAL_IN_EXPLORER(new QAction(QIcon(), "reveal in explorer", this)) {
  _PLAY_PAUSE->setEnabled(false);
  _PLAY_PAUSE->setShortcutVisibleInContextMenu(true);
  _PLAY_PAUSE->setShortcut(QKeySequence(Qt::Key_Space));
  _PLAY_PAUSE->setToolTip(QString("<b>%0 (%1)</b><br/> Switch between pause/play").arg(_PLAY_PAUSE->text(), _PLAY_PAUSE->shortcut().toString()));

  _LOAD_A_PATH->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_O));
  _LOAD_A_PATH->setShortcutVisibleInContextMenu(true);
  _LOAD_A_PATH->setToolTip(QString("<b>%0 (%1)</b><br/> Load videos from a path").arg(_LOAD_A_PATH->text(), _LOAD_A_PATH->shortcut().toString()));

  _VIDEOS_LIST_MENU->setCheckable(true);
  _VIDEOS_LIST_MENU->setChecked(true);

  _MARK_HOT_SCENE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_P));
  _MARK_HOT_SCENE->setShortcutVisibleInContextMenu(true);
  _MARK_HOT_SCENE->setToolTip(QString("<b>%0 (%1)</b><br/> Mark this position").arg(_MARK_HOT_SCENE->text(), _MARK_HOT_SCENE->shortcut().toString()));

  _GRAB_FRAME->setShortcut(QKeySequence(Qt::AltModifier | Qt::Key_N));
  _GRAB_FRAME->setShortcutVisibleInContextMenu(true);
  _GRAB_FRAME->setToolTip(
      QString("<b>%0 (%1)</b><br/> Grab one frame and output as an image").arg(_GRAB_FRAME->text(), _GRAB_FRAME->shortcut().toString()));

  _RENAME_VIDEO->setShortcut(QKeySequence(Qt::Key_F2));
  _RENAME_VIDEO->setShortcutVisibleInContextMenu(true);
  _RENAME_VIDEO->setToolTip(
      QString("<b>%0 (%1)</b><br/> Rename both video and json file name if exists").arg(_RENAME_VIDEO->text(), _RENAME_VIDEO->shortcut().toString()));

  _MOD_PERFORMERS->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_F2));
  _MOD_PERFORMERS->setShortcutVisibleInContextMenu(true);
  _MOD_PERFORMERS->setToolTip(
      QString("<b>%0 (%1)</b><br/> Mod performers in json file").arg(_MOD_PERFORMERS->text(), _MOD_PERFORMERS->shortcut().toString()));

  _REVEAL_IN_EXPLORER->setShortcutVisibleInContextMenu(true);
  _REVEAL_IN_EXPLORER->setToolTip(
      QString("<b>%0 (%1)</b><br/> Reveal file in explorer").arg(_REVEAL_IN_EXPLORER->text(), _REVEAL_IN_EXPLORER->shortcut().toString()));
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
