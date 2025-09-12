#ifndef PUBLICVARIABLE_H
#define PUBLICVARIABLE_H

#include <QString>
#include <QStringList>
#include <functional>

typedef std::function<bool(QString, bool, bool)> T_IntoNewPath;
typedef std::function<bool(QString)> T_on_searchTextChanged;
typedef std::function<bool(QString)> T_on_searchEnterKey;

struct SystemPath {
  static const QString HOME_PATH;
  static const QString WORK_PATH;
  static const QString STARRED_PATH;
  static const QString VIDS_DATABASE;
  static const QString DEVICES_AND_DRIVES_DATABASE;
  static const QString AI_MEDIA_DUP_DATABASE;
  static const QString RECYCLE_BIN_DATABASE;
  static const QString PEFORMERS_DATABASE;
  static const QString TORRENTS_DATABASE;
  static const QString PRODUCTION_STUDIOS_DATABASE;
};

namespace DB_TABLE {
const QString MOVIES = "MOVIES";
const QString PERFORMERS = "PERFORMERS";
const QString TORRENTS = "TORRENTS";
const QString DISKS = "DISKS";
}  // namespace DB_TABLE

namespace TYPE_FILTER {
const QStringList AI_DUP_VIDEO_TYPE_SET = {"*.mp4", "*.m4v", "*.mov", "*.avi", "*.flv", "*.wmv", "*.mkv", "*.divx", "*.m4v", "*.mpg", "*.mpeg", "*.ts", "*.asf"};
const QStringList VIDEO_TYPE_SET = {"*.mp4", "*.m4v", "*.mov", "*.avi", "*.flv", "*.wmv", "*.mkv", "*.divx", "*.m4v", "*.mpg", "*.mpeg", "*.ts", "*.asf", "*.vob", "*.iso", "*.3gp"};
const QStringList IMAGE_TYPE_SET = {"*.jpeg", "*.jpg", "*.png", "*.tiff", "*.jfif", "*.gif", "*.webp", "*.ico", "*.svg"};
const QStringList JSON_TYPE_SET = {"*.json"};
const QStringList TEXT_TYPE_SET = {"*.json", "*.txt", "*.html", "*.md", "*.dat", "*.svg"};
const QStringList BUILTIN_COMPRESSED_TYPE_SET = {"*.qz"};
const QStringList HAR_TYPE_SET = {"*.har"};
}  // namespace TYPE_FILTER

#endif  // PUBLICVARIABLE_H
