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
  FileExplorerEvent fee(nullptr, fileExplorer.fsmView->fileSysModel, fileExplorer.fsmView->view, fileExplorer._statusBar, fileExplorer.m_jsonEditor, std::bind(&FileExplorerReadOnly::UpdateComponentVisibility, &fileExplorer));
  fee.subscribe();
  auto* eventImplementer = new SubscribeDatabase(fileExplorer.dbView, fileExplorer.dbView->dbModel, fileExplorer.osm->sqlSearchLE,
                                                 std::bind(&FileExplorerReadOnly::SwitchStackWidget, &fileExplorer));
  fileExplorer.show();
  a.exec();
  return 0;
}
#endif
