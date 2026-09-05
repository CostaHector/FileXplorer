#include "FileXplorer.h"
#include "FolderPreviewSwitcher.h"
#include "ViewSwitchHelper.h"
#include "TagsHelper.h"
#include "ViewTypeTool.h"
#include "ViewActions.h"

#include "FileOpActs.h"
#include "PathKey.h"
#include "CompoVisKey.h"
#include "BehaviorKey.h"
#include "SizeTool.h"
#include "Configuration.h"
#include "CustomStatusBar.h"
#include "RatingStarsWidget.h"
#include "ScenePageControl.h"
#include "TagEditorSideBar.h"

#include "StyleSheet.h"
#include <QFileInfo>

void FileXplorer::initGlobalInstance() {
  auto* m_volumeWid = new VolumeWidget{QBoxLayout::Direction::LeftToRight, this};
  VolumeWidget::GInstance(m_volumeWid, true);

  auto* m_statusBar = new (std::nothrow) CustomStatusBar{this}; // status bar
  CustomStatusBar::GInstance(m_statusBar, true);

  auto* mRatingStarsWid = new (std::nothrow) RatingStarsWidget(this);
  RatingStarsWidget::GInstance(mRatingStarsWid, true);

  auto* m_scenePageControl = new (std::nothrow) ScenePageControl{"PaginationControl", this};
  ScenePageControl::GInstance(m_scenePageControl, true);

  auto* m_tagEditorSideBar = new (std::nothrow) TagEditorSideBar{"TagEditor SideBar", this};
  TagEditorSideBar::GInstance(m_tagEditorSideBar, true);
}

void FileXplorer::releaseGlobalInstance() {
  VolumeWidget::GInstance(nullptr, true);
  CustomStatusBar::GInstance(nullptr, true);
  RatingStarsWidget::GInstance(nullptr, true);
  ScenePageControl::GInstance(nullptr, true);
  TagEditorSideBar::GInstance(nullptr, true);
}

FileXplorer::~FileXplorer() {
  releaseGlobalInstance();
}

FileXplorer::FileXplorer(const QStringList& args, QWidget* parent) //
  : QMainWindow(parent)                                            //
{
  // 1. new widget compenent begin
  initGlobalInstance();

  m_previewFolder = new (std::nothrow) CurrentRowPreviewer{this}; // previewer in docker

  m_previewSwitcher = new (std::nothrow) FolderPreviewSwitcher{m_previewFolder, this};       // previewer switcher
  m_stackedBar = new (std::nothrow) StackedAddressAndSearchToolBar{"AddressToolbar", this}; // searchToolBar
  m_navigationToolBar = new (std::nothrow) NavigationToolBar{"NavigationToolBar", this};     // left navigation bar
  m_ribbonMenu = new (std::nothrow) RibbonMenu{this};                                        // ribbon menu

  m_fsPanel = new (std::nothrow) ViewsStackedWidget{m_previewFolder, this}; // main widget

  m_viewSwitchHelper = new (std::nothrow) ViewSwitchHelper{m_stackedBar, m_fsPanel, m_navigationToolBar, this}; // view/searchToolBar switcher

  {
    m_naviSideBarDock = new (std::nothrow) QDockWidget{"Navigation Sidebar", this};
    auto pNaviTitle = new QLabel{"Navi", this};
    pNaviTitle->setFixedHeight(12);
    m_naviSideBarDock->setTitleBarWidget(pNaviTitle);
    m_naviSideBarDock->setWidget(m_navigationToolBar);
    m_naviSideBarDock->setAllowedAreas(Qt::DockWidgetArea::LeftDockWidgetArea);
    addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, m_naviSideBarDock);
  }
  {
    m_previewHtmlDock = new (std::nothrow) PreviewDockWidget{"PreviewDockWidget", this}; // docker
    m_previewHtmlDock->setWidget(m_previewFolder);
    m_previewHtmlDock->setAllowedAreas(Qt::DockWidgetArea::LeftDockWidgetArea | Qt::DockWidgetArea::RightDockWidgetArea);
    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, m_previewHtmlDock);


  }
  // 1. new widget component end
  InitComponentVisibility();

  // 2. set the initial value begin
  {
    ViewTypeTool::ViewType initialViewType = ViewTypeTool::DEFAULT_VIEW_TYPE;
    if (auto* pStatusBar = CustomStatusBar::GInstance()) {
      initialViewType = pStatusBar->_GetCurViewType();
    }
    m_viewSwitchHelper->onSwitchByViewType(initialViewType);

    const QString& defaultPath = GetInitialPathFromArgs(args);
    m_fsPanel->onActionAndViewNavigate(defaultPath, true);

    const PreviewTypeTool::PREVIEW_TYPE_E initialPreviewType{m_previewHtmlDock->GetCurrentPreviewType()};
    m_previewSwitcher->onSwitchByPreviewType(initialPreviewType);
  }
  // 2. set the initial value end

  // 3. place component begin
  setMenuWidget(m_ribbonMenu);
  if (auto* pTagEditorSideBar = TagEditorSideBar::GInstance()) {
    addToolBar(Qt::ToolBarArea::RightToolBarArea, pTagEditorSideBar);
  }
  addToolBar(Qt::ToolBarArea::TopToolBarArea, m_stackedBar);
  // 3. place component end
  setCentralWidget(m_fsPanel);
  if (auto* pStatusBar = CustomStatusBar::GInstance()) {
    setStatusBar(pStatusBar);
  }

  // 4. subscribe and ui size location setting
  subscribe();
  RestoreWindowStateAndSetupUI();
}

void FileXplorer::closeEvent(QCloseEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  Configuration().setValue("FileXplorer/Geometry", saveGeometry());
  setConfig(PathKey::STARTUP_PATH, m_fsPanel->m_fsModel->rootPath());
  setConfig(CompoVisKey::SHOW_NAVIGATION_SIDEBAR, ViewActions::GetInst()._NAVIGATION_PANE->isChecked());
  setConfig(CompoVisKey::SHOW_PREVIEW_DOCKER, ViewActions::GetInst()._PREVIEW_PANEL->isChecked());
  setConfig(CompoVisKey::SHOW_TAG_EDITOR_SIDEBAR, ViewActions::GetInst()._TAG_EDITOR_SIDEBAR->isChecked());
  setConfig(BehaviorKey::FILESYSTEM_STRUCTURE, (int)FileOpActs::GetInst().GetCurFileStructurePolicy());

  m_previewFolder->saveSizeHint();
  QMainWindow::closeEvent(event);
}

void FileXplorer::showEvent(QShowEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  StyleSheet::UpdateTitleBar(this);
  QMainWindow::showEvent(event);
}

QString FileXplorer::GetInitialPathFromArgs(const QStringList& args) {
  LOG_I(R"(Program:[FileXplorer] running with given args["%s"])", qPrintable(args.join(R"(",")")));
  // executing the program with or without command-line arguments
  const bool bIsSpecifiedPath{args.size() > 1};
  QString path{bIsSpecifiedPath ? args[1] : ""};
#ifdef _WIN32
  if (path.endsWith(":\"")) { // e.g. "E:\"" => "E:/"
    path.back() = '/';
  }
#endif
  // when not specified or specied path is invalid, use last time path in preference setting
  if (!bIsSpecifiedPath || (!path.isEmpty() && !QFile::exists(path))) {
    QString lastTimePath = getConfig(PathKey::STARTUP_PATH).toString();
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
  if (Configuration().contains("FileXplorer/Geometry")) {
    restoreGeometry(Configuration().value("FileXplorer/Geometry").toByteArray());
  } else {
    setGeometry(SizeTool::DEFAULT_GEOMETRY);
  }
  setWindowTitle("FileXplorer");
  setWindowIcon(QIcon(":img/APP_ICON_PATH"));
}

void FileXplorer::InitComponentVisibility() {
  const bool showNavi{getConfig(CompoVisKey::SHOW_NAVIGATION_SIDEBAR).toBool()};
  if (!showNavi) {
    m_naviSideBarDock->setHidden(true);
  }

  const bool showFolderPreview = getConfig(CompoVisKey::SHOW_PREVIEW_DOCKER).toBool();
  if (!showFolderPreview) {
    m_previewHtmlDock->setHidden(true);
  }

  if (auto *pTagEditorSideBar = TagEditorSideBar::GInstance()) {
    const bool showTagEditorSideBar = getConfig(CompoVisKey::SHOW_TAG_EDITOR_SIDEBAR).toBool();
    if (!showTagEditorSideBar) {
      pTagEditorSideBar->setHidden(true);
    }
    connect(ViewActions::GetInst()._TAG_EDITOR_SIDEBAR, &QAction::toggled, pTagEditorSideBar, &QToolBar::setVisible);
  }
}

void FileXplorer::subscribe() {
  auto& vA = ViewActions::GetInst();
  connect(vA._NAVIGATION_PANE, &QAction::toggled, m_naviSideBarDock, &QWidget::setVisible);
  connect(vA._PREVIEW_PANEL, &QAction::toggled, m_previewHtmlDock, &PreviewDockWidget::setVisible);
  connect(m_previewHtmlDock, &PreviewDockWidget::previewTypeChanged, m_previewSwitcher, &FolderPreviewSwitcher::onSwitchByPreviewType);

  if (auto* pStatusBar = CustomStatusBar::GInstance()) {
    connect(pStatusBar, &CustomStatusBar::_viewTypeChanged, this, &FileXplorer::onViewWidgetChanged);
  }
  // connect(m_previewFolder, &CurrentRowPreviewer::reqWindowsTitleChange, m_previewHtmlDock, &PreviewDockWidget::onWindowsTitleChanged);
}

void FileXplorer::keyPressEvent(QKeyEvent* ev) {
  CHECK_NULLPTR_RETURN_VOID(ev);
  switch (ev->key()) {
    case Qt::Key_F3: { // F3 Search
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