#include "FileXplorer.h"

#include "Actions/FolderPreviewActions.h"
#include "Actions/ViewActions.h"

#include "public/MemoryKey.h"
#include "public/StyleSheet.h"

#include "Tools/FolderPreviewSwitcher.h"
#include "Tools/NavigationViewSwitcher.h"
#include "Tools/ViewTypeTool.h"

#include <QString>
#include <QDockWidget>
#include <QFileInfo>
#include <functional>

FileXplorer::FileXplorer(const int argc, char const* const argv[], QWidget* parent)  //
    : QMainWindow(parent)                                                            //
{
  previewHtmlDock = new (std::nothrow) QDockWidget("Preview", this);
  m_previewFolder = new (std::nothrow) PreviewFolder{previewHtmlDock};
  m_previewSwitcher = new (std::nothrow) FolderPreviewSwitcher{m_previewFolder, previewHtmlDock};
  m_stackedBar = new (std::nothrow) StackedToolBar;
  m_viewsSwitcher = g_viewActions().GetViewTB();
  m_navigationToolBar = new (std::nothrow) NavigationToolBar;
  m_ribbonMenu = new (std::nothrow) RibbonMenu{this};
  m_statusBar = new (std::nothrow) CustomStatusBar{m_viewsSwitcher, this};
  m_fsPanel = new (std::nothrow) ContentPanel(m_previewFolder, this);
  m_naviSwitcher = new (std::nothrow) NavigationViewSwitcher{m_stackedBar, m_fsPanel};

  m_fsPanel->BindLogger(m_statusBar);
  m_naviSwitcher->onSwitchByViewType(ViewTypeTool::ViewType::TABLE);

  const QString& defaultPath = ReadSettings(argc, argv);
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
  PreferenceSettings().setValue(CLASSNAME_2_STR(FileXplorer) "_Geometry", saveGeometry());
  PreferenceSettings().setValue(CLASSNAME_2_STR(FileXplorer) "_Docker_Width", m_previewFolder->width());
  PreferenceSettings().setValue(CLASSNAME_2_STR(FileXplorer) "_Docker_Height", m_previewFolder->height());
  PreferenceSettings().setValue(MemoryKey::DEFAULT_OPEN_PATH.name, m_fsPanel->m_fsModel->rootPath());
  return QMainWindow::closeEvent(event);
}

void FileXplorer::showEvent(QShowEvent* event) {
  QMainWindow::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

QString FileXplorer::ReadSettings(const int argc, char const* const argv[]) {
  qDebug("FileXplorer running in [%s]", qPrintable(QFileInfo(".").absoluteFilePath()));
  // executing the program with or without command-line arguments
  QString path{(argc > 1) ? argv[1] : ""};
  if (path.endsWith('"')) {
    path.chop(1);
  }
  // when argv[1] path invalid, use last time path in preference setting
  if (!QFile::exists(path)) {
    path = PreferenceSettings().value(MemoryKey::DEFAULT_OPEN_PATH.name, MemoryKey::DEFAULT_OPEN_PATH.v).toString();
  }
  const QFileInfo fi{path};
  if (!fi.isDir()) {
    path = fi.absolutePath();
  }
  qDebug("FileXplorer open path[%s].", qPrintable(path));
  return path;
}

void FileXplorer::RestoreWindowStateAndSetupUI() {
  if (PreferenceSettings().contains(CLASSNAME_2_STR(FileXplorer) "_Geometry")) {
    restoreGeometry(PreferenceSettings().value(CLASSNAME_2_STR(FileXplorer) "_Geometry").toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }
  setWindowTitle(CLASSNAME_2_STR(FileXplorer));
  setWindowIcon(QIcon(":img/APP_ICON_PATH"));
}

void FileXplorer::InitComponentVisibility() {
  const bool showNavi{PreferenceSettings().value(MemoryKey::SHOW_QUICK_NAVIGATION_TOOL_BAR.name, MemoryKey::SHOW_QUICK_NAVIGATION_TOOL_BAR.v).toBool()};
  if (!showNavi) {
    m_navigationToolBar->setVisible(false);
  }

  const bool showFolderPrev{PreferenceSettings().value(MemoryKey::SHOW_FOLDER_PREVIEW_HTML.name, MemoryKey::SHOW_FOLDER_PREVIEW_HTML.v).toBool()};
  const bool showPrev{m_fsPanel->isFSView() && showFolderPrev};
  if (!showPrev) {
    previewHtmlDock->setVisible(false);
  }
}

void FileXplorer::subscribe() {
  auto& vA = g_viewActions();
  connect(vA.NAVIGATION_PANE, &QAction::triggered, this, [this](const bool checked) {
    PreferenceSettings().setValue(MemoryKey::SHOW_QUICK_NAVIGATION_TOOL_BAR.name, checked);
    m_navigationToolBar->setVisible(checked);
  });

  auto& fpAG = g_folderPreviewActions();
  connect(fpAG.PREVIEW_AG, &QActionGroup::triggered, this, [this](QAction* triggeredActions) {
    if (triggeredActions == nullptr) {
      return;
    }
    const bool checked{triggeredActions->isChecked()};
    PreferenceSettings().setValue(MemoryKey::SHOW_FOLDER_PREVIEW_HTML.name, checked);
    previewHtmlDock->setVisible(checked);

    const QString& previewType = triggeredActions->text();
    if (!triggeredActions->isChecked()) {  // clear preview type
      PreferenceSettings().setValue(MemoryKey::FOLDER_PREVIEW_TYPE.name, "");
    } else {
      PreferenceSettings().setValue(MemoryKey::FOLDER_PREVIEW_TYPE.name, previewType);
    }
    m_previewSwitcher->onSwitchByViewType(previewType);
  });
  connect(vA._VIEWS_AG, &QActionGroup::triggered, m_naviSwitcher, &NavigationViewSwitcher::onSwitchByViewAction);
}

void FileXplorer::keyPressEvent(QKeyEvent* ev) {
  switch (ev->key()) {
    case Qt::Key_F3: {  // F3 Search
      const auto viewType = m_fsPanel->GetCurViewType();
      if (ViewTypeTool::IsUseFileSystemSearchBar(viewType)) {  // for filesytem view
        if (m_fsPanel->_addressBar != nullptr) {
          m_fsPanel->_addressBar->onGetFocus();
        }
      } else if (viewType == ViewTypeTool::ViewType::SEARCH) {
        if (m_fsPanel->_advanceSearchBar != nullptr) {
          m_fsPanel->_advanceSearchBar->onGetFocus();
        }
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
