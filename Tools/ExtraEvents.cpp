#include "ExtraEvents.h"
#include "TorrDBAction.h"
#include "TorrentsManagerWidget.h"

#include "FileLeafAction.h"
#include "ConfigsTable.h"
#include "SystemContextMenuControl.h"

#include "PopupWidgetManager.h"
#include "SimpleAES.h"
#include "PasswordBook.h"
#include "LoginQryWidget.h"

#include "MemoryKey.h"
#include "NotificatorMacro.h"
#include "PublicMacro.h"
#include <QMessageBox>

ExtraEvents::ExtraEvents(QWidget* parent)
  : QObject{parent} {
  CHECK_NULLPTR_RETURN_VOID(parent);
}

LoginQryWidget* ExtraEvents::LoginQryWidgetCreater(QWidget* parent) {
  auto* pLoginQryWidget = new (std::nothrow) LoginQryWidget{parent};
  CHECK_NULLPTR_RETURN_NULLPTR(pLoginQryWidget);
  m_pwdEntrance->setOnCloseCallback([pLoginQryWidget]() { pLoginQryWidget->onCancelButtonClicked(); });
  connect(pLoginQryWidget, &LoginQryWidget::accepted, pLoginQryWidget, [pLoginQryWidget, parent]() {
    QString key = pLoginQryWidget->getAESKey();
    LOG_INFO_P("key length", "%d char(s)", key.size());
    SimpleAES::InitInst(key);
    PasswordBook* pm = new (std::nothrow) PasswordBook{parent};
    CHECK_NULLPTR_RETURN_VOID(pm);
    pm->show();
  });
  return pLoginQryWidget;
}

void ExtraEvents::subscribe() {
  QWidget* pParentWidget = static_cast<QWidget*>(parent());
  CHECK_NULLPTR_RETURN_VOID(pParentWidget);

  auto& torrInst = g_torrActions();
  mTorrentsManager = new (std::nothrow) //
      PopupWidgetManager<TorrentsManagerWidget>{
          torrInst.SHOW_TORRENTS_MANAGER, //
          pParentWidget,                  //
          "TorrentsManagerWidgetGeometry" //
      };
  CHECK_NULLPTR_RETURN_VOID(mTorrentsManager);

  auto& leafInst = g_fileLeafActions();
  m_settingSys = new (std::nothrow) PopupWidgetManager<ConfigsTable>{leafInst._SETTINGS, pParentWidget, "ConfigsTableGeometry"};
  CHECK_NULLPTR_RETURN_VOID(m_settingSys);

  m_pwdEntrance = new (std::nothrow) PopupWidgetManager<LoginQryWidget>{leafInst._PWD_BOOK, pParentWidget, "PwdEntranceGeometry"};
  CHECK_NULLPTR_RETURN_VOID(m_pwdEntrance);
  m_pwdEntrance->setWidgetCreator([this](QWidget* parent) -> LoginQryWidget* { return LoginQryWidgetCreater(parent); });

  connect(leafInst._ABOUT_FILE_EXPLORER, &QAction::toggled, this, [pParentWidget]() {
    QMessageBox::about(pParentWidget,
                       "FileExplorer",
                       "Version: 62.5.6\n"
                       "Introduction: A minimalism app for image/video/json/folder explorer\n"
                       "Platform-supported: Linux and Win");
  });
  m_resMonitor = new (std::nothrow)
      PopupWidgetManager<ResourceMonitorPanel>{leafInst._CPU_MEMORY_USAGE_MONITOR, pParentWidget, "ResMonitorGeometry"};
  CHECK_NULLPTR_RETURN_VOID(m_resMonitor);

  connect(leafInst._LANUAGE, &QAction::triggered, this, [](const bool cnEnabled) {
    Configuration().setValue(MemoryKey::LANGUAGE_ZH_CN.name, cnEnabled);
    const char* languageName{cnEnabled ? "zh-cn" : "us-en"};
    LOG_INFO_P("Language switch", "[%s] work after reopen", qPrintable(languageName));
  });

  connect(leafInst._ADD_THIS_PROGRAM_TO_SYSTEM_CONTEXT_MENU, &QAction::triggered, &SystemContextMenuControl::Add);
  connect(leafInst._RMV_THIS_PROGRAM_FROM_SYSTEM_CONTEXT_MENU, &QAction::triggered, &SystemContextMenuControl::Rmv);
}
