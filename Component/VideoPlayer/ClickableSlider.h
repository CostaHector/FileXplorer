#ifndef CLICKABLESLIDER_H
#define CLICKABLESLIDER_H

#include <QSlider>

class ClickableSlider : public QSlider {
 public:
  using T_REG = std::function<void(int)>;

  using QSlider::QSlider;
  bool regMouseEventProcessor(T_REG regFunc);
  bool isRegistered() const { return (bool)m_reg; }

 protected:
  void mousePressEvent(QMouseEvent* ev) override;
  void keyPressEvent(QKeyEvent* ev) override;

 private:
  void mousePressEventCore(int curClickedPositionValue);
  T_REG m_reg;
};

#endif  // CLICKABLESLIDER_H
