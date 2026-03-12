#include "VideoTableView.h"
#include "NotificatorMacro.h"
#include <random>

VideoTableView::VideoTableView(QWidget* parent) : CustomTableView{"VIEDO_TABLE_VIEW", parent} {
  mVideoModel = new VideoTableModel{this};
  mProxyModel = new QSortFilterProxyModel{this};

  mProxyModel->setSourceModel(mVideoModel);
  setModel(mProxyModel);

  setEditTriggers(QAbstractItemView::NoEditTriggers);
  setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

  InitTableView();
  verticalHeader()->setVisible(false);

  connect(this, &QTableView::doubleClicked, this, [this](const QModelIndex& proIndex) { ReqPlay(proIndex, true); });
}

int VideoTableView::setPlayPath(const QString& path, bool bPlayInstantly) {
  const int mediasCnt = mVideoModel->setPlayPath(path);
  if (mediasCnt > 0) {
    selectRow(0);
    ReqPlay(currentIndex(), bPlayInstantly);
  }
  return mediasCnt;
}

int VideoTableView::setMediaFiles(const QString& folderPath, const QStringList& mediaFiles, bool bPlayInstantly) {
  const int mediasCnt = mVideoModel->setPlayMedias(folderPath, mediaFiles);
  if (mediasCnt > 0) {
    selectRow(0);
    ReqPlay(currentIndex(), bPlayInstantly);
  }
  return mediasCnt;
}

QModelIndex VideoTableView::previousIndex() const {
  return iteratorCore(-1);
}

QModelIndex VideoTableView::nextIndex() const {
  return iteratorCore(1);
}

QModelIndex VideoTableView::iteratorCore(int step) const {
  const int n = mVideoModel->rowCount();
  if (n == 0) {
    LOG_D("Empty media list");
    return {};
  }
  const QModelIndex proxyCur = currentIndex();
  if (!proxyCur.isValid()) {
    LOG_D("No current index");
    return {};
  }
  switch (mPlaybackMode) {
    case QMediaPlaylist::PlaybackMode::CurrentItemOnce: {
      return {};
    }
    case QMediaPlaylist::PlaybackMode::CurrentItemInLoop: {
      return proxyCur;
    }
    case QMediaPlaylist::PlaybackMode::Sequential: {
      int iterateToRow = proxyCur.row() + step;
      if (0 <= iterateToRow && iterateToRow < n) {
        return proxyCur.siblingAtRow(iterateToRow);
      } else {
        return {};
      }
    }
    case QMediaPlaylist::PlaybackMode::Loop: {
      int iterateToRowCoarse = proxyCur.row() + step;
      int iterateToRow = (iterateToRowCoarse % n + n) % n;
      return proxyCur.siblingAtRow(iterateToRow);
    }
    case QMediaPlaylist::PlaybackMode::Random: {
      static std::random_device rd;
      static std::mt19937 gen(rd());
      std::uniform_int_distribution<> dis(0, n - 1);
      int randomRow = dis(gen);
      return proxyCur.siblingAtRow(randomRow);
    }
    default:
      return {};
  }
}

// QMediaPlaylist is not usefull as tableview+model+proxymodel
void VideoTableView::PlayPreviousVideo() {
  const QModelIndex proIndex = previousIndex();
  if (!proIndex.isValid()) {
    LOG_W("No previous index");
    return;
  }
  LOG_OK_P("select", "previous: %d", proIndex.row());
  selectRow(proIndex.row());
  ReqPlay(proIndex, true);
}

void VideoTableView::PlayNextVideo() {
  const QModelIndex proIndex = nextIndex();
  if (!proIndex.isValid()) {
    LOG_W("No next index");
    return;
  }
  LOG_OK_P("select", "next: %d", proIndex.row());
  selectRow(proIndex.row());
  ReqPlay(proIndex, true);
}

void VideoTableView::ReqPlay(const QModelIndex& proIndex, bool bPlayInstantly) {
  const QModelIndex srcIndex = mProxyModel->mapToSource(proIndex);
  mVideoModel->updateDurationFields({srcIndex});
  const QString& mediaFullPath = mVideoModel->GetMediaFullPath(srcIndex);
  emit reqPlayMedia(mediaFullPath, bPlayInstantly);
}
