#ifndef FILESYSTEMTABLEVIEW_H
#define FILESYSTEMTABLEVIEW_H

#include "Component/FileSystemMenu.h"
#include "CustomTableView.h"
#include "MyQFileSystemModel.h"

#include <QContextMenuEvent>
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
  QMenu* m_fsMenu = new FileSystemMenu("Right click menu", this);
};

#endif  // FILESYSTEMTABLEVIEW_H
