#include "FileExplorerEvent.h"
#include "FileExplorerReadOnly.h"
#include "Tools/SubscribeDatabase.h"

#include <QApplication>
#include <QDebug>
#include <QTranslator>

#define RUN_MAIN_FILE 1
#ifdef RUN_MAIN_FILE

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);

  //  if (PreferenceSettings().value(MemoryKey::LANGUAGE_ZH_CN.name, MemoryKey::LANGUAGE_ZH_CN.v).toBool()) {
  //  QTranslator translator;
  //  const QString baseName = "FileExplorerReadOnly_zh_CN";
  //  if (translator.load(":/i18n/" + baseName)) {
  //    a.installTranslator(&translator);
  //  } else {
  //    qDebug("Cannot load %s", qPrintable(baseName));
  //  }
  //  }

  FileExplorerReadOnly fileExplorer(argc, argv, nullptr);
  if (argc > 1) {
    qDebug("argc[%d]>1. argv[1][%s].", argc, argv[1]);
  } else {
    qDebug("argc[%d]<=1.", argc);
  }
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
