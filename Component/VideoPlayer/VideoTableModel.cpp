#include "VideoTableModel.h"
#include "VideoDurationGetter.h"
#include "PathTool.h"
#include "PublicVariable.h"
#include "DataFormatter.h"
#include "RateHelper.h"
#include "Logger.h"
#include "RateHelper.h"
#include "FileTool.h"
#include <QDirIterator>

constexpr int VideoBasicInfo::DURATION_FIELD, VideoBasicInfo::SCORE_FIELD;

const QStringList VideoTableModel::VIDEO_VERTICAL_HEAD{"File name", "Relative path", "Size", "Duration", "Rate"};

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
      case VideoBasicInfo::DURATION_FIELD:
        return DataFormatter::formatDurationISO(item.duration);
      case VideoBasicInfo::SCORE_FIELD:
        return item.rate;
      default:
        return {};
    }
  } else if (role == Qt::DecorationRole && col == VideoBasicInfo::SCORE_FIELD) {
    return RateHelper::GetRatePixmap(item.rate);
  }

  return {};
}

int VideoTableModel::setRootPath(const QString& rootPath, VideoFindMode findMode, bool bForce) {
  if (mPlayPath == rootPath && !bForce) {
    LOG_D("skip, path[%s] unchange and not forceMode", qPrintable(mPlayPath));
    return 0;
  }
  mPlayPath = rootPath;
  mFindMode = findMode;
  using namespace PathTool;

  QList<QFileInfo> mediaFileInfoLst;
  mediaFileInfoLst.reserve(8);
  if (mFindMode == VideoFindMode::INCLUDING_SUBDIRECTORY) {
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
      for (const QFileInfo& fi : dir.entryInfoList()) {
        if (bSkipIfSpecial && fi.size() < 10 * 1024 * 1024) {
          continue;
        }
        fiInfoLst.push_back(fi);
      }
      return fiInfoLst;
    };

    static const QStringList specialFolders{"VIDEO_TS", "videos", "vids"};  // < 10MiB
    for (const QString& specialFolderName : specialFolders) {
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
  for (const QFileInfo& fi : mediaFileInfoLst) {
    fileName = fi.fileName();
    rel2searchItem = GetRelPathFromRootRelName(ROOT_PATH_N_WITH_NO_TRAILING_SLASH, fi.filePath(), fileName.size());
    const QString& jsonCorrespondVideo{PathTool::FileExtReplacedWithJson(fi.filePath())};
    const int scoreValue{GetRateFromJsonFile(jsonCorrespondVideo)};
    videosList.push_back(VideoBasicInfo{fileName, rel2searchItem, fi.size(), 0, (short)scoreValue});
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

int VideoTableModel::forceReload() {
  return setRootPath(rootPath(), findMode(), true);
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
  std::sort(videosList.begin(), videosList.end());

  beginResetModel();
  mVideosInfo.swap(videosList);
  endResetModel();
  return mVideosInfo.size();
}

QString VideoTableModel::GetMediaFullPath(const QModelIndex& ind) const {
  const int row = ind.row();
  if (row < 0 || row >= rowCount()) {
    LOG_W("row[%d] out of range", row);
    return "";
  }
  const VideoBasicInfo& item = mVideosInfo[row];
  return PathTool::GetAbsFilePathFromRootRelName(rootPath(), item.relPath, item.fileName);
}

int VideoTableModel::updateDurationFields(const QModelIndexList& indexes) {
  if (indexes.isEmpty()) {
    return 0;
  }
  int minRow{INT_MAX}, maxRow{-1};
  int affectedRows{0};

  VideoDurationGetter mi;
  if (!mi.StartToGet()) {
    return -1;
  }
  for (const QModelIndex& ind : indexes) {
    const int row = ind.row();
    if (row < 0 || row >= rowCount()) {
      LOG_W("row[%d] out of range", row);
      continue;
    }
    if (row > maxRow) {
      maxRow = row;
    }
    if (row < minRow) {
      minRow = row;
    }
    auto& item = mVideosInfo[ind.row()];
    const QString& mediaFullPath = GetMediaFullPath(ind);
    item.duration = mi.GetLengthQuick(mediaFullPath);
    ++affectedRows;
  }
  if (maxRow < 0 || minRow > maxRow) {
    return 0;
  }
  const QModelIndex& frontInd = sibling(minRow, VideoBasicInfo::DURATION_FIELD, {});
  const QModelIndex& backInd = sibling(maxRow, VideoBasicInfo::DURATION_FIELD, {});
  emit dataChanged(frontInd, backInd, {Qt::DisplayRole});
  LOG_D("%d in %d duration field in row[%d, %d] get updated", affectedRows, indexes.size(), minRow, maxRow);
  return affectedRows;
}

int VideoTableModel::rateSelectedMovies(const QModelIndexList& indexes, int newRate) {
  if (indexes.isEmpty()) {
    return 0;
  }
  int minRow{INT_MAX}, maxRow{-1};
  int affectedRows{0};
  for (const QModelIndex& ind : indexes) {
    const int row = ind.row();
    if (row < 0 || row >= rowCount()) {
      LOG_W("row[%d] out of range", row);
      continue;
    }
    const QString mediaPath{GetMediaFullPath(ind)};
    if (!RateHelper::RateMovie(mediaPath, newRate)) {
      LOG_W("Rate[%s] failed", qPrintable(mediaPath));
      continue;
    }
    mVideosInfo[row].rate = newRate;
    ++affectedRows;
    if (row > maxRow) {
      maxRow = row;
    }
    if (row < minRow) {
      minRow = row;
    }
  }
  if (maxRow < 0 || minRow > maxRow) {
    return 0;
  }
  const QModelIndex& frontInd = sibling(minRow, VideoBasicInfo::SCORE_FIELD, {});
  const QModelIndex& backInd = sibling(maxRow, VideoBasicInfo::SCORE_FIELD, {});
  emit dataChanged(frontInd, backInd, {Qt::DisplayRole});
  LOG_D("%d in %d rate field in row[%d, %d] get updated", affectedRows, indexes.size(), minRow, maxRow);
  return affectedRows;
}

QStringList VideoTableModel::rel2fileNames(const QModelIndexList& indexes) const {
  // full: "/home/to/a.json"
  // root: "/home"
  // relPath: "/to/"
  // name: "a.json"
  // rel2fileNames: "to/a.json"
  QStringList relativePaths2FileName;
  relativePaths2FileName.reserve(indexes.size());
  for (const QModelIndex& index : indexes) {
    int row = index.row();
    if (row < 0 || row >= rowCount()) {
      LOG_W("row: %d out of range", row);
      return {};
    }
    const VideoBasicInfo& item = mVideosInfo[row];
    const QString& noPreSlash = item.relPath.mid(item.relPath.isEmpty() ? 0 : 1);
    relativePaths2FileName.push_back(noPreSlash + item.fileName);
  }
  return relativePaths2FileName;
}

int VideoTableModel::AfterVideoFilesNameRenamed(const QModelIndexList& indexes) {
  const auto rowElementsRmv = [this](int beg, int end) { mVideosInfo.erase(mVideosInfo.begin() + beg, mVideosInfo.begin() + end); };
  return onRowsRemoved(indexes, rowElementsRmv);
}

int VideoTableModel::GetRateFromJsonFile(const QString& jsonFullPath, int defaultRateValue) {
  bool bReadResult{false};
  QByteArray contents{FileTool::ByteArrayReader(jsonFullPath, &bReadResult)};
  if (!bReadResult) {
    return defaultRateValue;
  }
  int rateIndex = contents.indexOf(R"("Rate":)");
  if (rateIndex == -1) {
    return defaultRateValue;
  }
  // 跳过"Rate":7个字符
  int valuePos = rateIndex + 7;
  while (valuePos < contents.size() && (contents[valuePos] == ' ' || contents[valuePos] == '\t')) {
    ++valuePos;
  }

  if (valuePos >= contents.size() || contents[valuePos] < '0' || contents[valuePos] > '9') {
    return defaultRateValue; // Todo: llt cover this line
  }
  int rate = 0;
  while (valuePos < contents.size() && contents[valuePos] >= '0' && contents[valuePos] <= '9') {
    rate = rate * 10 + (contents[valuePos] - '0');
    ++valuePos;
  }
  return rate;
}
