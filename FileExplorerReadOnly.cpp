#include "FileExplorerReadOnly.h"


#include <QLineEdit>
#include <QAction>

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QTableView>


#include <QVector>
#include <QString>
#include <QMap>


#include <QFileInfo>
#include "PublicVariable.h"

#include <QDockWidget>
#include "FolderPreviewHTML.h"

const QRect FileExplorerReadOnly::DEFAULT_GEOMETRY = QRect(0, 0, 1024, 768);
const QRect FileExplorerReadOnly::DOCKER_DEFAULT_GEOMETRY = QRect(1024/2, 0, 1024/2, 768);
const QString FileExplorerReadOnly::DEFAULT_PATH = "";

FileExplorerReadOnly::FileExplorerReadOnly(QWidget *parent, const QString& initialPath)
    : QMainWindow(parent),
      previewHtmlDock(new QDockWidget("Preview HTML")),
      previewHtml(new FolderPreviewHTML),
      previewWidget(new FolderPreviewWidget),
      explorerCentralWidget(nullptr),
      _navigationToolBar(new NavigationToolBar)
{
    const QString& defaultPath = ReadSettings(initialPath);
    explorerCentralWidget = new ContentPane(nullptr, defaultPath, previewHtml, previewWidget);
    this->setCentralWidget(explorerCentralWidget);

//    previewHtmlDock->setWidget(previewHtml);
    previewHtmlDock->setWidget(previewWidget);
    previewHtmlDock->setAllowedAreas(Qt::DockWidgetArea::LeftDockWidgetArea | Qt::DockWidgetArea::RightDockWidgetArea);

    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, previewHtmlDock);
    addToolBar(Qt::ToolBarArea::LeftToolBarArea, _navigationToolBar);

    subscribe();
}

FileExplorerReadOnly::~FileExplorerReadOnly()
{
}


void FileExplorerReadOnly::closeEvent(QCloseEvent *event){
    PreferenceSettings().setValue("geometry", saveGeometry());
    qDebug("closeEvent CurrentPath=[%s].", explorerCentralWidget->CurrentPath().toStdString().c_str());
    PreferenceSettings().setValue("defaultOpenPath", explorerCentralWidget->CurrentPath());
    return QMainWindow::closeEvent(event);
}

auto FileExplorerReadOnly::ReadSettings(const QString& initialPath)->QString{
    if (PreferenceSettings().contains("geometry")){
        restoreGeometry(PreferenceSettings().value("geometry").toByteArray());
    }else{
        setGeometry(FileExplorerReadOnly::DEFAULT_GEOMETRY);
    }

    QString openPath;
    QString inputPath(initialPath.endsWith('"')? initialPath.chopped(1): initialPath);
    const QFileInfo inputFi = QFileInfo(inputPath);
    if (inputFi.exists()){ // input valid
        if (inputFi.isFile()){
            openPath = inputFi.absolutePath();
        }else{
            openPath = inputFi.absoluteFilePath();
        }
    }else{ // when input invalid, use last time path
        openPath = PreferenceSettings().value("defaultOpenPath", FileExplorerReadOnly::DEFAULT_PATH).toString();
    }
    setWindowTitle("File Explorer");
    setWindowIcon(QIcon(":/themes/APP_ICON_PATH"));
    qDebug("ReadSettings defaultPath=[%s]", openPath.toStdString().c_str());
    return openPath;
}

#include <functional>

bool FileExplorerReadOnly::subscribe() {
    if (_navigationToolBar and explorerCentralWidget){
        using std::placeholders::_1;
        using std::placeholders::_2;
        using std::placeholders::_3;
        auto intoNewPath = std::bind(&ContentPane::IntoNewPath, explorerCentralWidget, _1,_2,_3);
        _navigationToolBar->subscribe(intoNewPath);
    }
}
