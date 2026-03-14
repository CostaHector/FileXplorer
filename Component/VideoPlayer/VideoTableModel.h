#ifndef VIDEOTABLEMODEL_H
#define VIDEOTABLEMODEL_H

#include "QAbstractTableModelPub.h"

struct VideoBasicInfo {
  QString fileName;
  QString relPath;
  qint64 fileSize;
  int duration;
  static constexpr int DURATION_FIELD = 3;
  short rate;
  static constexpr int SCORE_FIELD = 4;
  bool operator<(const VideoBasicInfo& rhs) const { return relPath < rhs.relPath || (relPath == rhs.relPath && fileName < rhs.fileName); }
};

class VideoTableModel : public QAbstractTableModelPub {
 public:
  enum class VideoFindMode {
    NORMAL = 0,                  // direct files and special folder, too small files in special folder will be ignored
    INCLUDING_SUBDIRECTORY = 1,  // all files
  };
  using QAbstractTableModelPub::QAbstractTableModelPub;

  int rowCount(const QModelIndex& /*parent*/ = {}) const override { return mVideosInfo.size(); }
  int columnCount(const QModelIndex& /*parent*/ = {}) const override { return VIDEO_VERTICAL_HEAD.size(); }

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
    if (role == Qt::TextAlignmentRole) {
      if (orientation == Qt::Vertical) {
        return Qt::AlignRight;
      }
    }
    if (role == Qt::DisplayRole) {
      if (0 <= section && section < columnCount() && orientation == Qt::Orientation::Horizontal) {
        return VIDEO_VERTICAL_HEAD[section];
      }
      return section + 1;
    }
    return QAbstractTableModel::headerData(section, orientation, role);
  }

  Qt::ItemFlags flags(const QModelIndex& /*index*/) const override { return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable; }

  int setPlayPath(const QString& rootPath, VideoFindMode findMode = VideoFindMode::NORMAL);
  QString GetPlayPath() const { return mPlayPath; }
  int setPlayMedias(const QString& rootPath, const QStringList& mediaFiles);
  QString GetMediaFullPath(const QModelIndex& ind) const;
  int updateDurationFields(const QModelIndexList& indexes);
  int rateSelectedMovies(const QModelIndexList& indexes, int newRate);

 private:
  QList<VideoBasicInfo> mVideosInfo;
  QString mPlayPath;
  static const QStringList VIDEO_VERTICAL_HEAD;
};

#endif  // VIDEOTABLEMODEL_H
