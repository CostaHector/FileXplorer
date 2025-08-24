#include "FileExplorerEvent.h"
#include "FileXplorer.h"
#include "ExtraViewVisibilityControl.h"
#include "LogHandler.h"
#include "PublicTool.h"
#include "MemoryKey.h"
#include <QApplication>

#define RUN_MAIN_FILE 1
#ifdef RUN_MAIN_FILE

int main(int argc, char* argv[]) {
  if (!CreateUserPath()) {
    return -1;
  }

  QApplication app{argc, argv};
#ifdef QT_DEBUG
  SetQtDebugMessagePattern();
#else
  LogHandler logModule;
  if (!logModule.IsLogModuleOk()) {
    return -1;
  }
  logModule.subscribe();
#endif
  if (argc > 1) {
    qInfo("argc[%d]. argv[1]=%s.", argc, argv[1]);
  } else {
    qInfo("argc[%d].", argc);
  }

  const QStringList& args = app.arguments();
  QTranslator translator;  // cannot define in local. will be release.
  if (Configuration().value(MemoryKey::LANGUAGE_ZH_CN.name, MemoryKey::LANGUAGE_ZH_CN.v).toBool()) {
    LoadCNLanguagePack(translator);
  }

  FileXplorer fileExplorer{args, nullptr};
  FileExplorerEvent::GetFileExlorerEvent(fileExplorer.m_fsPanel->m_fsModel, fileExplorer.m_fsPanel, fileExplorer.m_statusBar);
  ExtraViewVisibilityControl extraViewVisibility{fileExplorer.centralWidget()};
  fileExplorer.show();
  app.exec();
  return 0;
}
#endif
