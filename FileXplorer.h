#ifndef FILEXPLORER_H
#define FILEXPLORER_H

#include <QMainWindow>

#include "CustomStatusBar.h"
#include "NavigationToolBar.h"
#include "CurrentRowPreviewer.h"
#include "StackedAddressAndSearchToolBar.h"

#include "ViewsStackedWidget.h"
#include "RibbonMenu.h"

class FileXplorer : public QMainWindow {
public:
  FileXplorer(const QStringList& args, QWidget* parent = nullptr);
  ~FileXplorer() = default;
  void closeEvent(QCloseEvent* event) override;
  void showEvent(QShowEvent *event) override;
  static QString ReadSettings(const QStringList& args);
  void RestoreWindowStateAndSetupUI();
  void InitComponentVisibility();
  void subscribe();

  void keyPressEvent(QKeyEvent* ev) override;

  QDockWidget* previewHtmlDock{nullptr};

  CurrentRowPreviewer* m_previewFolder{nullptr};
  FolderPreviewSwitcher* m_previewSwitcher{nullptr};

  ViewsStackedWidget* m_fsPanel{nullptr};
  StackedAddressAndSearchToolBar* m_stackedBar{nullptr};

  ToolBarAndViewSwitcher* m_naviSwitcher{nullptr};
  QToolBar* m_viewsSwitcher{nullptr};

  NavigationToolBar* m_navigationToolBar{nullptr};
  RibbonMenu* m_ribbonMenu{nullptr};
  CustomStatusBar* m_statusBar{nullptr};
private:
  void onPreviewSwitched(PreviewTypeTool::PREVIEW_TYPE_E previewEnum);
  void onViewTypeChanged(const QAction* pViewAct);
};
#endif  // FILEXPLORER_H
