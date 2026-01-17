#include "FileLeafAction.h"
#include "MemoryKey.h"
#include "PublicVariable.h"
#include "PublicMacro.h"
#include "StyleSheet.h"
#include <QMenu>

FileLeafActions::FileLeafActions(QObject* parent) : QObject(parent) {
  _SETTINGS = new (std::nothrow) QAction{QIcon(":img/SETTINGS"), tr("Settings"), this};
  CHECK_NULLPTR_RETURN_VOID(_SETTINGS);
  _SETTINGS->setCheckable(true);
  _SETTINGS->setShortcutVisibleInContextMenu(true);
  _SETTINGS->setToolTip(QString("<b>%1 (%2)</b><br/> Show Preference Settings Window.").arg(_SETTINGS->text(), _SETTINGS->shortcut().toString()));

  _PWD_BOOK = new (std::nothrow) QAction{QIcon(":/PASSWORD_TABLE"), tr("Pwd book"), this};
  CHECK_NULLPTR_RETURN_VOID(_PWD_BOOK);
  _PWD_BOOK->setCheckable(true);
  _PWD_BOOK->setShortcutVisibleInContextMenu(true);
  _PWD_BOOK->setToolTip(QString("<b>%1 (%2)</b><br/> Show Password book.").arg(_PWD_BOOK->text(), _PWD_BOOK->shortcut().toString()));

  _ABOUT_FILE_EXPLORER = new (std::nothrow) QAction{QIcon(":img/ABOUT"), tr("About")};
  CHECK_NULLPTR_RETURN_VOID(_ABOUT_FILE_EXPLORER);
  _ABOUT_FILE_EXPLORER->setCheckable(true);

  _LANUAGE = new (std::nothrow) QAction{QIcon(":img/LANGUAGE"), tr("Language"), this};
  CHECK_NULLPTR_RETURN_VOID(_LANUAGE);
  _LANUAGE->setCheckable(true);
  _LANUAGE->setChecked(Configuration().value(MemoryKey::LANGUAGE_ZH_CN.name, MemoryKey::LANGUAGE_ZH_CN.v).toBool());

  _CPU_MEMORY_USAGE_MONITOR = new (std::nothrow) QAction{QIcon(":img/USAGE_MONITOR"), tr("Usage Monitor"), this};
  CHECK_NULLPTR_RETURN_VOID(_CPU_MEMORY_USAGE_MONITOR);
  _CPU_MEMORY_USAGE_MONITOR->setCheckable(true);
  _CPU_MEMORY_USAGE_MONITOR->setChecked(false);

  _ADD_THIS_PROGRAM_TO_SYSTEM_CONTEXT_MENU = new (std::nothrow) QAction{QIcon(":img/CONTEXT_MENU_ADD_THIS_PROGRAM"), tr("Add"), this};
  CHECK_NULLPTR_RETURN_VOID(_ADD_THIS_PROGRAM_TO_SYSTEM_CONTEXT_MENU);
  _ADD_THIS_PROGRAM_TO_SYSTEM_CONTEXT_MENU->setToolTip("Add this program to system context menu");

  _RMV_THIS_PROGRAM_FROM_SYSTEM_CONTEXT_MENU = new (std::nothrow) QAction{QIcon(":img/CONTEXT_MENU_RMV_THIS_PROGRAM"), tr("Remove"), this};
  CHECK_NULLPTR_RETURN_VOID(_RMV_THIS_PROGRAM_FROM_SYSTEM_CONTEXT_MENU);
  _RMV_THIS_PROGRAM_FROM_SYSTEM_CONTEXT_MENU->setToolTip("Remove this program from system context menu");
}

QToolButton* FileLeafActions::GetSystemContextMenu(QWidget* parent) {
  QMenu* addOrRemoveMenu = new (std::nothrow) QMenu{parent};
  CHECK_NULLPTR_RETURN_NULLPTR(addOrRemoveMenu)
  addOrRemoveMenu->addAction(_ADD_THIS_PROGRAM_TO_SYSTEM_CONTEXT_MENU);
  addOrRemoveMenu->addAction(_RMV_THIS_PROGRAM_FROM_SYSTEM_CONTEXT_MENU);
  addOrRemoveMenu->setToolTipsVisible(true);

  auto* systemContextMenuTb = new (std::nothrow) QToolButton{parent};
  CHECK_NULLPTR_RETURN_NULLPTR(systemContextMenuTb)
  systemContextMenuTb->setText(tr("System Menu"));
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
