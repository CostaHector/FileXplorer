#include "FileExplorerEvent.h"
#include "FileXplorer.h"
#include "ExtraEvents.h"
#include "PublicTool.h"
#include "MemoryKey.h"
#include "Logger.h"
#include <QApplication>

#define RUN_MAIN_FILE 1
#ifdef RUN_MAIN_FILE

int main(int argc, char* argv[]) {
  if (!CreateUserPath()) {
    return -1;
  }

  QApplication app{argc, argv};
  if (argc > 1) {
    LOG_I("argc[%d]. argv[1]=%s.", argc, argv[1]);
  } else {
    LOG_I("argc[%d].", argc);
  }
  Logger::SetAutoFlushAllLevel(Configuration().value(MemoryKey::ALL_LOG_LEVEL_AUTO_FFLUSH.name, MemoryKey::ALL_LOG_LEVEL_AUTO_FFLUSH.v).toBool());

  const QStringList& args = app.arguments();
  QTranslator translator;  // cannot define in local. will be release.
  if (Configuration().value(MemoryKey::LANGUAGE_ZH_CN.name, MemoryKey::LANGUAGE_ZH_CN.v).toBool()) {
    LoadCNLanguagePack(translator, ":/language/ZH_CN");
  }

  FileXplorer fileExplorer{args, nullptr};

  FileExplorerEvent commonEvent{fileExplorer.m_fsPanel->m_fsModel, fileExplorer.m_fsPanel, fileExplorer.m_statusBar};
  commonEvent.subscribe();

  ExtraEvents extraViewVisibility{fileExplorer.m_fsPanel};
  extraViewVisibility.subscribe();

  fileExplorer.show();
  const int exitCode = app.exec();
  LOG_I("Program:[" PROJECT_NAME "] exit with code[%d].", exitCode);
  return exitCode;
}
#endif
