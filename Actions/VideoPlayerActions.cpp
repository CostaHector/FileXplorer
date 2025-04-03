#include "VideoPlayerActions.h"
#include "PublicVariable.h"
#include "Component/SpacerWidget.h"

VideoPlayerActions::VideoPlayerActions(QObject* parent) : QObject{parent} {
  _VOLUME_CTRL_MUTE->setCheckable(true);
  _VOLUME_CTRL_MUTE->setChecked(PreferenceSettings().value(MemoryKey::VIDEO_PLAYER_MUTE.name, MemoryKey::VIDEO_PLAYER_MUTE.v).toBool());
  if (_VOLUME_CTRL_MUTE->isChecked()) {
    _VOLUME_CTRL_MUTE->setIcon(QIcon(":img/VOLUME_MUTE"));
  } else {
    _VOLUME_CTRL_MUTE->setIcon(QIcon(":img/VOLUME_UNMUTE"));
  }

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
  _LAST_10_SECONDS->setToolTip(QString("<b>%1 (%2)</b><br/> minus 10s").arg(_LAST_10_SECONDS->text(), _LAST_10_SECONDS->shortcut().toString()));

  _NEXT_10_SECONDS->setShortcut(QKeySequence(Qt::Key_Right));
  _NEXT_10_SECONDS->setShortcutVisibleInContextMenu(true);
  _NEXT_10_SECONDS->setToolTip(QString("<b>%1 (%2)</b><br/> plus 10s").arg(_NEXT_10_SECONDS->text(), _NEXT_10_SECONDS->shortcut().toString()));

  _AUTO_PLAY_NEXT_VIDEO->setCheckable(true);
  _AUTO_PLAY_NEXT_VIDEO->setChecked(PreferenceSettings().value(MemoryKey::AUTO_PLAY_NEXT_VIDEO.name, MemoryKey::AUTO_PLAY_NEXT_VIDEO.v).toBool());
  _AUTO_PLAY_NEXT_VIDEO->setToolTip(QString("<b>%1 (%2)</b><br/> Auto play next video when current finished").arg(_AUTO_PLAY_NEXT_VIDEO->text(), _AUTO_PLAY_NEXT_VIDEO->shortcut().toString()));

  _PLAY_PAUSE->setEnabled(false);
  _PLAY_PAUSE->setShortcutVisibleInContextMenu(true);
  _PLAY_PAUSE->setShortcut(QKeySequence(Qt::Key_Space));
  _PLAY_PAUSE->setToolTip(QString("<b>%1 (%2)</b><br/> Start play or pause").arg(_PLAY_PAUSE->text(), _PLAY_PAUSE->shortcut().toString()));

  _LOAD_A_PATH->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_O));
  _LOAD_A_PATH->setShortcutVisibleInContextMenu(true);
  _LOAD_A_PATH->setToolTip(QString("<b>%1 (%2)</b><br/> Load videos from a path").arg(_LOAD_A_PATH->text(), _LOAD_A_PATH->shortcut().toString()));

  _SHOW_VIDEOS_LIST->setCheckable(true);
  _SHOW_VIDEOS_LIST->setChecked(
      PreferenceSettings().value(MemoryKey::KEEP_VIDEOS_PLAYLIST_SHOW.name, MemoryKey::KEEP_VIDEOS_PLAYLIST_SHOW.v).toBool());

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

QToolBar* VideoPlayerActions::GetPlayControlToolBar(QWidget* parent, QLabel* label) {
  auto* controlTB = new QToolBar("play control", parent);
  auto* spacer = GetSpacerWidget();

  controlTB->addAction(g_videoPlayerActions()._LAST_VIDEO);
  controlTB->addAction(g_videoPlayerActions()._NEXT_VIDEO);
  controlTB->addAction(g_videoPlayerActions()._OPEN_A_VIDEO);
  controlTB->addSeparator();
  if (label != nullptr) {
    controlTB->addWidget(label);
  }
  controlTB->addSeparator();
  controlTB->addAction(g_videoPlayerActions()._MARK_HOT_SCENE);
  controlTB->addAction(g_videoPlayerActions()._GRAB_FRAME);
  controlTB->addSeparator();
  controlTB->addAction(g_videoPlayerActions()._RENAME_VIDEO);
  controlTB->addAction(g_videoPlayerActions()._MOD_PERFORMERS);
  controlTB->addSeparator();
  controlTB->addActions(g_videoPlayerActions()._RATE_AG->actions());
  controlTB->addSeparator();
  controlTB->addWidget(spacer);
  controlTB->addSeparator();
  controlTB->addAction(g_videoPlayerActions()._AUTO_PLAY_NEXT_VIDEO);
  controlTB->addSeparator();
  controlTB->addAction(g_videoPlayerActions()._SCROLL_TO_LAST_FOLDER);
  controlTB->addAction(g_videoPlayerActions()._SCROLL_TO_NEXT_FOLDER);
  controlTB->addSeparator();
  controlTB->addAction(g_videoPlayerActions()._CLEAR_VIDEOS_LIST);
  controlTB->addAction(g_videoPlayerActions()._LOAD_A_PATH);
  controlTB->addAction(g_videoPlayerActions()._SHOW_VIDEOS_LIST);
  controlTB->addAction(g_videoPlayerActions()._UPDATE_ITEM_PLAYABLE);
  controlTB->addSeparator();
  controlTB->addAction(g_videoPlayerActions()._MOVE_SELECTED_ITEMS_TO_TRASHBIN);
  controlTB->setContentsMargins(0, 0, 0, 0);
  return controlTB;
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
