#include "FileXplorerEvent.h"
#include "FileXplorer.h"
#include "ExtraEvents.h"
#include "FileTool.h"
#include "PreferenceActions.h"
#include "MemoryKey.h"
#include "BehaviorKey.h"
#include "Configuration.h"
#include "Logger.h"
#include "StyleKey.h"
#include <QApplication>

#define RUN_MAIN_FILE 1
#ifdef RUN_MAIN_FILE

int main(int argc, char* argv[]) {
  if (!CreateUserPath()) {
    return -1;
  }
  if (argc > 1) {
    LOG_I("argc[%d]. argv[1]=%s.", argc, argv[1]);
  } else {
    LOG_I("argc[%d].", argc);
  }

  QApplication app{argc, argv};
  Logger::SetAutoFlushAllLevel(Configuration().value(BehaviorKey::ALL_LOG_LEVEL_AUTO_FFLUSH.name, BehaviorKey::ALL_LOG_LEVEL_AUTO_FFLUSH.toVariant()).toBool());

  const QStringList& args = app.arguments();
  QTranslator translator;  // cannot define in local. will be release.
  if (Configuration().value(MemoryKey::LANGUAGE_ZH_CN.name, MemoryKey::LANGUAGE_ZH_CN.toVariant()).toBool()) {
    LoadCNLanguagePack(translator, ":/language/ZH_CN");
  }

  FileXplorer fileExplorer{args, nullptr};

  FileXplorerEvent commonEvent{fileExplorer.m_fsPanel, fileExplorer.m_statusBar};
  commonEvent.subscribe();

  ExtraEvents extraViewVisibility{fileExplorer.m_fsPanel};
  extraViewVisibility.subscribe();

  const PreferenceActions& prefInst = g_PreferenceActions();
  prefInst.initStylePreset();
  prefInst.initStyleTheme(true);

  fileExplorer.show();

  const int exitCode = app.exec();
  LOG_I("Program:[" PROJECT_NAME "] exit with code[%d].", exitCode);
  return exitCode;
}
#endif
