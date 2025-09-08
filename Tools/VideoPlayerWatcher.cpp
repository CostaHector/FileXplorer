#include "VideoPlayerWatcher.h"
#include "PublicVariable.h"
#include "MemoryKey.h"
#include "Logger.h"

#include <QMouseEvent>
#include <QTimer>

constexpr int VideoPlayerWatcher::RIGHT_EDGE_WIDTH_PIXEL;

VideoPlayerWatcher::VideoPlayerWatcher(QObject* parent, QWidget* watched, QListView* controlled)
    : QObject(parent), m_watched(watched), m_controlled(controlled) {
  m_keepListShow = Configuration().value(MemoryKey::KEEP_VIDEOS_PLAYLIST_SHOW.name, MemoryKey::KEEP_VIDEOS_PLAYLIST_SHOW.v).toBool();
  if (m_watched == nullptr or m_controlled == nullptr) {
    LOG_W("m_watched or m_controlled nullptr find");
    return;
  }
  m_watched->setMouseTracking(true);
  m_watched->installEventFilter(this);
}

bool VideoPlayerWatcher::eventFilter(QObject* watched, QEvent* event) {
  if (event->type() == QEvent::MouseMove and watched == m_watched) {
    auto* me = dynamic_cast<QMouseEvent*>(event);
    if (me == nullptr) {
      LOG_W("cannot dynamic cast to mouse event");
      return true;
    }
    if (m_controlled->isVisible()) {
      if (not m_keepListShow and me->pos().x() <= m_watched->width()) {
        m_controlled->setVisible(false);
      }
      return true;
    }
    if (m_watched->width() - me->pos().x() <= RIGHT_EDGE_WIDTH_PIXEL) {
      m_controlled->setVisible(true);
      if (m_controlled->currentIndex().isValid()) {
        QTimer::singleShot(100, [this]() { m_controlled->scrollTo(m_controlled->currentIndex()); });
      }
    }
    return true;
  }
  return QObject::eventFilter(watched, event);
}
