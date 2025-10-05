#ifndef DRAGGABLETOOLBUTTON_H
#define DRAGGABLETOOLBUTTON_H

#include <QToolButton>
#include <QMouseEvent>

class DraggableToolButton : public QToolButton {
 public:
  explicit DraggableToolButton(QWidget* parent = nullptr);

 protected:
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;

 private:
  QPoint mDragStartPosition;
};

#endif // DRAGGABLETOOLBUTTON_H
