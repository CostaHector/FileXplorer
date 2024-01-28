#include "Actions/FileLeafAction.h"
#include "PublicVariable.h"

FileLeafActions::FileLeafActions(QObject* parent)
    : QObject(parent),
      _PREFERENCE_SETTING{new QAction(QIcon(":/themes/SETTINGS"), tr("Settings"))},
      _ABOUT_FILE_EXPLORER{new QAction(QIcon(":/themes/ABOUT"), tr("About"))},
      _LANUAGE(new QAction(QIcon(":/themes/LANGUAGE"), tr("Language"))),
      LEAF_FILE(GetLeafTabActions()) {}

QActionGroup* FileLeafActions::GetLeafTabActions() {
  LEAF_FILE = new QActionGroup(nullptr);

  _PREFERENCE_SETTING->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_P));
  _PREFERENCE_SETTING->setShortcutVisibleInContextMenu(true);
  _PREFERENCE_SETTING->setToolTip(
      QString("<b>%1 (%2)</b><br/> Show Preference Settings Window.").arg(_PREFERENCE_SETTING->text(), _PREFERENCE_SETTING->shortcut().toString()));
  _PREFERENCE_SETTING->setCheckable(false);

  _LANUAGE->setCheckable(true);
  _LANUAGE->setChecked(PreferenceSettings().value(MemoryKey::LANGUAGE_ZH_CN.name, MemoryKey::LANGUAGE_ZH_CN.v).toBool());

  LEAF_FILE->addAction(_PREFERENCE_SETTING);
  LEAF_FILE->addAction(_ABOUT_FILE_EXPLORER);
  LEAF_FILE->addAction(_LANUAGE);
  LEAF_FILE->setExclusive(false);
  return LEAF_FILE;
}

FileLeafActions& g_fileLeafActions() {
  static FileLeafActions g_fileLeafActions;
  return g_fileLeafActions;
}
