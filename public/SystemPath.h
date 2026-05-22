#ifndef SYSTEMPATH_H
#define SYSTEMPATH_H

#include <QString>

namespace SystemPath {
bool initDirectory();
const QString& HomePath();
const QString& LocalPath();
const QString& RoamingPath();
const QString& CastStudioListPath();

inline const QString& VIDS_DATABASE() {
  static const QString path = RoamingPath() + "/VIDS_DATABASE.db";
  return path;
}

inline QString MovieDBConnection() {
  return "DBMOVIE_CONNECT";
}

inline const QString& AI_MEDIA_DUP_DATABASE() {
  static const QString path = RoamingPath() + "/DUPLICATES_DB.db";
  return path;
}

inline const QString& RECYCLE_BIN_DATABASE() {
  static const QString path = RoamingPath() + "/RECYCLE_BIN_DATABASE.db";
  return path;
}

inline const QString& PEFORMERS_DATABASE() {
  static const QString path = RoamingPath() + "/PERFORMERS_DATABASE.db";
  return path;
}

inline QString CastDBConnection() {
  return "CAST_CONNECTION";
}

inline const QString& TORRENTS_DATABASE() {
  static const QString path = RoamingPath() + "/TORRENTS_DATABASE.db";
  return path;
}

inline const QString& PRODUCTION_STUDIOS_DATABASE() {
  static const QString path = RoamingPath() + "/PRODUCTION_STUDIOS_DATABASE.db";
  return path;
}

inline const QString& GetLogFileAbsPath() {
  static QString logFilePath = SystemPath::LocalPath() + "/logs_info.log";
  return logFilePath;
}

inline const QString& GetVendorsTableFilePath() {
  static const QString path = SystemPath::CastStudioListPath() + "/VENDORS.txt";
  return path;
}

inline const QString& GetMononymVendorsListFilePath() {
  static const QString path = SystemPath::CastStudioListPath() + "/MONONYM_VENDORS.txt";
  return path;
}

inline const QString& GetActorsListFilePath() {
  static const QString path = SystemPath::CastStudioListPath() + "/ACTORS_LIST.txt";
  return path;
}

inline const QString& GetMononymActorsListFilePath() {
  static const QString path = SystemPath::CastStudioListPath() + "/MONONYM_ACTORS_LIST.txt";
  return path;
}

inline const QString& GetActorsAliasListFilePath() {
  static const QString path = SystemPath::CastStudioListPath() + "/ACTORS_ALIAS_LIST.txt";
  return path;
}

QString GetPathByApplicationDirPath(const QString& relativePath);
const QString& TORRENT_EDITOR_PROG_PATH();
} // namespace SystemPath

#endif // SYSTEMPATH_H
