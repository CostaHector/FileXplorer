#include "FileXplorer.h"
#include "FolderPreviewSwitcher.h"
#include "ViewSwitchHelper.h"
#include "ViewTypeTool.h"
#include "ViewActions.h"
#include "MemoryKey.h"
#include "StyleSheet.h"
#include <QFileInfo>

FileXplorer::FileXplorer(const QStringList& args, QWidget* parent)  //
    : QMainWindow(parent)                                           //
{
  m_viewSwitcher = new (std::nothrow) ViewSwitchToolBar{"ViewSwitcherToolBar", this};
  m_previewHtmlDock = new (std::nothrow) PreviewDockWidget{"PreviewDockWidget", this};  // docker
  m_scenePageControl = new (std::nothrow) ScenePageControl{"PaginationControl", this};
  const ViewTypeTool::ViewType initialViewType{m_viewSwitcher->GetCurViewType()};

  m_previewFolder = new (std::nothrow) CurrentRowPreviewer{this};  // previewer in docker

  m_previewSwitcher = new (std::nothrow) FolderPreviewSwitcher{m_previewFolder, this};        // previewer switcher
  m_stackedBar = new (std::nothrow) StackedAddressAndSearchToolBar{"Stacked Toolbar", this};  // searchToolBar
  m_navigationToolBar = new (std::nothrow) NavigationToolBar{"NavigationToolBar", this};      // left navigation bar
  m_ribbonMenu = new (std::nothrow) RibbonMenu{this};                                         // ribbon menu

  m_statusBar = new (std::nothrow) CustomStatusBar{this};  // status bar

  m_fsPanel = new (std::nothrow) ViewsStackedWidget{m_previewFolder, this};  // main widget
  m_fsPanel->BindLogger(m_statusBar);
  m_statusBar->addViewSwitcherToRightCorner(m_viewSwitcher);

  m_viewSwitchHelper = new (std::nothrow) ViewSwitchHelper{m_stackedBar, m_fsPanel, m_scenePageControl, m_navigationToolBar, this};  // view/searchToolBar switcher
  m_viewSwitchHelper->onSwitchByViewType(initialViewType);

  const QString& defaultPath = GetInitialPathFromArgs(args);
  m_fsPanel->onActionAndViewNavigate(defaultPath, true);

  setCentralWidget(m_fsPanel);

  m_ribbonMenu->AddScenePageControlWidget(m_scenePageControl);

  m_previewHtmlDock->setWidget(m_previewFolder);
  m_previewHtmlDock->setAllowedAreas(Qt::DockWidgetArea::LeftDockWidgetArea | Qt::DockWidgetArea::RightDockWidgetArea);
  addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, m_previewHtmlDock);

  addToolBar(Qt::ToolBarArea::TopToolBarArea, m_stackedBar);
  addToolBar(Qt::ToolBarArea::LeftToolBarArea, m_navigationToolBar);
  setMenuWidget(m_ribbonMenu);
  setStatusBar(m_statusBar);

  InitComponentVisibility();
  subscribe();

  RestoreWindowStateAndSetupUI();
}

void FileXplorer::closeEvent(QCloseEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  Configuration().setValue("Geometry/" CLASSNAME_2_STR(FileXplorer), saveGeometry());
  Configuration().setValue(PathKey::STARTUP_PATH.name, m_fsPanel->m_fsModel->rootPath());
  m_previewFolder->saveSizeHint();
  QMainWindow::closeEvent(event);
}

void FileXplorer::showEvent(QShowEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  StyleSheet::UpdateTitleBar(this);
  QMainWindow::showEvent(event);
}

QString FileXplorer::GetInitialPathFromArgs(const QStringList& args) {
  LOG_I("Program:[" PROJECT_NAME R"(] running with given args["%s"])", qPrintable(args.join(R"(",")")));
  // executing the program with or without command-line arguments
  const bool bIsSpecifiedPath{args.size() > 1};
  QString path{bIsSpecifiedPath ? args[1] : ""};
#ifdef _WIN32
  if (path.endsWith(":\"")) {  // e.g. "E:\"" => "E:/"
    path.back() = '/';
  }
#endif
  // when not specified or specied path is invalid, use last time path in preference setting
  if (!bIsSpecifiedPath || (!path.isEmpty() && !QFile::exists(path))) {
    QString lastTimePath = Configuration().value(PathKey::STARTUP_PATH.name, PathKey::STARTUP_PATH.v).toString();
    LOG_D("path[%s] not exists. use last time path[%s]", qPrintable(path), qPrintable(lastTimePath));
    path.swap(lastTimePath);
  }
  const QFileInfo fi{path};
  if (!path.isEmpty() && !fi.isDir()) {
    QString parentPath = fi.absolutePath();
    LOG_D("path[%s] not exists or is a file. Try using its parent path[%s] instead", qPrintable(path), qPrintable(parentPath));
    path.swap(parentPath);
  }
  LOG_D("Default path is %s.", qPrintable(path));
  return path;
}

void FileXplorer::RestoreWindowStateAndSetupUI() {
  if (Configuration().contains(PROJECT_NAME "/Geometry")) {
    restoreGeometry(Configuration().value(PROJECT_NAME "/Geometry").toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }
  setWindowTitle(PROJECT_NAME);
  setWindowIcon(QIcon(":img/APP_ICON_PATH"));
}

void FileXplorer::InitComponentVisibility() {
  const bool showNavi{Configuration().value(CompoVisKey::SHOW_NAVIGATION_SIDEBAR.name, CompoVisKey::SHOW_NAVIGATION_SIDEBAR.v).toBool()};
  if (!showNavi) {
    m_navigationToolBar->hide();
  }

  const bool showFolderPreview = Configuration().value(CompoVisKey::SHOW_PREVIEW_DOCKER.name, CompoVisKey::SHOW_PREVIEW_DOCKER.v).toBool();
  if (!showFolderPreview) {
    m_previewHtmlDock->setVisible(false);
  }

  const PreviewTypeTool::PREVIEW_TYPE_E initialPreviewType{m_previewHtmlDock->GetCurrentPreviewType()};
  m_previewSwitcher->onSwitchByPreviewType(initialPreviewType);
}

void FileXplorer::subscribe() {
  auto& vA = g_viewActions();
  connect(vA._NAVIGATION_PANE, &QAction::toggled, m_navigationToolBar, &QWidget::setVisible);
  connect(vA._PREVIEW_PANEL, &QAction::toggled, m_previewHtmlDock, &PreviewDockWidget::setVisible);

  connect(m_viewSwitcher, &ViewSwitchToolBar::viewTypeChanged, this, &FileXplorer::onViewWidgetChanged);

  connect(m_previewHtmlDock, &PreviewDockWidget::previewTypeChanged, m_previewSwitcher, &FolderPreviewSwitcher::onSwitchByPreviewType);

  connect(m_previewFolder, &CurrentRowPreviewer::reqWindowsTitleChange, m_previewHtmlDock, &PreviewDockWidget::onWindowsTitleChanged);
}

void FileXplorer::keyPressEvent(QKeyEvent* ev) {
  CHECK_NULLPTR_RETURN_VOID(ev);
  switch (ev->key()) {
    case Qt::Key_F3: {  // F3 Search
      const auto viewType = m_fsPanel->GetVt();
      switch (viewType) {
        case ViewTypeTool::ViewType::LIST:
        case ViewTypeTool::ViewType::TABLE:
        case ViewTypeTool::ViewType::TREE:
        case ViewTypeTool::ViewType::SCENE:
        case ViewTypeTool::ViewType::JSON: {
          if (m_fsPanel->_addressBar != nullptr) {
            m_fsPanel->_addressBar->onGetFocus();
          }
          break;
        }
        case ViewTypeTool::ViewType::SEARCH: {
          if (m_fsPanel->_advanceSearchBar != nullptr) {
            m_fsPanel->_advanceSearchBar->onGetFocus();
          }
          break;
        }
        case ViewTypeTool::ViewType::MOVIE: {
          if (m_fsPanel->_movieSearchBar != nullptr) {
            m_fsPanel->_movieSearchBar->onGetFocus();
          }
          break;
        }
        case ViewTypeTool::ViewType::CAST: {
          if (m_fsPanel->_castSearchBar != nullptr) {
            m_fsPanel->_castSearchBar->onGetFocus();
          }
          break;
        }
        default:
          break;
      }
      return;
    }
    case Qt::Key_Escape: {
      m_fsPanel->GetCurView()->setFocus();
    }
    default:
      break;
  }
  QMainWindow::keyPressEvent(ev);
}

void FileXplorer::onViewWidgetChanged(ViewTypeTool::ViewType viewType) {
  m_viewSwitchHelper->onSwitchByViewType(viewType);
  m_ribbonMenu->on_ViewTypeChanged(viewType);
}
