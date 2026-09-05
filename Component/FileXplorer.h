#ifndef FILEXPLORER_H
#define FILEXPLORER_H

#include <QMainWindow>
#include "NavigationToolBar.h"
#include "RibbonMenu.h"
#include "ViewsStackedWidget.h"
#include "PreviewDockWidget.h"
#include "CurrentRowPreviewer.h"
#include "StackedAddressAndSearchToolBar.h"

class FileXplorer : public QMainWindow {
public:
  explicit FileXplorer(const QStringList& args, QWidget* parent = nullptr);
  virtual ~FileXplorer();

  static QString GetInitialPathFromArgs(const QStringList& args);
  void RestoreWindowStateAndSetupUI();
  void InitComponentVisibility();
  void subscribe();

  QDockWidget* m_naviSideBarDock{nullptr};
  PreviewDockWidget* m_previewHtmlDock{nullptr};

  CurrentRowPreviewer* m_previewFolder{nullptr};
  FolderPreviewSwitcher* m_previewSwitcher{nullptr};

  ViewsStackedWidget* m_fsPanel{nullptr};
  StackedAddressAndSearchToolBar* m_stackedBar{nullptr};

  ViewSwitchHelper* m_viewSwitchHelper{nullptr};

  NavigationToolBar* m_navigationToolBar{nullptr};
  RibbonMenu* m_ribbonMenu{nullptr};

 protected:
  void keyPressEvent(QKeyEvent* ev) override;
  void closeEvent(QCloseEvent* event) override;
  void showEvent(QShowEvent *event) override;

private:
  void initGlobalInstance();
  void releaseGlobalInstance();
  void onViewWidgetChanged(ViewTypeTool::ViewType viewType);
};
#endif  // FILEXPLORER_H
