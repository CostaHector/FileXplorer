#ifndef CLICKABLESLIDER_H
#define CLICKABLESLIDER_H

#include <QSlider>

class ClickableSlider : public QSlider {
public:
  using T_REG = std::function<void(int)>;

  using QSlider::QSlider;
  void reg(T_REG regFunc) { m_reg = regFunc; }

protected:
  void mousePressEvent(QMouseEvent* ev) override;

private:
  T_REG m_reg;
};

#endif // CLICKABLESLIDER_H
