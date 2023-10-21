#include "Actions/FileLeafAction.h"

FileLeafActions::FileLeafActions(QObject *parent) : QObject(parent), LEAF_FILE(GetLeafTabActions()) {}

QActionGroup *FileLeafActions::GetLeafTabActions() {
  _PREFERENCE_SETTING = new QAction(QIcon(":/themes/SETTINGS"), "&Settings");
  _ABOUT_FILE_EXPLORER = new QAction(QIcon(":/themes/ABOUT"), "&About");

  LEAF_FILE = new QActionGroup(nullptr);

  _PREFERENCE_SETTING->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_P));
  _PREFERENCE_SETTING->setShortcutVisibleInContextMenu(true);
  _PREFERENCE_SETTING->setToolTip(
      QString("<b>%0 (%1)</b><br/> Show Preference Settings Window.").arg(_PREFERENCE_SETTING->text(), _PREFERENCE_SETTING->shortcut().toString()));
  _PREFERENCE_SETTING->setCheckable(false);

  LEAF_FILE->addAction(_PREFERENCE_SETTING);
  LEAF_FILE->addAction(_ABOUT_FILE_EXPLORER);
  LEAF_FILE->setExclusive(false);
  return LEAF_FILE;
}

FileLeafActions &g_fileLeafActions() {
  static FileLeafActions g_fileLeafActions;
  return g_fileLeafActions;
}
