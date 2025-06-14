#include "FileExplorerEvent.h"
#include "FileXplorer.h"
#include "Tools/ExtraViewVisibilityControl.h"
#include "Tools/LogHandler.h"
#include "public/PublicTool.h"
#include "public/MemoryKey.h"
#include <QApplication>

#define RUN_MAIN_FILE 1
#ifdef RUN_MAIN_FILE

int main(int argc, char* argv[]) {
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

  FileXplorer fileExplorer{argc, argv, nullptr};
  FileExplorerEvent::GetFileExlorerEvent(fileExplorer.m_fsPanel->m_fsModel, fileExplorer.m_fsPanel, fileExplorer.m_statusBar);
  ExtraViewVisibilityControl extraViewVisibility{fileExplorer.centralWidget()};
  fileExplorer.show();

  mo.subscribe();
  a.exec();
  return 0;
}
#endif
