#ifndef FILESYSTEMTREEVIEW_H
#define FILESYSTEMTREEVIEW_H

#include "MyQFileSystemModel.h"
#include "Component/FileSystemMenu.h"

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
  FileSystemTreeView(MyQFileSystemModel* fsmModel);

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
    m_fsMenu->popup(viewport()->mapToGlobal(event->pos()));  // or QCursor::pos()
  }

 private:
  QMenu* m_fsMenu = new FileSystemMenu("Right click menu", this);
  QPushButton* backwardBtn{nullptr};  // will not takeover
  QPushButton* forwardBtn{nullptr};   // will not takeover
};


#endif // FILESYSTEMTREEVIEW_H
