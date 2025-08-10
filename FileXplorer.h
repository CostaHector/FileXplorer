#ifndef FILEXPLORER_H
#define FILEXPLORER_H

#include <QMainWindow>

#include "CustomStatusBar.h"
#include "NavigationToolBar.h"
#include "PreviewFolder.h"
#include "StackedToolBar.h"

#include "ContentPanel.h"
#include "RibbonMenu.h"

class FileXplorer : public QMainWindow {
 public:
  FileXplorer(const int argc = 1, char const* const argv[] = nullptr, QWidget* parent = nullptr);
  ~FileXplorer() = default;
  void closeEvent(QCloseEvent* event) override;
  void showEvent(QShowEvent *event) override;
  static QString ReadSettings(const int argc, char const* const argv[]);
  void RestoreWindowStateAndSetupUI();
  void InitComponentVisibility();
  void subscribe();

  void keyPressEvent(QKeyEvent* ev) override;

  QDockWidget* previewHtmlDock{nullptr};

  PreviewFolder* m_previewFolder{nullptr};
  FolderPreviewSwitcher* m_previewSwitcher{nullptr};

  ContentPanel* m_fsPanel{nullptr};
  StackedToolBar* m_stackedBar{nullptr};

  NavigationViewSwitcher* m_naviSwitcher{nullptr};
  QToolBar* m_viewsSwitcher{nullptr};

  NavigationToolBar* m_navigationToolBar{nullptr};
  RibbonMenu* m_ribbonMenu{nullptr};
  CustomStatusBar* m_statusBar{nullptr};
};
#endif  // FILEXPLORER_H
