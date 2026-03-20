#include "VideoTableView.h"
#include "NotificatorMacro.h"
#include "RateActions.h"
#include "BatchRenameBy.h"
#include <random>

VideoTableView::VideoTableView(QWidget* parent) : CustomTableView{"VIEDO_TABLE_VIEW", parent} {
  mVideoModel = new VideoTableModel{this};
  mProxyModel = new QSortFilterProxyModel{this};

  mProxyModel->setSourceModel(mVideoModel);
  setModel(mProxyModel);

  setEditTriggers(QAbstractItemView::SelectedClicked);
  setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

  InitTableView();
  verticalHeader()->setVisible(false);

  const auto& rateInst = RateActions::GetInst(RateActions::RateRequestFrom::VIDEO_TABLE_VIEW);
  {
    mRenameVideoRelatedFilesReplace = new (std::nothrow) QAction(QIcon(":img/RENAME"), tr("Rename related(replace)"), this);
    mRenameVideoRelatedFilesReplace->setShortcutVisibleInContextMenu(true);
    mRenameVideoRelatedFilesReplace->setToolTip(
        QString("<b>%1 (%2)</b><br/>Rename selected video file(s) and associated files by replacing a substring in the file names.")  //
            .arg(mRenameVideoRelatedFilesReplace->text())
            .arg(mRenameVideoRelatedFilesReplace->shortcut().toString()));

    mRenameVideoRelatedFilesInsert = new (std::nothrow) QAction(QIcon(":img/NAME_STR_INSERTER_PATH"), tr("Rename related(Insert)"), this);
    mRenameVideoRelatedFilesInsert->setShortcutVisibleInContextMenu(true);
    mRenameVideoRelatedFilesInsert->setToolTip(
        QString("<b>%1 (%2)</b><br/>Rename selected video file(s) and associated files by inserting a string into the file names.")  //
            .arg(mRenameVideoRelatedFilesInsert->text())
            .arg(mRenameVideoRelatedFilesInsert->shortcut().toString()));

    mReloadCurrentPath = new (std::nothrow) QAction(QIcon(":img/REFRESH_THIS_PATH"), tr("Refresh current path"), this);
    mReloadCurrentPath->setShortcutVisibleInContextMenu(true);
    mReloadCurrentPath->setToolTip(QString("<b>%1 (%2)</b><br/>Force reload the file list for the current directory from disk.")  //
                                       .arg(mReloadCurrentPath->text())
                                       .arg(mReloadCurrentPath->shortcut().toString()));

    QList<QAction*> acts;
    acts.reserve(20);
    acts += rateInst.GetActionGroup()->actions();
    acts.push_back(NewSeperatorAction(this));
    acts += rateInst.GetAdjustRateActions();
    acts.push_back(NewSeperatorAction(this));
    acts.push_back(mRenameVideoRelatedFilesReplace);
    acts.push_back(mRenameVideoRelatedFilesInsert);
    acts.push_back(NewSeperatorAction(this));
    acts.push_back(mReloadCurrentPath);
    PushFrontExclusiveActions(acts);
  }

  connect(&rateInst, &RateActions::RateMovieReq, this, &VideoTableView::onRateSelectedMovies);
  connect(&rateInst, &RateActions::AdjustRateMovieReq, this, &VideoTableView::onAdjustSelectedMoviesRate);
  connect(mRenameVideoRelatedFilesReplace, &QAction::triggered, this, &VideoTableView::onRenameJsonAndRelatedReplace);
  connect(mRenameVideoRelatedFilesInsert, &QAction::triggered, this, &VideoTableView::onRenameJsonAndRelatedInsert);
  connect(mReloadCurrentPath, &QAction::triggered, mVideoModel, &VideoTableModel::forceReload);
  connect(this, &QTableView::doubleClicked, this, [this](const QModelIndex& proIndex) { ReqPlay(proIndex, true); });
}

int VideoTableView::setPlayPath(const QString& path, bool bPlayInstantly) {
  const int mediasCnt = mVideoModel->setRootPath(path);
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

QModelIndexList VideoTableView::selectedRowsSource() const {
  QModelIndexList proxyIndexes{selectionModel()->selectedRows()};
  QModelIndexList srcIndexes;
  srcIndexes.reserve(proxyIndexes.size());
  for (const QModelIndex& proxyIndex : proxyIndexes) {
    srcIndexes.push_back(mProxyModel->mapToSource(proxyIndex));
  }
  return srcIndexes;
}

int VideoTableView::onRateSelectedMovies(int newRate) {
  const QModelIndexList& srcIndexes{selectedRowsSource()};
  if (srcIndexes.isEmpty()) {
    LOG_INFO_NP("Skip rate", "no row selected");
    return 0;
  }
  const int succeedCnt{mVideoModel->rateSelectedMovies(srcIndexes, newRate)};
  const int totalRow{srcIndexes.size()};
  LOG_OE_P(succeedCnt == totalRow, "Rate selection", "%d/%d row(s) succeed", succeedCnt, totalRow);
  return succeedCnt;
}

int VideoTableView::onAdjustSelectedMoviesRate(int delta) {
  if (delta == 0) {
    LOG_INFO_NP("Skip adjust rate", "delta=0");
    return 0;
  }
  const QModelIndexList& srcIndexes{selectedRowsSource()};
  if (srcIndexes.isEmpty()) {
    LOG_INFO_NP("Skip adjust rate", "no row selected");
    return 0;
  }
  const int succeedCnt{mVideoModel->adjustRateSelectedMovies(srcIndexes, delta)};
  const int totalRow{srcIndexes.size()};
  LOG_OE_P(succeedCnt == totalRow, "Adjust rate selection", "%d/%d row(s) succeed adjust[%1]", succeedCnt, totalRow, delta);
  return succeedCnt;
}

int VideoTableView::onRenameJsonAndRelatedReplace() {
  const QModelIndexList& srcIndexes{selectedRowsSource()};
  if (srcIndexes.isEmpty()) {
    LOG_INFO_NP("Skip rename(replace)", "no row selected");
    return 0;
  }

  const QString& videoLocatedInPath{mVideoModel->rootPath()};
  const QStringList& videoFileNames{mVideoModel->rel2fileNames(srcIndexes)};
  const int relatedFilesCnt{BatchRenameBy::ReplaceBySpecifiedJson(videoLocatedInPath, videoFileNames)};
  if (relatedFilesCnt <= 0) {
    return 0;
  }

  const int removeRowCnt{mVideoModel->AfterVideoFilesNameRenamed(srcIndexes)};
  return relatedFilesCnt;
}

int VideoTableView::onRenameJsonAndRelatedInsert() {
  const QModelIndexList& srcIndexes{selectedRowsSource()};
  if (srcIndexes.isEmpty()) {
    LOG_INFO_NP("Skip rename(insert)", "no row selected");
    return 0;
  }

  const QString& videoLocatedInPath{mVideoModel->rootPath()};
  const QStringList& videoFileNames{mVideoModel->rel2fileNames(srcIndexes)};
  const int relatedFilesCnt{BatchRenameBy::InsertBySpecifiedJson(videoLocatedInPath, videoFileNames)};
  if (relatedFilesCnt <= 0) {
    return 0;
  }

  const int removeRowCnt{mVideoModel->AfterVideoFilesNameRenamed(srcIndexes)};
  return relatedFilesCnt;
}
