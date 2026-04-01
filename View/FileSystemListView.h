#ifndef FILESYSTEMLISTVIEW_H
#define FILESYSTEMLISTVIEW_H

#include "CustomListView.h"
#include "FileSystemModel.h"

class FileSystemListView : public CustomListView {
 public:
  explicit FileSystemListView(FileSystemModel* fsmModel, QWidget* parent = nullptr);

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
  void initExclusivePreferenceSetting() override;

  FileSystemModel* _fsModel{nullptr};
  QPoint mDragStartPosition;
};

#endif  // FILESYSTEMLISTVIEW_H
