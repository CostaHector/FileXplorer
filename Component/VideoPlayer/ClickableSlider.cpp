#include "ClickableSlider.h"
#include "Logger.h"
#include "PublicMacro.h"
#include <QMouseEvent>

bool ClickableSlider::regMouseEventProcessor(T_REG regFunc) {  // 点击事件, 滑动事件 处理回调
  if (isRegistered()) {
    LOG_E("Already registered, cannot register again");
    return false;
  }
  if (!regFunc) {
    LOG_W("Cannot register an nullptr callback");
    return false;
  }
  m_reg = regFunc;
  connect(this, &QAbstractSlider::sliderMoved, m_reg);
  return true;
}

void ClickableSlider::mousePressEvent(QMouseEvent* ev) {
  CHECK_NULLPTR_RETURN_VOID(ev);
  if (ev->button() == Qt::MouseButton::LeftButton) {
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
    const int curPositionValue = (double)dev / duration * span + this->minimum();
    mousePressEventCore(curPositionValue);
    ev->setAccepted(true);
    return;
  }
  QSlider::mousePressEvent(ev);
}

void ClickableSlider::keyPressEvent(QKeyEvent* ev) {
  CHECK_NULLPTR_RETURN_VOID(ev);
  int modifiedToValue = value();
  if (ev->key() == Qt::Key_Left) {
    modifiedToValue -= singleStep();
  } else if (ev->key() == Qt::Key_Right) {
    modifiedToValue += singleStep();
  } else {
    QSlider::keyPressEvent(ev);
    return;
  }
  mousePressEventCore(modifiedToValue);  // 调用核心处理函数
  ev->accept();  // 接受键盘事件
}

void ClickableSlider::mousePressEventCore(int curClickedPositionValue) {
  if (value() == curClickedPositionValue) {
    return;
  }
  setValue(curClickedPositionValue);
  if (m_reg) {
    m_reg(curClickedPositionValue);
  }
}
