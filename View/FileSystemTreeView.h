#ifndef FILESYSTEMTREEVIEW_H
#define FILESYSTEMTREEVIEW_H

#include "MyQFileSystemModel.h"

#include <QTreeView>
#include <QPushButton>
#include <QDropEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDragEnterEvent>
#include <QContextMenuEvent>
#include <QMouseEvent>
#include <QDragMoveEvent>
#include <QMenu>

class FileSystemTreeView : public QTreeView {
 public:
  FileSystemTreeView(MyQFileSystemModel* fsmModel, QMenu* menu);

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

  void contextMenuEvent(QContextMenuEvent *event) override{
    _menu->popup(viewport()->mapToGlobal(event->pos()));  // or QCursor::pos()
  }

 private:
  QMenu* _menu;
  QPushButton* backwardBtn;  // will not takeover
  QPushButton* forwardBtn;   // will not takeover
};


#endif // FILESYSTEMTREEVIEW_H
