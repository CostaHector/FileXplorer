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
    qFatal("Init plain text path failed. Force quit now!");
    return -1;
  }

  if (argc > 1) {
    qDebug("argc[%d]>1. argv[1][%s].", argc, argv[1]);
  } else {
    qDebug("argc[%d]<=1.", argc);
  }

  QApplication a(argc, argv);

  QTranslator translator;  // cannot define in local. will be release.
  if (PreferenceSettings().value(MemoryKey::LANGUAGE_ZH_CN.name, MemoryKey::LANGUAGE_ZH_CN.v).toBool()) {
    LoadCNLanguagePack(translator);
  }

  FileExplorerReadOnly fileExplorer(argc, argv, nullptr);

  FileExplorerEvent fee(nullptr, fileExplorer.m_fsPanel->fileSysModel, fileExplorer.m_fsPanel->view, fileExplorer._statusBar,
                        fileExplorer.m_jsonEditor, fileExplorer.m_videoPlayer,
                        std::bind(&FileExplorerReadOnly::UpdateComponentVisibility, &fileExplorer));
  fee.subscribe();

  auto* eventImplementer = new SubscribeDatabase(fileExplorer.m_dbPanel->m_dbView, std::bind(&FileExplorerReadOnly::SwitchStackWidget, &fileExplorer),
                                                 fileExplorer.m_performerManager, fileExplorer.m_torrentsManager);
  fileExplorer.show();

  a.exec();
  return 0;
}
#endif
