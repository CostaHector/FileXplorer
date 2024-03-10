#ifndef FILESYSTEMLISTVIEW_H
#define FILESYSTEMLISTVIEW_H

#include "Component/FileSystemMenu.h"
#include "MyQFileSystemModel.h"

#include <QContextMenuEvent>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QListView>
#include <QMenu>
#include <QMouseEvent>
#include <QPushButton>

class FileSystemListView : public QListView {
 public:
  FileSystemListView(MyQFileSystemModel* fsmModel);

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

  void contextMenuEvent(QContextMenuEvent* event) override {
    m_fsMenu->popup(viewport()->mapToGlobal(event->pos()));  // or QCursor::pos()
  }

 private:
  QMenu* m_fsMenu = new FileSystemMenu("Right click menu", this);
  QPushButton* backwardBtn{nullptr};  // will not takeover
  QPushButton* forwardBtn{nullptr};   // will not takeover
};

#endif  // FILESYSTEMLISTVIEW_H
