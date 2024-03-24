#ifndef VIDEOPLAYERWATCHER_H
#define VIDEOPLAYERWATCHER_H

#include <QEvent>
#include <QListView>
#include <QObject>
#include <QWidget>

class VideoPlayerWatcher : public QObject {
 public:
  explicit VideoPlayerWatcher(QObject* parent = nullptr, QWidget* watched = nullptr, QListView* controlled = nullptr);
  bool eventFilter(QObject* watched, QEvent* event) override;

  void setKeepListShow(bool keepShow) { m_keepListShow = keepShow; }

  bool m_keepListShow;
  QWidget* m_watched;
  QListView* m_controlled;
  static constexpr int RIGHT_EDGE_WIDTH_PIXEL = 20;
};

#endif  // VIDEOPLAYERWATCHER_H
