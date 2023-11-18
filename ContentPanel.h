#ifndef CONTENTPANEL_H
#define CONTENTPANEL_H

#include <QWidget>
#include "Component/CustomStatusBar.h"
#include "FolderPreviewHTML.h"
#include "FolderPreviewWidget.h"
#include "MyQFileSystemModel.h"
#include "NavigationAndAddressBar.h"
#include "View/DragDropTableView.h"

class ContentPanel : public QWidget {
  Q_OBJECT
 public:
  explicit ContentPanel(QWidget* parent = nullptr,
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

  auto IntoNewPath(QString newPath, bool isNewPath = true, bool isF5Force = false) -> bool;

  auto on_searchTextChanged(const QString& targetStr) -> bool;
  auto on_searchEnterKey(const QString& targetStr) -> bool;

  auto subscribe() -> void;
  auto on_cellDoubleClicked(QModelIndex clickedIndex) -> bool;
  auto on_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) -> bool;
  auto onAfterDirectoryLoaded(const QString& loadedPath) -> bool;

  auto keyPressEvent(QKeyEvent* e) -> void override {
    if (e->modifiers() == Qt::NoModifier and e->key() == Qt::Key_Backspace) {
      addressBar->onBackspaceEvent();
      return;
    } else if (e->modifiers() == Qt::NoModifier and (e->key() == Qt::Key_Enter or e->key() == Qt::Key_Return)) {
      on_cellDoubleClicked(view->currentIndex());
      return;
    }
    QWidget::keyPressEvent(e);
  }

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

#endif  // CONTENTPANEL_H
