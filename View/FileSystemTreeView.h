#ifndef FILESYSTEMTREEVIEW_H
#define FILESYSTEMTREEVIEW_H

#include "RightClickMenu.h"
#include "FileSystemModel.h"

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMouseEvent>
#include <QTreeView>
#include <QMenu>

class FileSystemTreeView : public QTreeView {
 public:
  FileSystemTreeView(FileSystemModel* fsmModel, QWidget* parent = nullptr);

  void subscribe();
  auto InitViewSettings() -> void;
  auto UpdateItemViewFontSize() -> void;

  void dropEvent(QDropEvent* event) override;

  void dragEnterEvent(QDragEnterEvent* event) override;

  void dragMoveEvent(QDragMoveEvent* event) override;

  void dragLeaveEvent(QDragLeaveEvent* event) override;

  void mousePressEvent(QMouseEvent* event) override;

  void mouseMoveEvent(QMouseEvent* event) override;

  void keyPressEvent(QKeyEvent* event) override;

  void contextMenuEvent(QContextMenuEvent* event) override;

 private:
  FileSystemModel* _fsModel {nullptr};
  QMenu* m_menu {nullptr};
  QPoint mDragStartPosition;
};

#endif  // FILESYSTEMTREEVIEW_H
