#include "FileExplorerReadOnly.h"

#include "Actions/FolderPreviewActions.h"
#include "Actions/ViewActions.h"

#include "public/PublicVariable.h"
#include "public/MemoryKey.h"

#include "Tools/FolderPreviewSwitcher.h"
#include "Tools/NavigationViewSwitcher.h"
#include "Tools/ViewTypeTool.h"

#include <QString>
#include <QDockWidget>
#include <QFileInfo>
#include <functional>

FileExplorerReadOnly::FileExplorerReadOnly(const int argc, char const* const argv[], QWidget* parent)
  : QMainWindow(parent),
    previewHtmlDock(new QDockWidget("Preview", this)),

    m_previewFolder{new PreviewFolder{previewHtmlDock}},
    m_previewSwitcher{new FolderPreviewSwitcher{m_previewFolder, previewHtmlDock}},

    m_fsPanel{nullptr},
    m_stackedBar{new StackedToolBar},
    m_naviSwitcher{nullptr},

    m_viewsSwitcher{g_viewActions().GetViewTB()},
    m_navigationToolBar(new NavigationToolBar),
    m_ribbonMenu(new RibbonMenu{this}),

    m_statusBar(new CustomStatusBar{m_viewsSwitcher, this}) {

  m_fsPanel = new ContentPanel(m_previewFolder, this);
  m_fsPanel->BindLogger(m_statusBar);

  m_naviSwitcher = new NavigationViewSwitcher{m_stackedBar, m_fsPanel};

  m_naviSwitcher->onSwitchByViewType(ViewTypeTool::ViewType::TABLE);

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
  setWindowIcon(QIcon(":img/APP_ICON_PATH"));
  qDebug("File Explorer read preference settings from[%s].", qPrintable(openPath));
  return openPath;
}

void FileExplorerReadOnly::InitComponentVisibility() {
  const bool showNavi {PreferenceSettings().value(MemoryKey::SHOW_QUICK_NAVIGATION_TOOL_BAR.name, MemoryKey::SHOW_QUICK_NAVIGATION_TOOL_BAR.v).toBool()};
  if (!showNavi) {
    m_navigationToolBar->setVisible(false);
  }

  const bool showFolderPrev {PreferenceSettings().value(MemoryKey::SHOW_FOLDER_PREVIEW_HTML.name, MemoryKey::SHOW_FOLDER_PREVIEW_HTML.v).toBool()};
  const bool showPrev {m_fsPanel->isFSView() && showFolderPrev};
  if (!showPrev) {
    previewHtmlDock->setVisible(false);
  }
}

void FileExplorerReadOnly::subscribe() {
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
    PreferenceSettings().setValue(MemoryKey::FOLDER_PREVIEW_TYPE.name, previewType);
    m_previewSwitcher->onSwitchByViewType(previewType);
  });
  connect(vA._VIEWS_AG, &QActionGroup::triggered, m_naviSwitcher, &NavigationViewSwitcher::onSwitchByViewAction);
}

void FileExplorerReadOnly::keyPressEvent(QKeyEvent* ev) {
  const auto ky = ev->key();
  if (ky == Qt::Key_F3 || (ev->modifiers() == Qt::KeyboardModifier::ControlModifier && ky == Qt::Key_F)) {
    // F3/Ctrl+F Search
    const auto vt = m_fsPanel->GetCurViewType();
    if (ViewTypeTool::isFSView(vt)) {
      if (m_fsPanel->_addressBar != nullptr) {
        m_fsPanel->_addressBar->onGetFocus();
      }
      return;
    } else if (vt == ViewTypeTool::ViewType::SEARCH) {
      if (m_fsPanel->_advanceSearchBar != nullptr) {
        m_fsPanel->_advanceSearchBar->onGetFocus();
      }
      return;
    }
    return;
  } else if (ky == Qt::Key_Escape) {
    m_fsPanel->GetCurView()->setFocus();
    return;
  }
  QMainWindow::keyPressEvent(ev);
}
