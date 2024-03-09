#ifndef FILEEXPLORERREADONLY_H
#define FILEEXPLORERREADONLY_H

#include <QFileSystemModel>
#include <QMainWindow>
#include <QRect>
#include <QSettings>
#include <QStackedWidget>

#include "Component/CustomStatusBar.h"
#include "Component/StackedToolBar.h"
#include "Component/NavigationToolBar.h"

#include "Tools/NavigationViewSwitcher.h"

#include "ContentPanel.h"
#include "RibbonMenu.h"

class FileExplorerReadOnly : public QMainWindow {
  Q_OBJECT

 public:
  FileExplorerReadOnly(const int argc = 1, char const* const argv[] = nullptr, QWidget* parent = nullptr);
  ~FileExplorerReadOnly();
  virtual void closeEvent(QCloseEvent* event);

  auto ReadSettings(const QString& initialPath) -> QString;
  void InitComponentVisibility();
  void UpdateComponentVisibility();

  void keyPressEvent(QKeyEvent* ev){
    if (ev->key() == Qt::Key_F3){  // F3 Search
      m_fsPanel->_addressBar->m_searchLE->setFocus();
      m_fsPanel->_addressBar->m_searchLE->selectAll();
      return;
    }else if (ev->key() == Qt::Key_Escape){
      m_fsPanel->m_fsTableView->clearSelection();
      m_fsPanel->m_fsTableView->setFocus();
      return;
    }
    QMainWindow::keyPressEvent(ev);
  }

  QDockWidget* previewHtmlDock;
  FolderPreviewHTML* previewHtml;
  FolderPreviewWidget* previewWidget;

  ContentPanel* m_fsPanel;
  StackedToolBar* m_stackedBar;

  NavigationViewSwitcher* m_viewSwitcher;
  QToolBar* m_views;

  NavigationToolBar* m_navigationToolBar;
  RibbonMenu* osm;
  CustomStatusBar* _statusBar;
};
#endif  // FILEEXPLORERREADONLY_H
