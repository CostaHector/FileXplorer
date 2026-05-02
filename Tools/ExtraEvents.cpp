#include "ExtraEvents.h"

#include "TorrDBAction.h"
#include "TorrentsManagerWidget.h"

#include "FileLeafAction.h"
#include "ConfigsMgr.h"
#include "SystemContextMenuControl.h"

#include "PopupWidgetManager.h"
#ifdef PASSVAULT_ENABLED
#include "SimpleAES.h"
#include "PasswordBook.h"
#endif

#include "PreferenceActions.h"
#include "StyleSheetMgr.h"

#include "MemoryKey.h"
#include "Configuration.h"

#include "NotificatorMacro.h"
#include "PublicMacro.h"

#include <QMessageBox>

ExtraEvents::ExtraEvents(QWidget* parent)
  : QObject{parent} {
  CHECK_NULLPTR_RETURN_VOID(parent);
}

void ExtraEvents::subscribe() {
  QWidget* pParentWidget = static_cast<QWidget*>(parent());
  CHECK_NULLPTR_RETURN_VOID(pParentWidget);

  auto& torrInst = g_torrActions();
  mTorrentsManager = new (std::nothrow) //
      PopupWidgetManager<TorrentsManagerWidget>{
          torrInst.SHOW_TORRENTS_MANAGER, //
          pParentWidget,                  //
          "Geometry/TorrentsManagerWidget" //
      };
  CHECK_NULLPTR_RETURN_VOID(mTorrentsManager);

  auto& leafInst = g_fileLeafActions();
  m_settingSys = new (std::nothrow) PopupWidgetManager<ConfigsMgr>{leafInst._SETTINGS, pParentWidget, "Geometry/ConfigsTable"};
  CHECK_NULLPTR_RETURN_VOID(m_settingSys);

#ifdef PASSVAULT_ENABLED
  mPwdBook = new (std::nothrow) PopupWidgetManager<PasswordBook>{leafInst._PWD_BOOK, pParentWidget, "Geometry/PasswordBook"};
  CHECK_NULLPTR_RETURN_VOID(mPwdBook);
  mPwdBook->setWidgetCreator(PasswordBook::Creater);
#endif

  connect(leafInst._ABOUT_FILE_EXPLORER, &QAction::triggered, this, []() {
    QString aboutText = R"(<h3>FileXplorer</h3>
<p>A minimalist, cross-platform file management suite for media professionals.</p>
<hr>
<p><b>Copyright © 2026 [CostaHector]</b></p>
<p>This program is free software: you can redistribute it and/or modify it under the terms of the
<a href='https://www.gnu.org/licenses/gpl-3.0.html'>GNU General Public License v3.0 (GPL v3)</a>.</p>
<br>
<p><b>Third-Party Components:</b></p>
<ul>
    <li><b>Qt</b> © The Qt Company<br>
        Licensed under <a href='https://www.gnu.org/licenses/lgpl-3.0.html'>LGPL v3</a>.
    </li>
    <li><b>FFmpeg</b> © The FFmpeg Developers<br>
        Licensed under <a href='https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html'>LGPL v2.1+</a>.<br>
        <i>Note: This application statically links against FFmpeg.</i>
    </li>
    <li><b>OpenSSL</b> © OpenSSL Software Foundation<br>
        Licensed under <a href='https://www.openssl.org/source/license.html'>Apache License 2.0</a>.
    </li>
    <li><b>MediaInfo</b> © MediaArea.net SARL<br>
        Licensed under <a href='https://mediaarea.net/en/MediaInfo/License'>Simplified BSD License</a>.
    </li>
    <li><b>DB Browser for SQLite Icons</b> © The DB Browser for SQLite Team<br>
        Licensed under <a href='https://www.gnu.org/licenses/gpl-3.0.html'>GPL v3</a>.
    </li>
</ul>
<hr>
<p>Source code available at:<br>
<a href='https://github.com/CostaHector/FileXplorer'>github.com/CostaHector/FileXplorer</a></p>
<p>Platform: Linux & Windows</p>)";

    QMessageBox msgBox;
    msgBox.setWindowTitle("About FileXplorer");
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText(aboutText);
    msgBox.setIconPixmap(QPixmap{":img/APP_ICON_PATH"}.scaled(64, 64));
    msgBox.setWindowIcon(QIcon{":img/APP_ICON_PATH"});
    msgBox.exec();
  });

  m_resMonitor = new (std::nothrow)
      PopupWidgetManager<ResourceMonitorPanel>{leafInst._CPU_MEMORY_USAGE_MONITOR, pParentWidget, "Geometry/ResMonitor"};
  CHECK_NULLPTR_RETURN_VOID(m_resMonitor);

  auto& prefInst = g_PreferenceActions();
  connect(prefInst.STYLESHEET_MGR, &QAction::triggered, this, []() {
    StyleSheetMgr styleSheetMgr;
    styleSheetMgr.exec();
  });

  connect(leafInst._LANUAGE, &QAction::triggered, this, [](const bool cnEnabled) {
    setConfig(MemoryKey::LANGUAGE_ZH_CN, cnEnabled);
    const char* languageName{cnEnabled ? "zh-cn" : "us-en"};
    LOG_INFO_P("Language switch", "[%s] work after reopen", qPrintable(languageName));
  });

  connect(leafInst._ADD_THIS_PROGRAM_TO_SYSTEM_CONTEXT_MENU, &QAction::triggered, &SystemContextMenuControl::Add);
  connect(leafInst._RMV_THIS_PROGRAM_FROM_SYSTEM_CONTEXT_MENU, &QAction::triggered, &SystemContextMenuControl::Rmv);
}
