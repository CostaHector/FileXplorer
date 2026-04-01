#ifndef FILESYSTEMTREEVIEW_H
#define FILESYSTEMTREEVIEW_H

#include "FileSystemModel.h"
#include "CustomTreeView.h"

class FileSystemTreeView : public CustomTreeView {
 public:
  explicit FileSystemTreeView(FileSystemModel* fsmModel, QWidget* parent = nullptr);
  void subscribe();

 protected:
  void dropEvent(QDropEvent* event) override;
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dragMoveEvent(QDragMoveEvent* event) override;
  void dragLeaveEvent(QDragLeaveEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;

 private:
  FileSystemModel* _fsModel{nullptr};
  QPoint mDragStartPosition;
};

#endif  // FILESYSTEMTREEVIEW_H
