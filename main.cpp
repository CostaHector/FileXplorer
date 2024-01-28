#include "FileExplorerEvent.h"
#include "FileExplorerReadOnly.h"
#include "Tools/SubscribeDatabase.h"

#include <QApplication>
#include <QDebug>
#include <QTranslator>

#define RUN_MAIN_FILE 1
#ifdef RUN_MAIN_FILE

void LoadCNLanguagePack(QTranslator& translator) {
  qDebug("Load Chinese pack");
  const QString baseName = "FileExplorerReadOnly_zh_CN";
  if (translator.load(":/i18n/" + baseName)) {
    qDebug("Load language pack succeed %s", qPrintable(baseName));
    QCoreApplication::installTranslator(&translator);
  }
}

void LoadSysLanaguagePack(QTranslator& translator) {
  qDebug("Load System Language pack");
  const QStringList uiLanguages = QLocale::system().uiLanguages();
  for (const QString& locale : uiLanguages) {
    const QString baseName = "FileExplorerReadOnly" + QLocale(locale).name();
    if (translator.load(":/i18n/" + baseName)) {
      qDebug("Load language pack succeed %s", qPrintable(baseName));
      QCoreApplication::installTranslator(&translator);
      break;
    } else {
      qDebug("No need to load language pack %s", qPrintable(baseName));
    }
  }
}

int main(int argc, char* argv[]) {
  if (argc > 1) {
    qDebug("argc[%d]>1. argv[1][%s].", argc, argv[1]);
  } else {
    qDebug("argc[%d]<=1.", argc);
  }

  QApplication a(argc, argv);

  QTranslator translator; // cannot define in local. will be release.
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
