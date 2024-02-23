#include "VideoPlayerWatcher.h"
#include <QDebug>
#include <QMouseEvent>

constexpr int VideoPlayerWatcher::RIGHT_EDGE_WIDTH_PIXEL;

bool VideoPlayerWatcher::eventFilter(QObject* watched, QEvent* event) {
  if (event->type() == QEvent::MouseMove and watched == m_watched) {
    auto* me = dynamic_cast<QMouseEvent*>(event);
    if (me == nullptr) {
      qWarning("cannot dynamic cast to mouse event");
      return true;
    }
    if (m_controlled->isVisible()) {
      if (me->pos().x() <= RIGHT_EDGE_WIDTH_PIXEL) {
        m_controlled->setVisible(false);
      }
      return true;
    }
    if (m_watched->width() - me->pos().x() <= RIGHT_EDGE_WIDTH_PIXEL) {
      m_controlled->setVisible(true);
    }
    return true;
  }
  return QObject::eventFilter(watched, event);
}
