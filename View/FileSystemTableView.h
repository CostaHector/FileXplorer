#ifndef FILESYSTEMTABLEVIEW_H
#define FILESYSTEMTABLEVIEW_H

#include "MyQFileSystemModel.h"

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

class FileSystemTableView : public QTableView {
 public:
  FileSystemTableView(MyQFileSystemModel* fsmModel, QMenu* menu);

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

#endif  // FILESYSTEMTABLEVIEW_H
