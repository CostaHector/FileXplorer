#ifndef VIDEOPLAYERSPLITTER_H
#define VIDEOPLAYERSPLITTER_H

#include <QSplitter>

class QMimeData;
class BasicVideoView;
class QLabel;

class VideoPlayerSplitter : public QSplitter {
  Q_OBJECT
public:
  explicit VideoPlayerSplitter(Qt::Orientation orientation, QWidget* parent = nullptr);
  void onShowViewPlayer(bool bShow);
  bool onPlayVideos(const QStringList& paths);

protected:
  void dragEnterEvent(QDragEnterEvent* e) override;

  void dragMoveEvent(QDragMoveEvent* e) override;

  void dropEvent(QDropEvent* e) override;

private:
  static bool hasPlayableUrls(const QMimeData* mime);
  static QStringList collectLocalFiles(const QMimeData* mime);

  bool m_isInited{false};

  BasicVideoView* m_left{nullptr};
  BasicVideoView* m_right{nullptr};
  bool m_useLeftNextTime{true}; // 交替使用
};

#endif // VIDEOPLAYERSPLITTER_H
