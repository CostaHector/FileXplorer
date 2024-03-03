#ifndef FILESYSTEMTABLEVIEW_H
#define FILESYSTEMTABLEVIEW_H

#include "MyQFileSystemModel.h"
#include "CustomTableView.h"

#include <QTableView>
#include <QPushButton>
#include <QDropEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDragEnterEvent>
#include <QContextMenuEvent>
#include <QMouseEvent>
#include <QDragMoveEvent>
#include <QMenu>

class FileSystemTableView : public CustomTableView {
 public:
  FileSystemTableView(MyQFileSystemModel* fsmModel, QMenu* menu, QWidget* parent=nullptr);

  void subscribe();

  void dropEvent(QDropEvent* event) override;

  void dragEnterEvent(QDragEnterEvent* event) override;

  void dragMoveEvent(QDragMoveEvent* event) override;

  void dragLeaveEvent(QDragLeaveEvent* event) override;

  void mousePressEvent(QMouseEvent* event) override;

  void mouseMoveEvent(QMouseEvent* event) override;

  auto keyPressEvent(QKeyEvent* event) -> void override;

 private:
  QPushButton* backwardBtn;  // will not takeover
  QPushButton* forwardBtn;   // will not takeover
};

#endif  // FILESYSTEMTABLEVIEW_H
