#ifndef FILEEXPLORERREADONLY_H
#define FILEEXPLORERREADONLY_H

#include <QFileSystemModel>
#include <QMainWindow>
#include <QRect>
#include <QSettings>
#include <QStackedWidget>

#include "Component/CustomStatusBar.h"
#include "Component/NavigationToolBar.h"
#include "Component/PreviewFolder.h"
#include "Component/StackedToolBar.h"

#include "ContentPanel.h"
#include "RibbonMenu.h"

class FileExplorerReadOnly : public QMainWindow {
  Q_OBJECT

 public:
  FileExplorerReadOnly(const int argc = 1, char const* const argv[] = nullptr, QWidget* parent = nullptr);
  ~FileExplorerReadOnly() = default;
  virtual void closeEvent(QCloseEvent* event) override;

  auto ReadSettings(const QString& initialPath) -> QString;
  void InitComponentVisibility();
  void subscribe();

  void keyPressEvent(QKeyEvent* ev) override;

  QDockWidget* previewHtmlDock;

  PreviewFolder* m_previewFolder;
  FolderPreviewSwitcher* m_previewSwitcher;

  ContentPanel* m_fsPanel;
  StackedToolBar* m_stackedBar;

  NavigationViewSwitcher* m_naviSwitcher;
  QToolBar* m_viewsSwitcher;

  NavigationToolBar* m_navigationToolBar;
  RibbonMenu* m_ribbonMenu;
  CustomStatusBar* m_statusBar;
};
#endif  // FILEEXPLORERREADONLY_H
