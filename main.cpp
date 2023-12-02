#include "FileExplorerReadOnly.h"

#include <QApplication>
#include <qDebug>

#include "FileExplorerEvent.h"
#include "Tools/SubscribeDatabase.h"

#define RUN_MAIN_FILE 1
#ifdef RUN_MAIN_FILE

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  FileExplorerReadOnly fileExplorer(argc, argv, nullptr);
  if (argc > 1) {
    qDebug("argc[%d]>1. argv[1][%s].", argc, argv[1]);
  } else {
    qDebug("argc[%d]<=1.", argc);
  }
  FileExplorerEvent fee(nullptr, fileExplorer.m_fsPanel->fileSysModel, fileExplorer.m_fsPanel->view, fileExplorer._statusBar, fileExplorer.m_jsonEditor, fileExplorer.m_videoPlayer, std::bind(&FileExplorerReadOnly::UpdateComponentVisibility, &fileExplorer));
  fee.subscribe();
  auto* eventImplementer = new SubscribeDatabase(fileExplorer.m_dbPanel->m_dbView,
                                                 fileExplorer.m_dbPanel->m_dbView->m_dbModel,
                                                 fileExplorer.m_dbPanel->m_searchLE,
                                                 std::bind(&FileExplorerReadOnly::SwitchStackWidget, &fileExplorer),
                                                 fileExplorer.m_performerManager);
  fileExplorer.show();
  a.exec();
  return 0;
}
#endif
