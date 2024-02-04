#include "ViewActions.h"

ViewActions& g_viewActions() {
  static ViewActions ins;
  return ins;
}

QActionGroup* ViewActions::Get_NAVIGATION_PANE_Actions(){
  NAVIGATION_PANE->setToolTip(
      QString("<b>%1 (%2)</b><br/> Show or hide the navigation pane.").arg(NAVIGATION_PANE->text(), NAVIGATION_PANE->shortcut().toString()));
  NAVIGATION_PANE->setCheckable(true);

  PREVIEW_PANE_HTML->setShortcut(QKeySequence(Qt::KeyboardModifier::AltModifier | Qt::Key::Key_P));
  PREVIEW_PANE_HTML->setToolTip(
      QString("<b>%1 (%2)</b><br/> Show or hide the preview pane.").arg(PREVIEW_PANE_HTML->text(), PREVIEW_PANE_HTML->shortcut().toString()));
  PREVIEW_PANE_HTML->setCheckable(true);

  _JSON_EDITOR_PANE->setToolTip(
      QString("<b>%1 (%2)</b><br/>Show Json Edit Pane.").arg(_JSON_EDITOR_PANE->text(), _JSON_EDITOR_PANE->shortcut().toString()));

  _VIDEO_PLAYER_EMBEDDED->setShortcutVisibleInContextMenu(true);
  _VIDEO_PLAYER_EMBEDDED->setToolTip(QString("<b>%1 (%2)</b><br/> Open the selected item in embedded video player.")
                                         .arg(_VIDEO_PLAYER_EMBEDDED->text(), _VIDEO_PLAYER_EMBEDDED->shortcut().toString()));

  auto* actionGroup = new QActionGroup(this);
  actionGroup->addAction(NAVIGATION_PANE);
  actionGroup->addAction(PREVIEW_PANE_HTML);
  actionGroup->addAction(_JSON_EDITOR_PANE);
  actionGroup->addAction(_VIDEO_PLAYER_EMBEDDED);
  actionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  NAVIGATION_PANE->setChecked(PreferenceSettings().value(MemoryKey::SHOW_QUICK_NAVIGATION_TOOL_BAR.name).toBool());
  PREVIEW_PANE_HTML->setChecked(PreferenceSettings().value(MemoryKey::SHOW_FOLDER_PREVIEW_HTML.name).toBool());
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
