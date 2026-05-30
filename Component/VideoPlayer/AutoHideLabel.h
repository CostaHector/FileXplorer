#ifndef AUTOHIDELABEL_H
#define AUTOHIDELABEL_H

#include <QLabel>
#include <QTimer>

class AutoHideLabel : public QLabel {
public:
  explicit AutoHideLabel(const QString& text, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags())
    : QLabel{text, parent, f} {
    mAutoHideTimer.setInterval(TIME_PERIOD);
    mAutoHideTimer.setSingleShot(true);
    connect(&mAutoHideTimer, &QTimer::timeout, this, &QLabel::hide);
  }

  explicit AutoHideLabel(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags())
    : AutoHideLabel{"", parent, f} {}

  void setAutoHideText(const QString &text) {
    mAutoHideTimer.stop();
    setText(text);
    setVisible(true);
    mAutoHideTimer.start();
  }

private:
  static constexpr int TIME_PERIOD{3000}; // unit ms
  QTimer mAutoHideTimer;
};

#endif // AUTOHIDELABEL_H
