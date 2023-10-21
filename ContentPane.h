#ifndef CONTENTPANE_H
#define CONTENTPANE_H

#include <QWidget>
#include "Component/CustomStatusBar.h"
#include "FolderPreviewHTML.h"
#include "FolderPreviewWidget.h"
#include "MyQFileSystemModel.h"
#include "NavigationAndAddressBar.h"
#include "View/DragDropTableView.h"

class ContentPane : public QWidget {
  Q_OBJECT
 public:
  explicit ContentPane(QWidget* parent = nullptr,
                       const QString& defaultPath = "",
                       FolderPreviewHTML* previewHtml_ = nullptr,
                       FolderPreviewWidget* previewWidget_ = nullptr,
                       CustomStatusBar* _statusBar = nullptr);
  auto CurrentPath() -> QString {
    if (!fileSysModel) {
      qDebug("[Error] fileSysModel is nullptr");
      return "";
    }
    return QFileInfo(fileSysModel->rootPath()).absoluteFilePath();
  }

  auto IntoNewPath(QString newPath,
                   bool isNewPath = true,
                   bool isF5Force = false) -> bool;

  auto on_searchTextChanged(const QString& targetStr) -> bool;
  auto on_searchEnterKey(const QString& targetStr) -> bool;

  auto subscribe() -> void;
  auto on_cellDoubleClicked(QModelIndex clickedIndex) -> bool;
  auto on_selectionChanged(const QItemSelection& selected,
                           const QItemSelection& deselected) -> bool;
  auto onAfterDirectoryLoaded(const QString& loadedPath) -> bool;

 signals:

 public:
  struct Anchor {
    int row;
    int col;
  };
  QMap<QString, Anchor> m_anchorTags;
  MyQFileSystemModel* fileSysModel;
  NavigationAndAddressBar* addressBar;
  DragDropTableView* view;
  FolderPreviewHTML* previewHtml;
  FolderPreviewWidget* previewWidget;
  CustomStatusBar* logger;
};

#endif  // CONTENTPANE_H
