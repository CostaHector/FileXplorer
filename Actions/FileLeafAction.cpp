#include "FileLeafAction.h"
#include "MemoryKey.h"
#include "PublicVariable.h"

FileLeafActions::FileLeafActions(QObject* parent) : QObject(parent) {
  _SETTINGS = new (std::nothrow) QAction{QIcon(":img/SETTINGS"), "Settings"};
  if (_SETTINGS == nullptr) {
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

  _SETTINGS->setCheckable(true);
  _SETTINGS->setShortcutVisibleInContextMenu(true);
  _SETTINGS->setToolTip(QString("<b>%1 (%2)</b><br/> Show Preference Settings Window.").arg(_SETTINGS->text(), _SETTINGS->shortcut().toString()));

  _LANUAGE->setCheckable(true);
  _LANUAGE->setChecked(Configuration().value(MemoryKey::LANGUAGE_ZH_CN.name, MemoryKey::LANGUAGE_ZH_CN.v).toBool());

  leafFile->addAction(_SETTINGS);
  leafFile->addAction(_ABOUT_FILE_EXPLORER);
  leafFile->addAction(_LANUAGE);
  leafFile->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
  return leafFile;
}

FileLeafActions& g_fileLeafActions() {
  static FileLeafActions g_fileLeafActions;
  return g_fileLeafActions;
}
