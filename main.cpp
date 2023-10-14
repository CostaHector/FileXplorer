#include "FileExplorerReadOnly.h"

#include <QApplication>
#include <QDebug>

#include "PublicVariable.h"
#include "FileExplorerEvent.h"

//#define RUN_MAIN_FILE 1
#ifdef RUN_MAIN_FILE

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FileExplorerReadOnly* fileExplorer = nullptr;
    if (argc > 1){
        qDebug("argc[%d]>1. argv[1][%s].", argc, argv[1]);
        fileExplorer = new FileExplorerReadOnly(nullptr, argv[1]);
    }else{
        qDebug("argc[%d]<=1.", argc);
        fileExplorer = new FileExplorerReadOnly();
    }

    fileExplorer->show();

    FileExplorerEvent fee(nullptr, fileExplorer->explorerCentralWidget->fileSysModel, fileExplorer->explorerCentralWidget->view);
    fee.subscribe();

    a.exec();
    delete fileExplorer;
    return 0;
}
#endif
