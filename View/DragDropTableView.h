#ifndef DRAGDROPTABLEVIEW_H
#define DRAGDROPTABLEVIEW_H

#include "View/ViewHelper.h"
#include "Component/RightClickMenu.h"

class DragDropTableView : public QTableView, public View {
 public:
  DragDropTableView(MyQFileSystemModel* fsmModel, QPushButton* mouseSideKeyBackwardBtn, QPushButton* mouseSideKeyForwardBtn);

  void subscribe() override;

  auto InitViewSettings() -> void override;
  auto UpdateItemViewFontSize() -> void override;

  void dropEvent(QDropEvent* event) override;

  void dragEnterEvent(QDragEnterEvent* event) override;

  void dragMoveEvent(QDragMoveEvent* event) override;

  void dragLeaveEvent(QDragLeaveEvent* event) override;

  void mousePressEvent(QMouseEvent* event) override;

  void mouseMoveEvent(QMouseEvent* event) override;

  void on_ShowContextMenu(const QPoint pnt);

  auto keyPressEvent(QKeyEvent* e) -> void override;

 private:
  QPushButton* backwardBtn;  // will not takeover
  QPushButton* forwardBtn;   // will not takeover
  RightClickMenu* menu;
};

#endif  // DRAGDROPTABLEVIEW_H
