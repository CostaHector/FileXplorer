#ifndef AUTOHIDELABEL_H
#define AUTOHIDELABEL_H

#include <QLabel>
#include <QTimer>

class AutoHideLabel : public QLabel {
public:
  explicit AutoHideLabel(const QString& text, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
  explicit AutoHideLabel(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags())
    : AutoHideLabel{"", parent, f} {}
  void setStatusMessage(const QString &text);
  void showPlayingMediaName(const QString& mediaFullPath);
  void setDebugMode(bool bDebugMode);
protected:
  void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
  void updateText();
  void updateDebugVisualHint();

  static constexpr int TIME_PERIOD{3000}; // unit ms
  QString m_statusMsg, m_mediaMsg;
  QTimer mAutoHideTimer;
  bool m_timerDisabled{false};
};

#endif // AUTOHIDELABEL_H
