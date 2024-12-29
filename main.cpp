#include "FileExplorerEvent.h"
#include "FileExplorerReadOnly.h"
#include "InitCheck.h"
#include "PublicTool.h"
#include "Tools/ExtraViewVisibilityControl.h"
#include "Tools/LogHandler.h"

#include <QApplication>

#define RUN_MAIN_FILE 1
#ifdef RUN_MAIN_FILE

int main(int argc, char* argv[]) {
  if (!InitCheck::InitOutterPlainTextPath()) {
    qWarning("Init plain text path failed. Some json/where clause function may not work");
  }

  LogHandler mo;

  if (argc > 1) {
    qInfo("argc[%d]>1. argv[1][%s].", argc, argv[1]);
  } else {
    qInfo("argc[%d]<=1.", argc);
  }

  QApplication a(argc, argv);

  QTranslator translator;  // cannot define in local. will be release.
  if (PreferenceSettings().value(MemoryKey::LANGUAGE_ZH_CN.name, MemoryKey::LANGUAGE_ZH_CN.v).toBool()) {
    LoadCNLanguagePack(translator);
  }

  FileExplorerReadOnly fileExplorer(argc, argv, nullptr);
  FileExplorerEvent::GetFileExlorerEvent(fileExplorer.m_fsPanel->m_fsModel, fileExplorer.m_fsPanel, fileExplorer.m_statusBar);

  auto* extraViewVisibility = new (std::nothrow)ExtraViewVisibilityControl(fileExplorer.centralWidget());
  fileExplorer.show();

  mo.subscribe();
  a.exec();

  delete extraViewVisibility;
  return 0;
}
#endif
