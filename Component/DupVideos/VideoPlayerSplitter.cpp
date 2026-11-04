#include "VideoPlayerSplitter.h"
#include "BasicVideoView.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <QStringList>

VideoPlayerSplitter::VideoPlayerSplitter(Qt::Orientation orientation, QWidget *parent)
  : QSplitter{orientation, parent} {
  setAcceptDrops(true);
}

void VideoPlayerSplitter::onShowViewPlayer(bool bShow) {
  if (!bShow) {
    setHidden(true);
    return;
  }
  if (!m_isInited) {
    m_isInited = true;
    if (m_left == nullptr) {
      m_left = new BasicVideoView{true, this};
      m_left->setAlwaysShowStatusLabel();
      m_left->setAcceptDrops(false);
    }
    if (m_right == nullptr) {
      m_right = new BasicVideoView{true, this};
      m_right->setAlwaysShowStatusLabel();
      m_right->setAcceptDrops(false);
    }
    addWidget(m_left);
    addWidget(m_right);
  }
  if (isHidden()) {
    setHidden(false);
  }
}

void VideoPlayerSplitter::dragEnterEvent(QDragEnterEvent *e) {
  if (hasPlayableUrls(e->mimeData())) {
    e->acceptProposedAction();
    return;
  }
  e->ignore();
}

void VideoPlayerSplitter::dragMoveEvent(QDragMoveEvent *e) {
  if (hasPlayableUrls(e->mimeData())) {
    e->acceptProposedAction();
    return;
  }
  e->ignore();
}

void VideoPlayerSplitter::dropEvent(QDropEvent *e) {
  const QStringList paths = collectLocalFiles(e->mimeData());
  if (paths.isEmpty() || m_left == nullptr || m_right == nullptr) {
    e->ignore();
    return;
  }

  if (paths.size() == 1)  {
    // 数量 == 1：按松手点 x 判左右
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const int dropX = e->position().toPoint().x();
#else
    const int dropX = e->pos().x();   // Qt 5
#endif
    m_useLeftNextTime = dropX < width() / 2;
  }
  onPlayVideos(paths);
  e->acceptProposedAction();
}

bool VideoPlayerSplitter::onPlayVideos(const QStringList& paths) {
  if (m_left == nullptr || m_right == nullptr) {
    return false;
  }
  if (paths.isEmpty()) {
    return false;
  }
  if (paths.size() >= 2) {
    m_left->PlayAVideo(paths[0], /*forcePlayInstantly=*/true);
    m_right->PlayAVideo(paths[1], /*forcePlayInstantly=*/true);
  } else {
    if (m_useLeftNextTime) {
      m_left->PlayAVideo(paths[0], /*forcePlayInstantly=*/true);
    } else {
      m_right->PlayAVideo(paths[0], /*forcePlayInstantly=*/true);
    }
    m_useLeftNextTime = !m_useLeftNextTime;
  }
  return true;
}

bool VideoPlayerSplitter::hasPlayableUrls(const QMimeData *mime) {
  return mime != nullptr && mime->hasUrls();
}

QStringList VideoPlayerSplitter::collectLocalFiles(const QMimeData *mime) {
  QStringList paths;
  if (mime == nullptr) return paths;
  const QList<QUrl> urls = mime->urls();
  for (const QUrl& u : urls) {
    const QString p = u.toLocalFile();
    if (p.isEmpty()) continue; // 非本地 url 跳过
    paths << p;
  }
  return paths;
}
