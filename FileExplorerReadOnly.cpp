#include "FileExplorerReadOnly.h"

#include <QString>

#include <QDockWidget>
#include <QFileInfo>
#include <functional>

#include "Actions/ViewActions.h"
#include "Component/FolderPreviewHTML.h"
#include "PublicVariable.h"

const QString FileExplorerReadOnly::DEFAULT_PATH = "";

FileExplorerReadOnly::FileExplorerReadOnly(const int argc, char const* const argv[], QWidget* parent)
    : QMainWindow(parent),
      previewHtmlDock(new QDockWidget("Preview HTML", this)),
      previewHtml(new FolderPreviewHTML(previewHtmlDock)),
      //      previewWidget(new FolderPreviewWidget),
      previewWidget{nullptr},

      m_fsPanel{nullptr},
      m_stackedBar{new StackedToolBar},
      m_viewSwitcher{nullptr},

      m_views{new QToolBar("views switch", this)},
      m_navigationToolBar(new NavigationToolBar),
      osm(new RibbonMenu),

      _statusBar(new CustomStatusBar{m_views, this}) {
  m_views->addActions(g_viewActions()._TRIPLE_VIEW->actions());

  m_fsPanel = new ContentPanel(previewHtml, nullptr, this);
  m_fsPanel->BindCustomStatusBar(_statusBar);

  m_viewSwitcher = new NavigationViewSwitcher{m_stackedBar, m_fsPanel};
  m_viewSwitcher->onSwitchByViewType("table");

  qDebug("FileExplorerReadOnly Current path [%s]", qPrintable(QFileInfo(".").absoluteFilePath()));
  QString initialPath = (argc > 1) ? argv[1] : "";
  const QString& defaultPath = ReadSettings(initialPath);
  m_fsPanel->onActionAndViewNavigate(defaultPath, true);

  setCentralWidget(m_fsPanel);

  previewHtmlDock->setWidget(previewHtml);
  //  previewHtmlDock->setWidget(previewWidget);
  previewHtmlDock->setAllowedAreas(Qt::DockWidgetArea::LeftDockWidgetArea | Qt::DockWidgetArea::RightDockWidgetArea);
  addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, previewHtmlDock);

  addToolBar(Qt::ToolBarArea::TopToolBarArea, m_stackedBar);
  addToolBar(Qt::ToolBarArea::LeftToolBarArea, m_navigationToolBar);
  setMenuWidget(osm);
  setStatusBar(_statusBar);

  InitComponentVisibility();

  connect(m_views, &QToolBar::actionTriggered, m_viewSwitcher, &NavigationViewSwitcher::onSwitchByViewAction);
}

FileExplorerReadOnly::~FileExplorerReadOnly() {}

void FileExplorerReadOnly::closeEvent(QCloseEvent* event) {
  PreferenceSettings().setValue("geometry", saveGeometry());
  if (previewWidget) {
    PreferenceSettings().setValue("dockerWidgetWidth", previewWidget->width());
    PreferenceSettings().setValue("dockerWidgetHeight", previewWidget->height());
  }
  if (previewHtml) {
    PreferenceSettings().setValue("dockerHtmlWidth", previewHtml->width());
    PreferenceSettings().setValue("dockerHtmlHeight", previewHtml->height());
  }
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
  qDebug("File Explorer read preference settings from[%s].", qPrintable(openPath));
  return openPath;
}

void FileExplorerReadOnly::InitComponentVisibility() {
  const bool showNavi =
      PreferenceSettings().value(MemoryKey::SHOW_QUICK_NAVIGATION_TOOL_BAR.name, MemoryKey::SHOW_QUICK_NAVIGATION_TOOL_BAR.v).toBool();
  if (not showNavi) {
    m_navigationToolBar->setVisible(false);
  }

  const bool showDB = PreferenceSettings().value(MemoryKey::SHOW_DATABASE.name, MemoryKey::SHOW_DATABASE.v).toBool();
  const bool showFolderPrev = PreferenceSettings().value(MemoryKey::SHOW_FOLDER_PREVIEW_HTML.name, MemoryKey::SHOW_FOLDER_PREVIEW_HTML.v).toBool();
  const bool showPrev = not showDB and showFolderPrev;
  if (not showPrev) {
    previewHtmlDock->setVisible(false);
  }
}

void FileExplorerReadOnly::UpdateComponentVisibility() {
  const bool showNavi =
      PreferenceSettings().value(MemoryKey::SHOW_QUICK_NAVIGATION_TOOL_BAR.name, MemoryKey::SHOW_QUICK_NAVIGATION_TOOL_BAR.v).toBool();
  m_navigationToolBar->setVisible(showNavi);

  const bool showDB = PreferenceSettings().value(MemoryKey::SHOW_DATABASE.name, MemoryKey::SHOW_DATABASE.v).toBool();
  const bool showFolderPrev = PreferenceSettings().value(MemoryKey::SHOW_FOLDER_PREVIEW_HTML.name, MemoryKey::SHOW_FOLDER_PREVIEW_HTML.v).toBool();
  const bool showPrev = not showDB and showFolderPrev;
  previewHtmlDock->setVisible(showPrev);
}
