﻿#ifndef FILESYSTEMTREEVIEW_H
#define FILESYSTEMTREEVIEW_H

#include "Component/RightClickMenu.h"
#include "Model/MyQFileSystemModel.h"

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QTreeView>

class FileSystemTreeView : public QTreeView {
 public:
  FileSystemTreeView(MyQFileSystemModel* fsmModel, QWidget* parent = nullptr);

  void subscribe();
  auto InitViewSettings() -> void;
  auto UpdateItemViewFontSize() -> void;

  void dropEvent(QDropEvent* event) override;

  void dragEnterEvent(QDragEnterEvent* event) override;

  void dragMoveEvent(QDragMoveEvent* event) override;

  void dragLeaveEvent(QDragLeaveEvent* event) override;

  void mousePressEvent(QMouseEvent* event) override;

  void mouseMoveEvent(QMouseEvent* event) override;

  auto keyPressEvent(QKeyEvent* event) -> void override;

  void contextMenuEvent(QContextMenuEvent* event) override {
    m_fsMenu->popup(viewport()->mapToGlobal(event->pos()));  // or QCursor::pos()
  }

 private:
  QMenu* m_fsMenu = new RightClickMenu("Right click menu", this);
  QPoint mDragStartPosition;
};

#endif  // FILESYSTEMTREEVIEW_H
