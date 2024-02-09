#include "FileExplorerEvent.h"
#include "FileExplorerReadOnly.h"
#include "PublicTool.h"
#include "Tools/SubscribeDatabase.h"

#include <QApplication>
#include <QDebug>

#define RUN_MAIN_FILE 1
#ifdef RUN_MAIN_FILE

int main(int argc, char* argv[]) {
  if (not InitOutterPlainTextPath()) {
    qWarning("Init plain text path failed. Some json/where clause function may not work");
  }
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

  FileExplorerEvent fee(nullptr, fileExplorer.m_fsPanel->fileSysModel, fileExplorer.m_fsPanel->view, fileExplorer._statusBar,
                        std::bind(&FileExplorerReadOnly::UpdateComponentVisibility, &fileExplorer));
  fee.subscribe();

  auto* eventImplementer =
      new SubscribeDatabase(fileExplorer.m_dbPanel->m_dbView, std::bind(&FileExplorerReadOnly::SwitchStackWidget, &fileExplorer));
  fileExplorer.show();

  a.exec();
  return 0;
}
#endif
