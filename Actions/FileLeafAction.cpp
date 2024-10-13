#include "Actions/FileLeafAction.h"
#include "PublicVariable.h"

FileLeafActions::FileLeafActions(QObject* parent)
    : QObject(parent),
      _LOGGING{new QAction{QIcon(":img/FLOW_LOGS"), "Logs"}},
      _ALERT_ITEMS{new QAction(QIcon(":img/SETTINGS"), "Alerts")},
      _ABOUT_FILE_EXPLORER{new QAction(QIcon(":img/ABOUT"), "About")},
      _LANUAGE(new QAction(QIcon(":img/LANGUAGE"), "Language")),
      LEAF_FILE(GetLeafTabActions()) {}

QActionGroup* FileLeafActions::GetLeafTabActions() {
  LEAF_FILE = new QActionGroup(nullptr);

  _LOGGING->setCheckable(false);
  _LOGGING->setShortcutVisibleInContextMenu(true);
  _LOGGING->setToolTip(QString("<b>%1 (%2)</b><br/> Show log files.").arg(_LOGGING->text(), _LOGGING->shortcut().toString()));

  _ALERT_ITEMS->setCheckable(true);
  _ALERT_ITEMS->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_P));
  _ALERT_ITEMS->setShortcutVisibleInContextMenu(true);
  _ALERT_ITEMS->setToolTip(
      QString("<b>%1 (%2)</b><br/> Show Preference Settings Window.").arg(_ALERT_ITEMS->text(), _ALERT_ITEMS->shortcut().toString()));

  _LANUAGE->setCheckable(true);
  _LANUAGE->setChecked(PreferenceSettings().value(MemoryKey::LANGUAGE_ZH_CN.name, MemoryKey::LANGUAGE_ZH_CN.v).toBool());

  LEAF_FILE->addAction(_LOGGING);
  LEAF_FILE->addAction(_ALERT_ITEMS);
  LEAF_FILE->addAction(_ABOUT_FILE_EXPLORER);
  LEAF_FILE->addAction(_LANUAGE);
  LEAF_FILE->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
  return LEAF_FILE;
}

FileLeafActions& g_fileLeafActions() {
  static FileLeafActions g_fileLeafActions;
  return g_fileLeafActions;
}
