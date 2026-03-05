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
  int setPlayPath(const QString& path);
  int setMediaFiles(const QString& folderPath, const QStringList& mediaFiles);
  void PlayPreviousVideo();
  void PlayNextVideo();

signals:
  void reqPlayMedia(QString mediaUrl, bool bPlayInstantly);

private:
  QModelIndex previousIndex() const;
  QModelIndex nextIndex() const;
  QModelIndex iteratorCore(int step) const;
  void ReqPlay(const QModelIndex& proIndex, bool bPlayInstantly);

  VideoTableModel* mVideoModel{nullptr};
  QSortFilterProxyModel* mProxyModel{nullptr};
  QMediaPlaylist::PlaybackMode mPlaybackMode{QMediaPlaylist::PlaybackMode::CurrentItemOnce};
};

#endif // VIDEOTABLEVIEW_H
