#ifndef VIDEOTABLEVIEW_H
#define VIDEOTABLEVIEW_H

#include "CustomTableView.h"
#include "VideoTableModel.h"
#include <QMediaPlaylist>
#include <QSortFilterProxyModel>

class VideoTableView : public CustomTableView {
  Q_OBJECT
public:
  explicit VideoTableView(QWidget* parent = nullptr);
  void setPlaybackMode(QMediaPlaylist::PlaybackMode mode) { mPlaybackMode = mode; }
  int setPlayPath(const QString& path, bool bPlayInstantly);
  int setMediaFiles(const QString& folderPath, const QStringList& mediaFiles, bool bPlayInstantly);
  void PlayPreviousVideo();
  void PlayNextVideo();
  int onRateSelectedMovies(int newRate);
  int onAdjustSelectedMoviesRate(int delta);
  int onRenameJsonAndRelatedReplace();
  int onRenameJsonAndRelatedInsert();
  int onUpdateDurationFields();

signals:
  void reqPlayMedia(QString mediaUrl, bool bPlayInstantly);

private:
  QModelIndexList selectedRowsSource() const;

  QModelIndex previousIndex() const;
  QModelIndex nextIndex() const;
  QModelIndex iteratorCore(int step) const;
  void ReqPlay(const QModelIndex& proIndex, bool bPlayInstantly);

  QAction* mRenameVideoRelatedFilesReplace{nullptr};
  QAction* mRenameVideoRelatedFilesInsert{nullptr};
  QAction* mUpdateDurations{nullptr};
  QAction* mReloadCurrentPath{nullptr};

  VideoTableModel* mVideoModel{nullptr};
  QSortFilterProxyModel* mProxyModel{nullptr};
  QMediaPlaylist::PlaybackMode mPlaybackMode{QMediaPlaylist::PlaybackMode::CurrentItemOnce};
};

#endif // VIDEOTABLEVIEW_H
