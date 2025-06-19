#ifndef FILESYSTEMTABLEVIEW_H
#define FILESYSTEMTABLEVIEW_H

#include "CustomTableView.h"
#include "Component/RightClickMenu.h"
#include "Model/MyQFileSystemModel.h"

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMenu>
#include <QMouseEvent>

class FileSystemTableView : public CustomTableView {
 public:
  FileSystemTableView(MyQFileSystemModel* fsmModel, QWidget* parent = nullptr);

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

#endif  // FILESYSTEMTABLEVIEW_H
