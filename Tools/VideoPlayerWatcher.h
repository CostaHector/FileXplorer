#ifndef VIDEOPLAYERWATCHER_H
#define VIDEOPLAYERWATCHER_H

#include <QEvent>
#include <QObject>
#include <QWidget>


class VideoPlayerWatcher : public QObject {
 public:
  explicit VideoPlayerWatcher(QObject* parent = nullptr, QWidget* watched = nullptr, QWidget* controlled = nullptr)
      : QObject(parent), m_watched(watched), m_controlled(controlled) {
    if (m_watched == nullptr or m_controlled == nullptr) {
      qWarning("m_watched or m_controlled nullptr find");
      return;
    }
    m_watched->setMouseTracking(true);
    m_watched->installEventFilter(this);
  }

  bool eventFilter(QObject* watched, QEvent* event) override;

  QWidget* m_watched;
  QWidget* m_controlled;
  static constexpr int RIGHT_EDGE_WIDTH_PIXEL = 20;
};

#endif  // VIDEOPLAYERWATCHER_H
