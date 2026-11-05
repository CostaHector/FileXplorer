#include "AutoHideLabel.h"
#include "PathTool.h"
#include <QMouseEvent>

constexpr int AutoHideLabel::TIME_PERIOD;

AutoHideLabel::AutoHideLabel(const QString &text, QWidget *parent, Qt::WindowFlags f)
  : QLabel{text, parent, f} {
  setTextInteractionFlags(Qt::TextSelectableByMouse);
  mAutoHideTimer.setInterval(TIME_PERIOD);
  mAutoHideTimer.setSingleShot(true);
  connect(&mAutoHideTimer, &QTimer::timeout, this, &QLabel::hide);
}

void AutoHideLabel::setStatusMessage(const QString &text) {
  m_statusMsg = text;
  updateText();
}

void AutoHideLabel::showPlayingMediaName(const QString &mediaFullPath) {
  m_mediaMsg = PathTool::fileName(mediaFullPath);
  updateText();
}

void AutoHideLabel::updateText() {
  setText(m_statusMsg + " | " + m_mediaMsg);
  setVisible(true);
  if (m_timerDisabled) {
    return;
  }
  mAutoHideTimer.stop();
  mAutoHideTimer.start();
}

void AutoHideLabel::setDebugMode(bool bDebugMode) {
  if (m_timerDisabled == bDebugMode) {
    return;
  }
  m_timerDisabled = bDebugMode;
  if (bDebugMode) {
    mAutoHideTimer.stop();
    setVisible(true);
  } else {
    // 恢复自动隐藏：立即重新计时，否则标签会一直挂着
    mAutoHideTimer.stop();
    mAutoHideTimer.start();
  }
  updateDebugVisualHint();
}

void AutoHideLabel::mouseDoubleClickEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    setDebugMode(!m_timerDisabled); // toggle
    event->accept();
    return;
  }
  QLabel::mouseDoubleClickEvent(event);
}

void AutoHideLabel::updateDebugVisualHint() {
  // 可选：调试模式给个明显的视觉标记，方便判断当前状态
  setToolTip(m_timerDisabled
                 ? "Debug: auto-hide disabled (double-click to re-enable)"
                 : "Double-click to disable auto-hide for debugging");
}
