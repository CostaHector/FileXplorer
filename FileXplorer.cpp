#include "FileXplorer.h"

#include "FolderPreviewActions.h"
#include "ViewActions.h"

#include "MemoryKey.h"
#include "StyleSheet.h"

#include "FolderPreviewSwitcher.h"
#include "ToolBarAndViewSwitcher.h"
#include "ViewTypeTool.h"

#include <QString>
#include <QDockWidget>
#include <QFileInfo>

class DockWidget : public QDockWidget {
public:
  using QDockWidget::QDockWidget;
  void showEvent(QShowEvent* event) override {
    QDockWidget::showEvent(event);
    StyleSheet::UpdateTitleBar(this);
  }
};

FileXplorer::FileXplorer(const QStringList& args, QWidget* parent)  //
  : QMainWindow(parent)                                                            //
{
  previewHtmlDock = new (std::nothrow) DockWidget{"Preview", this}; //  docker
  m_previewFolder = new (std::nothrow) CurrentRowPreviewer{previewHtmlDock}; // previewer in docker
  m_previewSwitcher = new (std::nothrow) FolderPreviewSwitcher{m_previewFolder, this}; // previewer switcher
  m_stackedBar = new (std::nothrow) StackedAddressAndSearchToolBar; // searchToolBar
  m_viewsSwitcher = g_viewActions().GetViewTB(); // right-down corner permanent widget
  m_navigationToolBar = new (std::nothrow) NavigationToolBar; // left navigation bar
  m_ribbonMenu = new (std::nothrow) RibbonMenu{this}; // ribbon menu
  m_statusBar = new (std::nothrow) CustomStatusBar{m_viewsSwitcher, this}; // status bar
  m_fsPanel = new (std::nothrow) ViewsStackedWidget{m_previewFolder, this}; // main widget
  m_naviSwitcher = new (std::nothrow) ToolBarAndViewSwitcher{m_stackedBar, m_fsPanel}; // view/searchToolBar switcher

  m_fsPanel->BindLogger(m_statusBar);
  m_naviSwitcher->onSwitchByViewType(ViewTypeTool::ViewType::TABLE);

  const QString& defaultPath = ReadSettings(args);
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

  RestoreWindowStateAndSetupUI();
}

void FileXplorer::closeEvent(QCloseEvent* event) {
  Configuration().setValue(CLASSNAME_2_STR(FileXplorer) "_Geometry", saveGeometry());
  Configuration().setValue("SELECTION_PREVIEWER_WIDTH", m_previewFolder->width());
  Configuration().setValue("SELECTION_PREVIEWER_HEIGHT", m_previewFolder->height());
  Configuration().setValue(MemoryKey::DEFAULT_OPEN_PATH.name, m_fsPanel->m_fsModel->rootPath());
  return QMainWindow::closeEvent(event);
}

void FileXplorer::showEvent(QShowEvent* event) {
  QMainWindow::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

QString FileXplorer::ReadSettings(const QStringList& args) {
  qDebug("Program:'" PROJECT_NAME R"(' running with given args["%s"])", qPrintable(args.join(R"(",")")));
  // executing the program with or without command-line arguments
  QString path{(args.size() > 1) ? args[1] : ""};
  // when argv[1] path invalid, use last time path in preference setting
  if (!QFile::exists(path)) {
    QString lastTimePath = Configuration().value(MemoryKey::DEFAULT_OPEN_PATH.name, MemoryKey::DEFAULT_OPEN_PATH.v).toString();
    qDebug("path[%s] not exists. use last time path[%s]", qPrintable(path), qPrintable(lastTimePath));
    path.swap(lastTimePath);
  }
  const QFileInfo fi{path};
  if (!fi.isDir()) {
    QString parentPath = fi.absolutePath();
    qDebug("path[%s] not exists or is a file. Try using its parent path[%s] instead", qPrintable(path), qPrintable(parentPath));
    path.swap(parentPath);
  }
  qDebug("Default path is %s.", qPrintable(path));
  return path;
}

void FileXplorer::RestoreWindowStateAndSetupUI() {
  if (Configuration().contains(PROJECT_NAME "_Geometry")) {
    restoreGeometry(Configuration().value(PROJECT_NAME "_Geometry").toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }
  setWindowTitle(PROJECT_NAME);
  setWindowIcon(QIcon(":img/APP_ICON_PATH"));
}

void FileXplorer::InitComponentVisibility() {
  const bool showNavi{Configuration().value(MemoryKey::SHOW_QUICK_NAVIGATION_TOOL_BAR.name, MemoryKey::SHOW_QUICK_NAVIGATION_TOOL_BAR.v).toBool()};
  if (!showNavi) {
    m_navigationToolBar->setVisible(false);
  }
  const int folderPreviewType = Configuration().value(MemoryKey::FOLDER_PREVIEW_TYPE.name, MemoryKey::FOLDER_PREVIEW_TYPE.v).toInt();
  if (folderPreviewType == (int)PreviewTypeTool::PREVIEW_TYPE_E::NONE) {
    previewHtmlDock->setVisible(false);
  }
}

void FileXplorer::subscribe() {
  auto& vA = g_viewActions();
  connect(vA.NAVIGATION_PANE, &QAction::toggled, this, [this](const bool checked) {
    Configuration().setValue(MemoryKey::SHOW_QUICK_NAVIGATION_TOOL_BAR.name, checked);
    m_navigationToolBar->setVisible(checked);
  });
  connect(vA._VIEWS_AG, &QActionGroup::triggered, this, &FileXplorer::onViewTypeChanged);

  PreviewTypeToolBar* previewToolBar = g_folderPreviewActions().GetPreviewsToolbar(this);
  connect(previewToolBar, &PreviewTypeToolBar::PreviewTypeChange, this, &FileXplorer::onPreviewSwitched);

  connect(m_previewFolder, &QStackedWidget::windowTitleChanged, previewHtmlDock, &QDockWidget::setWindowTitle);
}

void FileXplorer::keyPressEvent(QKeyEvent* ev) {
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
        case ViewTypeTool::ViewType::SEARCH:{
          if (m_fsPanel->_advanceSearchBar != nullptr) {
            m_fsPanel->_advanceSearchBar->onGetFocus();
          }
          break;
        }
        case ViewTypeTool::ViewType::MOVIE:{
          if (m_fsPanel->_movieSearchBar != nullptr) {
            m_fsPanel->_movieSearchBar->onGetFocus();
          }
          break;
        }
        case ViewTypeTool::ViewType::CAST:{
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

void FileXplorer::onPreviewSwitched(PreviewTypeTool::PREVIEW_TYPE_E previewEnum) {
  if (previewEnum == PreviewTypeTool::PREVIEW_TYPE_E::NONE) {
    qWarning("Here should hide");
  }
  Configuration().setValue(MemoryKey::FOLDER_PREVIEW_TYPE.name, (int)previewEnum);
  previewHtmlDock->setVisible(previewEnum != PreviewTypeTool::PREVIEW_TYPE_E::NONE);
  m_previewSwitcher->onSwitchByViewType(previewEnum);
}

void FileXplorer::onViewTypeChanged(const QAction* pViewAct) {
  using namespace ViewTypeTool;
  ViewType vt = GetViewTypeByActionText(pViewAct);
  m_naviSwitcher->onSwitchByViewType(vt);
  m_ribbonMenu->whenViewTypeChanged(vt);
}
