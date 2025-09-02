#include "ExtraEvents.h"
#include "TorrDBAction.h"
#include "TorrentsManagerWidget.h"

#include "FileLeafAction.h"
#include "ConfigsTable.h"
#include "SystemContextMenuControl.h"

#include "OnCheckedPopupOrHideAWidget.h"

#include "MemoryKey.h"
#include "NotificatorMacro.h"
#include "PublicMacro.h"

#include <QMessageBox>

ExtraEvents::ExtraEvents(QWidget* parent)
  : QObject{parent}, parentWidget{parent} {
}

void ExtraEvents::subscribe() {
  connect(g_torrActions().SHOW_TORRENTS_MANAGER, &QAction::toggled, this, &ExtraEvents::on_showTorrentsManager);

  {
    auto& leafInst = g_fileLeafActions();
    connect(leafInst._SETTINGS, &QAction::toggled, this, &ExtraEvents::on_settings);
    connect(leafInst._ABOUT_FILE_EXPLORER, &QAction::triggered, this, [this]() {
      QMessageBox::about(parentWidget, "FileExplorer",
                         "Version: 46.0\n"
                         "Introduction: A minimalism app for image/video/json/folder explorer\n"
                         "Platform-supported: Linux and Win");
    });
    connect(leafInst._LANUAGE, &QAction::triggered, this, [](const bool cnEnabled) {
      Configuration().setValue(MemoryKey::LANGUAGE_ZH_CN.name, cnEnabled);
      LOG_INFO_NP("Language switch", "work after reopen");
    });
    connect(leafInst._ADD_THIS_PROGRAM_TO_SYSTEM_CONTEXT_MENU, &QAction::triggered, &SystemContextMenuControl::Add);
    connect(leafInst._RMV_THIS_PROGRAM_FROM_SYSTEM_CONTEXT_MENU, &QAction::triggered, &SystemContextMenuControl::Rmv);
  }
}

void ExtraEvents::on_showTorrentsManager(const bool checked) {
  mTorrentsManager = PopupHideWidget<TorrentsManagerWidget>(mTorrentsManager, checked, parentWidget);
  CHECK_NULLPTR_RETURN_VOID(mTorrentsManager)
}

void ExtraEvents::on_settings(const bool checked) {
  m_settingSys = PopupHideWidget<ConfigsTable>(m_settingSys, checked, parentWidget);
  CHECK_NULLPTR_RETURN_VOID(m_settingSys)                                                                              //
}
