#ifndef FILESYSTEMTABLEVIEW_H
#define FILESYSTEMTABLEVIEW_H

#include "CustomTableView.h"
#include "RightClickMenu.h"
#include "FileSystemModel.h"

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMenu>
#include <QMouseEvent>

class FileSystemTableView : public CustomTableView {
 public:
  explicit FileSystemTableView(FileSystemModel* fsmModel, QWidget* parent = nullptr);

  void subscribe();

  void dropEvent(QDropEvent* event) override;

  void dragEnterEvent(QDragEnterEvent* event) override;

  void dragMoveEvent(QDragMoveEvent* event) override;

  void dragLeaveEvent(QDragLeaveEvent* event) override;

  void mousePressEvent(QMouseEvent* event) override;

  void mouseMoveEvent(QMouseEvent* event) override;

  auto keyPressEvent(QKeyEvent* event) -> void override;

 private:
  FileSystemModel* _fsModel {nullptr};
  QPoint mDragStartPosition;
};

#endif  // FILESYSTEMTABLEVIEW_H
