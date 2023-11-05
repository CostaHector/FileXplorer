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

FileExplorerReadOnly::FileExplorerReadOnly(const int argc, char const* const argv[], QWidget* parent)
    : QMainWindow(parent),
      previewHtmlDock(new QDockWidget("Preview HTML")),
      previewHtml(new FolderPreviewHTML),
      previewWidget(new FolderPreviewWidget),
      m_fsPanel(nullptr),
      m_dbPanel(nullptr),
      stackCentralWidget(new QStackedWidget(this)),
      _navigationToolBar(new NavigationToolBar),
      osm(new RibbonMenu),
      _statusBar(new CustomStatusBar),
      m_jsonEditor(new JsonEditor(this)) {
  const QString& initialPath = (argc > 1) ? argv[1] : "";
  const QString& defaultPath = ReadSettings(initialPath);

  m_fsPanel = new ContentPanel(nullptr, defaultPath, previewHtml, previewWidget, _statusBar);
  m_dbPanel = new DatabasePanel;

  stackCentralWidget->addWidget(m_fsPanel);
  stackCentralWidget->addWidget(m_dbPanel);

  this->setCentralWidget(stackCentralWidget);

  //    previewHtmlDock->setWidget(previewHtml);
  previewHtmlDock->setWidget(previewWidget);
  previewHtmlDock->setAllowedAreas(Qt::DockWidgetArea::LeftDockWidgetArea | Qt::DockWidgetArea::RightDockWidgetArea);

  addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, previewHtmlDock);
  addToolBar(Qt::ToolBarArea::LeftToolBarArea, _navigationToolBar);
  setMenuWidget(osm);
  setStatusBar(_statusBar);
  subscribe();

  SwitchStackWidget();
  InitComponentVisibility();
}

FileExplorerReadOnly::~FileExplorerReadOnly() {}

void FileExplorerReadOnly::closeEvent(QCloseEvent* event) {
  PreferenceSettings().setValue("geometry", saveGeometry());
  PreferenceSettings().setValue("dockerWidgetWidth", previewWidget->width());
  PreferenceSettings().setValue("dockerWidgetHeight", previewWidget->height());
  PreferenceSettings().setValue("defaultOpenPath", m_fsPanel->CurrentPath());
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
  if (_navigationToolBar and m_fsPanel) {
    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;
    auto intoNewPath = std::bind(&ContentPanel::IntoNewPath, m_fsPanel, _1, _2, _3);
    _navigationToolBar->subscribe(intoNewPath);
  }
}

void FileExplorerReadOnly::SwitchStackWidget() {
  const bool showDB = PreferenceSettings().value(MemoryKey::SHOW_DATABASE.name, MemoryKey::SHOW_DATABASE.v).toBool();
  if (showDB) {
    stackCentralWidget->setCurrentWidget(m_dbPanel);
  } else {
    stackCentralWidget->setCurrentWidget(m_fsPanel);
  }
}

void FileExplorerReadOnly::InitComponentVisibility() {
  if (not PreferenceSettings().value(MemoryKey::SHOW_QUICK_NAVIGATION_TOOL_BAR.name, MemoryKey::SHOW_QUICK_NAVIGATION_TOOL_BAR.v).toBool()) {
    _navigationToolBar->setVisible(false);
  }

  const bool showDB = PreferenceSettings().value(MemoryKey::SHOW_DATABASE.name, MemoryKey::SHOW_DATABASE.v).toBool();
  if (showDB or not PreferenceSettings().value(MemoryKey::SHOW_FOLDER_PREVIEW_HTML.name, MemoryKey::SHOW_FOLDER_PREVIEW_HTML.v).toBool()) {
    previewHtmlDock->setVisible(false);
  }

  // floating out window
  m_jsonEditor->setVisible(
      PreferenceSettings().value(MemoryKey::SHOW_FOLDER_PREVIEW_JSON_EDITOR.name, MemoryKey::SHOW_FOLDER_PREVIEW_JSON_EDITOR.v).toBool());
}

void FileExplorerReadOnly::UpdateComponentVisibility() {
  const auto b1 = PreferenceSettings().value(MemoryKey::SHOW_QUICK_NAVIGATION_TOOL_BAR.name, MemoryKey::SHOW_QUICK_NAVIGATION_TOOL_BAR.v).toBool();
  if (_navigationToolBar->isVisible() != b1) {
    _navigationToolBar->setVisible(b1);
  }

  const bool showDB = PreferenceSettings().value(MemoryKey::SHOW_DATABASE.name, MemoryKey::SHOW_DATABASE.v).toBool();
  const auto b2 = PreferenceSettings().value(MemoryKey::SHOW_FOLDER_PREVIEW_HTML.name, MemoryKey::SHOW_FOLDER_PREVIEW_HTML.v).toBool();
  const bool shouldShow = not showDB and b2;
  if (previewHtmlDock->isVisible() != shouldShow) {
    previewHtmlDock->setVisible(b2);
  }

  const auto b3 = PreferenceSettings().value(MemoryKey::SHOW_FOLDER_PREVIEW_JSON_EDITOR.name, MemoryKey::SHOW_FOLDER_PREVIEW_JSON_EDITOR.v).toBool();
  if (m_jsonEditor->isVisible() != b3) {
    m_jsonEditor->setVisible(b3);
  }
}
