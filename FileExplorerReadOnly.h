#ifndef FILEEXPLORERREADONLY_H
#define FILEEXPLORERREADONLY_H

#include <QFileSystemModel>
#include <QMainWindow>
#include <QRect>
#include <QSettings>

#include "Component/CustomStatusBar.h"
#include "ContentPane.h"
#include "NavigationToolBar.h"
#include "RibbonMenu.h"

class FileExplorerReadOnly : public QMainWindow {
  Q_OBJECT

 public:
  FileExplorerReadOnly(QWidget* parent = nullptr, const QString& initialPath = "");
  ~FileExplorerReadOnly();
  virtual void closeEvent(QCloseEvent* event);

  auto ReadSettings(const QString& initialPath) -> QString;
  void subscribe();

  const static QString DEFAULT_PATH;
  QDockWidget* previewHtmlDock;
  FolderPreviewHTML* previewHtml;
  FolderPreviewWidget* previewWidget;
  ContentPane* explorerCentralWidget;
  NavigationToolBar* _navigationToolBar;
  RibbonMenu* osm;
  CustomStatusBar* _statusBar;
};
#endif  // FILEEXPLORERREADONLY_H
