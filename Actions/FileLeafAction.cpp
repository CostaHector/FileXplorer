#include "Actions/FileLeafAction.h"
#include "PublicVariable.h"

FileLeafActions::FileLeafActions(QObject* parent) : QObject(parent) {
  _ALERT_ITEMS = new QAction{QIcon(":img/SETTINGS"), "Alerts"};
  if (_ALERT_ITEMS == nullptr) {
    qCritical("_ALERT_ITEMS is nullptr");
    return;
  }

  _ABOUT_FILE_EXPLORER = new (std::nothrow) QAction{QIcon(":img/ABOUT"), "About"};
  if (_ABOUT_FILE_EXPLORER == nullptr) {
    qCritical("_ABOUT_FILE_EXPLORER is nullptr");
    return;
  }

  _LANUAGE = new (std::nothrow) QAction{QIcon(":img/LANGUAGE"), "Language"};
  if (_LANUAGE == nullptr) {
    qCritical("_LANUAGE is nullptr");
    return;
  }

  _LEAF_FILE = GetLeafTabActions();
  if (_LEAF_FILE == nullptr) {
    qCritical("_LEAF_FILE is nullptr");
    return;
  }
}

QActionGroup* FileLeafActions::GetLeafTabActions() {
  auto* leafFile{new (std::nothrow) QActionGroup(nullptr)};
  if (leafFile == nullptr) {
    qCritical("_LANUAGE is nullptr");
    return nullptr;
  }

  _ALERT_ITEMS->setCheckable(true);
  _ALERT_ITEMS->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_P));
  _ALERT_ITEMS->setShortcutVisibleInContextMenu(true);
  _ALERT_ITEMS->setToolTip(QString("<b>%1 (%2)</b><br/> Show Preference Settings Window.").arg(_ALERT_ITEMS->text(), _ALERT_ITEMS->shortcut().toString()));

  _LANUAGE->setCheckable(true);
  _LANUAGE->setChecked(PreferenceSettings().value(MemoryKey::LANGUAGE_ZH_CN.name, MemoryKey::LANGUAGE_ZH_CN.v).toBool());

  leafFile->addAction(_ALERT_ITEMS);
  leafFile->addAction(_ABOUT_FILE_EXPLORER);
  leafFile->addAction(_LANUAGE);
  leafFile->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
  return leafFile;
}

FileLeafActions& g_fileLeafActions() {
  static FileLeafActions g_fileLeafActions;
  return g_fileLeafActions;
}
