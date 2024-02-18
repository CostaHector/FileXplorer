#include "VideoPlayerWatcher.h"
#include <QMouseEvent>
#include <QDebug>

constexpr int VideoPlayerWatcher::RIGHT_EDGE_WIDTH_PIXEL;

bool VideoPlayerWatcher::eventFilter(QObject* watched, QEvent* event) {
  if (event->type() == QEvent::MouseMove and watched == m_watched and not m_controlled->isVisible()) {
    auto* me = dynamic_cast<QMouseEvent*>(event);
    if (me == nullptr) {
      qWarning("cannot dynamic cast to mouse event");
      return false;
    }
    if (m_watched->width() - me->pos().x() <= RIGHT_EDGE_WIDTH_PIXEL) {
      m_controlled->setVisible(true);
    }
  }
  return QObject::eventFilter(watched, event);
}
