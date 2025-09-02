#include "FileLeafAction.h"
#include "MemoryKey.h"
#include "PublicVariable.h"
#include "PublicMacro.h"
#include "StyleSheet.h"
#include <QMenu>

FileLeafActions::FileLeafActions(QObject* parent) : QObject(parent) {
  _SETTINGS = new (std::nothrow) QAction{QIcon(":img/SETTINGS"), "Settings"};
  CHECK_NULLPTR_RETURN_VOID(_SETTINGS);
  _SETTINGS->setCheckable(true);
  _SETTINGS->setShortcutVisibleInContextMenu(true);
  _SETTINGS->setToolTip(QString("<b>%1 (%2)</b><br/> Show Preference Settings Window.").arg(_SETTINGS->text(), _SETTINGS->shortcut().toString()));


  _ABOUT_FILE_EXPLORER = new (std::nothrow) QAction{QIcon(":img/ABOUT"), "About"};
  CHECK_NULLPTR_RETURN_VOID(_ABOUT_FILE_EXPLORER);

  _LANUAGE = new (std::nothrow) QAction{QIcon(":img/LANGUAGE"), "Language"};
  CHECK_NULLPTR_RETURN_VOID(_LANUAGE);
  _LANUAGE->setCheckable(true);
  _LANUAGE->setChecked(Configuration().value(MemoryKey::LANGUAGE_ZH_CN.name, MemoryKey::LANGUAGE_ZH_CN.v).toBool());

  _ADD_THIS_PROGRAM_TO_SYSTEM_CONTEXT_MENU = new (std::nothrow) QAction{QIcon(":img/CONTEXT_MENU_ADD_THIS_PROGRAM"), "Add"};
  CHECK_NULLPTR_RETURN_VOID(_ADD_THIS_PROGRAM_TO_SYSTEM_CONTEXT_MENU);
  _ADD_THIS_PROGRAM_TO_SYSTEM_CONTEXT_MENU->setToolTip("Add this program to system context menu");

  _RMV_THIS_PROGRAM_FROM_SYSTEM_CONTEXT_MENU = new (std::nothrow) QAction{QIcon(":img/CONTEXT_MENU_RMV_THIS_PROGRAM"), "Remove"};
  CHECK_NULLPTR_RETURN_VOID(_RMV_THIS_PROGRAM_FROM_SYSTEM_CONTEXT_MENU);
  _RMV_THIS_PROGRAM_FROM_SYSTEM_CONTEXT_MENU->setToolTip("Remove this program from system context menu");

  _LEAF_FILE = GetLeafTabActions();
  CHECK_NULLPTR_RETURN_VOID(_LEAF_FILE);
}

QActionGroup* FileLeafActions::GetLeafTabActions() {
  auto* leafFile{new (std::nothrow) QActionGroup(nullptr)};
  CHECK_NULLPTR_RETURN_NULLPTR(leafFile)
  leafFile->addAction(_SETTINGS);
  leafFile->addAction(_ABOUT_FILE_EXPLORER);
  leafFile->addAction(_LANUAGE);
  leafFile->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
  return leafFile;
}

QToolButton* FileLeafActions::GetSystemContextMenu(QWidget* parent) {
  QMenu* addOrRemoveMenu = new (std::nothrow) QMenu{parent};
  CHECK_NULLPTR_RETURN_NULLPTR(addOrRemoveMenu)
  addOrRemoveMenu->addAction(_ADD_THIS_PROGRAM_TO_SYSTEM_CONTEXT_MENU);
  addOrRemoveMenu->addAction(_RMV_THIS_PROGRAM_FROM_SYSTEM_CONTEXT_MENU);
  addOrRemoveMenu->setToolTipsVisible(true);

  auto* systemContextMenuTb = new (std::nothrow) QToolButton{parent};
  CHECK_NULLPTR_RETURN_NULLPTR(systemContextMenuTb)
  systemContextMenuTb->setText("System Menu");
  systemContextMenuTb->setIcon(QIcon(":img/CONTEXT_MENU_SYSTEM"));
  systemContextMenuTb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  systemContextMenuTb->setToolTip("Add/Remove System Context Menu");
  systemContextMenuTb->setMenu(addOrRemoveMenu);
  systemContextMenuTb->setPopupMode(QToolButton::ToolButtonPopupMode::InstantPopup);
  systemContextMenuTb->setIconSize(QSize{IMAGE_SIZE::TABS_ICON_IN_MENU_48, IMAGE_SIZE::TABS_ICON_IN_MENU_48});
  return systemContextMenuTb;
}

FileLeafActions& g_fileLeafActions() {
  static FileLeafActions g_fileLeafActions;
  return g_fileLeafActions;
}
