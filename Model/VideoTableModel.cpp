#include "VideoTableModel.h"
#include "PathTool.h"
#include "PublicVariable.h"
#include "DataFormatter.h"
#include "RateHelper.h"
#include "Logger.h"
#include <QDirIterator>

const QStringList VideoTableModel::VIDEO_VERTICAL_HEAD{"File name", "Relative path", "Size", "Duration", "Rate"};

int VideoTableModel::setPlayPath(const QString& rootPath, VideoFindMode findMode) {
  if (mPlayPath == rootPath) {
    LOG_D("skip, path[%s] unchange", qPrintable(mPlayPath));
    return 0;
  }
  mPlayPath = rootPath;
  using namespace PathTool;

  QList<QFileInfo> mediaFileInfoLst;
  mediaFileInfoLst.reserve(8);
  if (findMode == VideoFindMode::INCLUDING_SUBDIRECTORY) {
    QDirIterator it{mPlayPath, TYPE_FILTER::VIDEO_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
    while (it.hasNext()) {
      it.next();
      mediaFileInfoLst.push_back(it.fileInfo());
    }
  } else {
    static const auto GetDirectMediaFileInfo = [](const QString& path, bool bSkipIfSpecial) -> QList<QFileInfo> {
      QList<QFileInfo> fiInfoLst;
      QDir dir{path, "", QDir::SortFlag::Name, QDir::Filter::Files};
      dir.setNameFilters(TYPE_FILTER::VIDEO_TYPE_SET);
      for (const QFileInfo& fi: dir.entryInfoList()) {
        if (bSkipIfSpecial && fi.size() < 10 * 1024 * 1024) {
          continue;
        }
        fiInfoLst.push_back(fi);
      }
      return fiInfoLst;
    };

    static const QStringList specialFolders{"VIDEO_TS", "videos", "vids"}; // < 10MiB
    for (const QString& specialFolderName: specialFolders) {
      const QString& specialPath = PathTool::join(mPlayPath, specialFolderName);
      if (!QFile::exists(specialPath)) {
        continue;
      }
      mediaFileInfoLst += GetDirectMediaFileInfo(specialPath, true);
    }
    mediaFileInfoLst += GetDirectMediaFileInfo(mPlayPath, false);
  }

  QList<VideoBasicInfo> videosList;
  videosList.reserve(mediaFileInfoLst.size());
  QString rel2searchItem;
  QString fileName;

  const int ROOT_PATH_N_WITH_NO_TRAILING_SLASH = mPlayPath.size();
  for (const QFileInfo& fi: mediaFileInfoLst) {
    fileName = fi.fileName();
    rel2searchItem = GetRelPathFromRootRelName(ROOT_PATH_N_WITH_NO_TRAILING_SLASH, fi.filePath(), fileName.size());
    videosList.push_back(VideoBasicInfo{fileName, rel2searchItem, fi.size(), 0, 0});
  }

  // C:/A/B/C
  // C:/A   file
  std::sort(videosList.begin(), videosList.end());
  LOG_D("%d item(s) find out under path [%s]", videosList.size(), qPrintable(mPlayPath));

  beginResetModel();
  mVideosInfo.swap(videosList);
  endResetModel();
  return mVideosInfo.size();
}

int VideoTableModel::setPlayMedias(const QString& rootPath, const QStringList& mediaFiles) {
  mPlayPath = rootPath;
  const int ROOT_PATH_N_WITH_NO_TRAILING_SLASH = mPlayPath.size();

  QList<VideoBasicInfo> videosList;
  QString rel2searchItem;
  QString fileName;

  using namespace PathTool;
  for (const QString& fileAbsPath : mediaFiles) {
    const QFileInfo fi{fileAbsPath};
    fileName = fi.fileName();
    rel2searchItem = GetRelPathFromRootRelName(ROOT_PATH_N_WITH_NO_TRAILING_SLASH, fileAbsPath, fileName.size());
    videosList.push_back(VideoBasicInfo{fileName, rel2searchItem, fi.size(), 0, 0});
  }

  beginResetModel();
  mVideosInfo.swap(videosList);
  endResetModel();
  return mVideosInfo.size();
}

QString VideoTableModel::mediaPath(const QModelIndex& ind) const {
  int row = ind.row();
  const VideoBasicInfo& item = mVideosInfo[row];
  return PathTool::GetAbsFilePathFromRootRelName(mPlayPath, item.relPath, item.fileName);
}

QVariant VideoTableModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return {};
  }
  int col = index.column();
  const VideoBasicInfo& item = mVideosInfo[index.row()];
  if (role == Qt::DisplayRole) {
    switch (col) {
      case 0:
        return item.fileName;
      case 1:
        return item.relPath;
      case 2:
        return DataFormatter::formatFileSizeGMKB(item.fileSize);
      case 3:
        return DataFormatter::formatDurationISO(item.duration);
      case 4:
        return item.rate;
      default:
        return {};
    }
  } else if (role == Qt::DecorationRole && col == 4) {
    return RateHelper::GetRatePixmap(item.rate);
  }

  return {};
}
