#include "FileExplorerReadOnly.h"

#include <QApplication>
#include <QDebug>

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
  fileExplorer.show();
  FileExplorerEvent fee(nullptr, fileExplorer.fsmView->fileSysModel, fileExplorer.fsmView->view, fileExplorer._statusBar);
  fee.subscribe();
  auto* eventImplementer = new SubscribeDatabase(fileExplorer.dbView, fileExplorer.dbView->dbModel, fileExplorer.osm->sqlSearchLE,
                                                 std::bind(&FileExplorerReadOnly::HotUpdate, &fileExplorer));
  a.exec();
  return 0;
}
#endif
