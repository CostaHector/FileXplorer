#ifndef FILESYSTEMLISTVIEW_H
#define FILESYSTEMLISTVIEW_H

#include "RightClickMenu.h"
#include "CustomListView.h"
#include "FileSystemModel.h"

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QListView>
#include <QMenu>
#include <QMouseEvent>

class FileSystemListView : public CustomListView {
 public:
  FileSystemListView(FileSystemModel* fsmModel, QWidget* parent=nullptr);

  void subscribe();

  void dropEvent(QDropEvent* event) override;

  void dragEnterEvent(QDragEnterEvent* event) override;

  void dragMoveEvent(QDragMoveEvent* event) override;

  void dragLeaveEvent(QDragLeaveEvent* event) override;

  void mousePressEvent(QMouseEvent* event) override;

  void mouseMoveEvent(QMouseEvent* event) override;

  auto keyPressEvent(QKeyEvent* event) -> void override;

 private:
  QMenu* m_fsMenu = new RightClickMenu("Right click menu", this);
  QPoint mDragStartPosition;
};

#endif  // FILESYSTEMLISTVIEW_H
