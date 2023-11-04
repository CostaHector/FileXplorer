#ifndef FILEEXPLORERREADONLY_H
#define FILEEXPLORERREADONLY_H

#include <QFileSystemModel>
#include <QMainWindow>
#include <QRect>
#include <QSettings>
#include <QStackedWidget>

#include "Component/CustomStatusBar.h"
#include "Component/JsonEditor.h"
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

  const static QString DEFAULT_PATH;
  QDockWidget* previewHtmlDock;
  FolderPreviewHTML* previewHtml;
  FolderPreviewWidget* previewWidget;
  ContentPanel* fsmView;
  DatabaseTableView* dbView;
  QStackedWidget* stackCentralWidget;
  NavigationToolBar* _navigationToolBar;
  RibbonMenu* osm;
  CustomStatusBar* _statusBar;

  JsonEditor* m_jsonEditor;
};
#endif  // FILEEXPLORERREADONLY_H
