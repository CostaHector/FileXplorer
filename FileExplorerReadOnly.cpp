#include "FileExplorerReadOnly.h"

#include <QString>

#include <QDockWidget>
#include <QFileInfo>
#include <functional>

#include "Actions/FolderPreviewActions.h"
#include "Actions/ViewActions.h"
#include "PublicVariable.h"

#include "Tools/FolderPreviewSwitcher.h"
#include "Tools/NavigationViewSwitcher.h"

FileExplorerReadOnly::FileExplorerReadOnly(const int argc, char const* const argv[], QWidget* parent)
    : QMainWindow(parent),
      previewHtmlDock(new QDockWidget("Preview", this)),

      m_previewFolder{new PreviewFolder{previewHtmlDock}},
      m_previewSwitcher{new FolderPreviewSwitcher{m_previewFolder, previewHtmlDock}},

      m_fsPanel{nullptr},
      m_stackedBar{new StackedToolBar},
      m_naviSwitcher{nullptr},

      m_viewsSwitcher{new QToolBar("views switch", this)},
      m_navigationToolBar(new NavigationToolBar),
      m_ribbonMenu(new RibbonMenu{this}),

      m_statusBar(new CustomStatusBar{m_viewsSwitcher, this}) {
  m_viewsSwitcher->addActions(g_viewActions()._TRIPLE_VIEW->actions());

  m_fsPanel = new ContentPanel(m_previewFolder, this);
  m_fsPanel->BindLogger(m_statusBar);

  m_naviSwitcher = new NavigationViewSwitcher{m_stackedBar, m_fsPanel};
  m_naviSwitcher->onSwitchByViewType("table");

  qDebug("FileExplorerReadOnly Current path [%s]", qPrintable(QFileInfo(".").absoluteFilePath()));
  QString initialPath = (argc > 1) ? argv[1] : "";
  const QString& defaultPath = ReadSettings(initialPath);
  m_fsPanel->onActionAndViewNavigate(defaultPath, true);

  setCentralWidget(m_fsPanel);

  previewHtmlDock->setWidget(m_previewFolder);
  previewHtmlDock->setAllowedAreas(Qt::DockWidgetArea::LeftDockWidgetArea | Qt::DockWidgetArea::RightDockWidgetArea);
  addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, previewHtmlDock);

  addToolBar(Qt::ToolBarArea::TopToolBarArea, m_stackedBar);
  addToolBar(Qt::ToolBarArea::LeftToolBarArea, m_navigationToolBar);
  setMenuWidget(m_ribbonMenu);
  setStatusBar(m_statusBar);

  InitComponentVisibility();
  subscribe();
}

void FileExplorerReadOnly::closeEvent(QCloseEvent* event) {
  PreferenceSettings().setValue("geometry", saveGeometry());
  PreferenceSettings().setValue("dockerFolderPreviewWidth", m_previewFolder->width());
  PreferenceSettings().setValue("dockerFolderPreviewHeight", m_previewFolder->height());
  PreferenceSettings().setValue(MemoryKey::DEFAULT_OPEN_PATH.name, m_fsPanel->m_fsModel->rootPath());
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
    openPath = PreferenceSettings().value(MemoryKey::DEFAULT_OPEN_PATH.name, MemoryKey::DEFAULT_OPEN_PATH.v).toString();
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

  const bool showFolderPrev = PreferenceSettings().value(MemoryKey::SHOW_FOLDER_PREVIEW_HTML.name, MemoryKey::SHOW_FOLDER_PREVIEW_HTML.v).toBool();
  const bool showPrev = m_fsPanel->isFSView() and showFolderPrev;
  if (not showPrev) {
    previewHtmlDock->setVisible(false);
  }
}

void FileExplorerReadOnly::subscribe() {
  auto& vA = g_viewActions();
  connect(vA.NAVIGATION_PANE, &QAction::triggered, this, [this](const bool checked) {
    PreferenceSettings().setValue(MemoryKey::SHOW_QUICK_NAVIGATION_TOOL_BAR.name, checked);
    m_navigationToolBar->setVisible(checked);
  });

  connect(vA.PREVIEW_PANE_HTML, &QAction::triggered, this, [this](const bool checked) {
    PreferenceSettings().setValue(MemoryKey::SHOW_FOLDER_PREVIEW_HTML.name, checked);
    const bool showPrev = m_fsPanel->isFSView() and checked;
    previewHtmlDock->setVisible(showPrev);
  });
  auto& fpAG = g_folderPreviewActions();
  connect(fpAG.PREVIEW_AG, &QActionGroup::triggered, this, [this](QAction* triggeredActions) {
    if (triggeredActions == nullptr)
      return;
    const QString& previewType = triggeredActions->text();
    PreferenceSettings().setValue(MemoryKey::FOLDER_PREVIEW_TYPE.name, previewType);
    m_previewSwitcher->onSwitchByViewType(previewType);
  });
  connect(m_viewsSwitcher, &QToolBar::actionTriggered, m_naviSwitcher, &NavigationViewSwitcher::onSwitchByViewAction);
}

void FileExplorerReadOnly::keyPressEvent(QKeyEvent* ev) {
  if (ev->key() == Qt::Key_F3) {  // F3 Search
    if (m_fsPanel->isFSView()) {
      if (m_fsPanel->_addressBar != nullptr)
        m_fsPanel->_addressBar->onGetFocus();
    } else if (m_fsPanel->GetCurViewName() == "search") {
      if (m_fsPanel->_advanceSearchBar != nullptr) {
        m_fsPanel->_advanceSearchBar->onGetFocus();
      }
    }
    return;
  } else if (ev->key() == Qt::Key_Escape) {
    m_fsPanel->GetCurView()->setFocus();
    return;
  }
  QMainWindow::keyPressEvent(ev);
}
