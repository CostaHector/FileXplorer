#ifndef CLICKABLESLIDER_H
#define CLICKABLESLIDER_H

#include <QHBoxLayout>
#include <QLabel>
#include <QMediaPlayer>
#include <QSlider>
using T_REG = std::function<void(int)>;

class ClickableSlider : public QSlider {
 public:
  explicit ClickableSlider(QWidget* parent = nullptr) : QSlider(Qt::Orientation::Horizontal, parent) {}
  auto mousePressEvent(QMouseEvent* ev) -> void override;
  auto reg(T_REG reg) -> void { m_reg = reg; }
  T_REG m_reg;
};
#endif  // CLICKABLESLIDER_H
