#ifndef FILESYSTEMTREEVIEW_H
#define FILESYSTEMTREEVIEW_H

#include "RightClickMenu.h"
#include "FileSystemModel.h"

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QTreeView>

class FileSystemTreeView : public QTreeView {
 public:
  FileSystemTreeView(FileSystemModel* fsmModel, QWidget* parent = nullptr);

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
#ifndef RUNNING_UNIT_TESTS
    m_fsMenu->popup(viewport()->mapToGlobal(event->pos()));  // or QCursor::pos()
#endif
  }

 private:
  FileSystemModel* _fsModel {nullptr};
  QMenu* m_fsMenu {nullptr};
  QPoint mDragStartPosition;
};

#endif  // FILESYSTEMTREEVIEW_H
