#include "ClickableSlider.h"

#include <QMouseEvent>
void ClickableSlider::mousePressEvent(QMouseEvent* ev) {
  int dev = -1;
  int duration = -1;
  if (orientation() == Qt::Orientation::Horizontal) {
    dev = ev->x();
    duration = width();
  } else {
    dev = ev->y();
    duration = height();
  }
  const int span = this->maximum() - this->minimum();
  const int curPosition = (double)dev / duration * span + this->minimum();
  if (m_reg){
    m_reg(curPosition);
  }else{
    setValue(curPosition);
  }
  QSlider::mousePressEvent(ev);
}

//#define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  ClickableValueSlider slider;
  slider.show();
  a.exec();
  return 0;
}
#endif
