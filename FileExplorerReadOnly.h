#ifndef FILEEXPLORERREADONLY_H
#define FILEEXPLORERREADONLY_H

#include <QFileSystemModel>
#include <QMainWindow>
#include <QRect>
#include <QSettings>
#include <QStackedWidget>

#include "Component/CustomStatusBar.h"
#include "Component/JsonEditor.h"
#include "Component/VideoPlayer.h"
#include "ContentPanel.h"
#include "NavigationToolBar.h"
#include "RibbonMenu.h"
#include "View/DatabaseTableView.h"

class FileExplorerReadOnly : public QMainWindow {
  Q_OBJECT

 public:
  FileExplorerReadOnly(const int argc = 1, char const* const argv[] = nullptr, QWidget* parent = nullptr);
  ~FileExplorerReadOnly();
  virtual void closeEvent(QCloseEvent* event);

  auto ReadSettings(const QString& initialPath) -> QString;
  void subscribe();

  void SwitchStackWidget();
  void InitComponentVisibility();
  void UpdateComponentVisibility();

  void keyPressEvent(QKeyEvent* ev){
    if (ev->key() == Qt::Key_F3){  // F3 Search
      m_fsPanel->addressBar->searchLE->setFocus();
      m_fsPanel->addressBar->searchLE->selectAll();
      return;
    }
    QMainWindow::keyPressEvent(ev);
  }

  const static QString DEFAULT_PATH;
  QDockWidget* previewHtmlDock;
  FolderPreviewHTML* previewHtml;
  FolderPreviewWidget* previewWidget;
  ContentPanel* m_fsPanel;
  DatabasePanel* m_dbPanel;
  QStackedWidget* stackCentralWidget;
  NavigationToolBar* _navigationToolBar;
  RibbonMenu* osm;
  CustomStatusBar* _statusBar;

  JsonEditor* m_jsonEditor;
  VideoPlayer* m_videoPlayer;
};
#endif  // FILEEXPLORERREADONLY_H
