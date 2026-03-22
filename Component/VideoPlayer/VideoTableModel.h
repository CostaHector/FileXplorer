#ifndef VIDEOTABLEMODEL_H
#define VIDEOTABLEMODEL_H

#include "QAbstractTableModelPub.h"

struct VideoBasicInfo {
  QString fileName;
  QString relPath;
  qint64 fileSize;
  int duration;
  short rate;
  enum DataType {
    SCORE_FIELD = 0,
    FILE_NAME = 1,
    DURATION_FIELD = 2,
    FILE_SIZE = 3,
    REL_PATH = 4,
  };
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
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

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

  Qt::ItemFlags flags(const QModelIndex& index) const override {
    if (index.column() == VideoBasicInfo::SCORE_FIELD) {
      return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEditable;
    }
    return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
  }

  int setRootPath(const QString& rootPath, VideoFindMode findMode = VideoFindMode::NORMAL, bool bForce=false);
  QString rootPath() const { return mPlayPath; }
  int forceReload();
  VideoFindMode findMode() const { return mFindMode; }

  int setPlayMedias(const QString& rootPath, const QStringList& mediaFiles);
  QString GetMediaFullPath(const QModelIndex& ind) const;
  int updateDurationFields(const QModelIndexList& indexes);
  int rateSelectedMovies(const QModelIndexList& indexes, int newRate);
  int adjustRateSelectedMovies(const QModelIndexList& indexes, int delta);
  QStringList rel2fileNames(const QModelIndexList& indexes) const;
  int AfterVideoFilesNameRenamed(const QModelIndexList& indexes);

 private:
  QList<VideoBasicInfo> mVideosInfo;
  QString mPlayPath;
  VideoFindMode mFindMode{VideoFindMode::NORMAL};
  static const QStringList VIDEO_VERTICAL_HEAD;
};

#endif  // VIDEOTABLEMODEL_H
