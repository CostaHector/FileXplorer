#include "FileExplorerReadOnly.h"

#include <QAction>
#include <QLineEdit>

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QTableView>

#include <QMap>
#include <QString>
#include <QVector>

#include <QFileInfo>
#include "PublicVariable.h"

#include <QDockWidget>
#include "FolderPreviewHTML.h"

const QString FileExplorerReadOnly::DEFAULT_PATH = "";

FileExplorerReadOnly::FileExplorerReadOnly(QWidget* parent, const QString& initialPath)
    : QMainWindow(parent),
      previewHtmlDock(new QDockWidget("Preview HTML")),
      previewHtml(new FolderPreviewHTML),
      previewWidget(new FolderPreviewWidget),
      explorerCentralWidget(nullptr),
      _navigationToolBar(new NavigationToolBar),
      osm(new RibbonMenu),
      _statusBar(new CustomStatusBar) {
  const QString& defaultPath = ReadSettings(initialPath);
  explorerCentralWidget = new ContentPanel(nullptr, defaultPath, previewHtml, previewWidget, _statusBar);
  this->setCentralWidget(explorerCentralWidget);

  //    previewHtmlDock->setWidget(previewHtml);
  previewHtmlDock->setWidget(previewWidget);
  previewHtmlDock->setAllowedAreas(Qt::DockWidgetArea::LeftDockWidgetArea | Qt::DockWidgetArea::RightDockWidgetArea);

  addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, previewHtmlDock);
  addToolBar(Qt::ToolBarArea::LeftToolBarArea, _navigationToolBar);
  setMenuWidget(osm);
  setStatusBar(_statusBar);
  subscribe();
}

FileExplorerReadOnly::~FileExplorerReadOnly() {}

void FileExplorerReadOnly::closeEvent(QCloseEvent* event) {
  PreferenceSettings().setValue("geometry", saveGeometry());
  PreferenceSettings().setValue("dockerWidgetWidth", previewWidget->width());
  PreferenceSettings().setValue("dockerWidgetHeight", previewWidget->height());
  PreferenceSettings().setValue("defaultOpenPath", explorerCentralWidget->CurrentPath());
  return QMainWindow::closeEvent(event);
}

auto FileExplorerReadOnly::ReadSettings(const QString& initialPath) -> QString {
  if (PreferenceSettings().contains("geometry")) {
    restoreGeometry(PreferenceSettings().value("geometry").toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }

  QString openPath;
  QString inputPath(initialPath.endsWith('"') ? initialPath.chopped(1) : initialPath);
  const QFileInfo inputFi = QFileInfo(inputPath);
  if (not inputFi.exists()) {  // input valid
    openPath = PreferenceSettings().value("defaultOpenPath", FileExplorerReadOnly::DEFAULT_PATH).toString();
  } else {  // when input invalid, use last time path
    if (inputFi.isFile()) {
      openPath = inputFi.absolutePath();
    } else {
      openPath = inputFi.absoluteFilePath();
    }
  }
  setWindowTitle("File Explorer");
  setWindowIcon(QIcon(":/themes/APP_ICON_PATH"));
  qDebug("ReadSettings openPath=[%s]", openPath.toStdString().c_str());
  return openPath;
}

#include <functional>

void FileExplorerReadOnly::subscribe() {
  if (_navigationToolBar and explorerCentralWidget) {
    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;
    auto intoNewPath = std::bind(&ContentPanel::IntoNewPath, explorerCentralWidget, _1, _2, _3);
    _navigationToolBar->subscribe(intoNewPath);
  }
}
