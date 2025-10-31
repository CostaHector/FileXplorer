#ifndef PUBLICVARIABLE_H
#define PUBLICVARIABLE_H

#include <QString>
#include <QStringList>
#include <functional>

typedef std::function<bool(QString, bool, bool)> T_IntoNewPath;
typedef std::function<bool(QString)> T_on_searchTextChanged;
typedef std::function<bool(QString)> T_on_searchEnterKey;

namespace SystemPath {
const QString& HOME_PATH();

inline const QString& WORK_PATH() {
  static const QString path = HOME_PATH() + "/" + PROJECT_NAME;
  return path;
}

inline const QString& STARRED_PATH() {
  static const QString path = HOME_PATH() + "/Documents";
  return path;
}

inline const QString& VIDS_DATABASE() {
  static const QString path = WORK_PATH() + "/VIDS_DATABASE.db";
  return path;
}

inline const QString& AI_MEDIA_DUP_DATABASE() {
  static const QString path = WORK_PATH() + "/DUPLICATES_DB.db";
  return path;
}

inline const QString& RECYCLE_BIN_DATABASE() {
  static const QString path = WORK_PATH() + "/RECYCLE_BIN_DATABASE.db";
  return path;
}

inline const QString& PEFORMERS_DATABASE() {
  static const QString path = WORK_PATH() + "/PERFORMERS_DATABASE.db";
  return path;
}

inline const QString& TORRENTS_DATABASE() {
  static const QString path = WORK_PATH() + "/TORRENTS_DATABASE.db";
  return path;
}

inline const QString& PRODUCTION_STUDIOS_DATABASE() {
  static const QString path = WORK_PATH() + "/PRODUCTION_STUDIOS_DATABASE.db";
  return path;
}
}

namespace DB_TABLE {
const QString MOVIES = "MOVIES";
const QString PERFORMERS = "PERFORMERS";
const QString TORRENTS = "TORRENTS";
const QString DISKS = "DISKS";
}  // namespace DB_TABLE

namespace TYPE_FILTER {
const QStringList AI_DUP_VIDEO_TYPE_SET = {"*.mp4", "*.m4v", "*.mov", "*.avi", "*.flv", "*.wmv", "*.mkv", "*.divx", "*.m4v", "*.mpg", "*.mpeg", "*.ts", "*.asf"};
const QStringList VIDEO_TYPE_SET = {"*.mp4", "*.m4v", "*.mov", "*.avi", "*.flv", "*.wmv", "*.mkv", "*.divx", "*.m4v", "*.mpg", "*.mpeg", "*.ts", "*.asf", "*.vob", "*.iso", "*.3gp", "*.webm"};
const QStringList IMAGE_TYPE_SET = {"*.jpeg", "*.jpg", "*.png", "*.tiff", "*.jfif", "*.gif", "*.webp", "*.ico", "*.svg", "*.avif"};
const QStringList JSON_TYPE_SET = {"*.json"};
const QStringList TEXT_TYPE_SET = {"*.json", "*.txt", "*.html", "*.md", "*.dat"};
const QStringList NON_BINARY_SET = {"*.json", "*.txt", "*.html", "*.md", "*.dat", "*.svg"};
const QStringList BUILTIN_COMPRESSED_TYPE_SET = {"*.qz"};
const QStringList HAR_TYPE_SET = {"*.har"};
bool isDotExtVideo(const QString& dotExt);
bool isDotExtImage(const QString& dotExt);
bool isDotExtCompressed(const QString& dotExt);
}  // namespace TYPE_FILTER

#endif  // PUBLICVARIABLE_H
