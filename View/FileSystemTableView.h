#ifndef FILESYSTEMTABLEVIEW_H
#define FILESYSTEMTABLEVIEW_H

#include "CustomTableView.h"
#include "FileSystemModel.h"

class FileSystemTableView : public CustomTableView {
 public:
  explicit FileSystemTableView(FileSystemModel* fsmModel, QWidget* parent = nullptr);

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

#endif  // FILESYSTEMTABLEVIEW_H
