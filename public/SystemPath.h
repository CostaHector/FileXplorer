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

const QString& GetVendorsTableFilePath();
const QString& GetMononymVendorsListFilePath();
const QString& GetActorsListFilePath();
const QString& GetMononymActorsListFilePath();
const QString& GetActorsAliasListFilePath();

QString GetPathByApplicationDirPath(const QString& relativePath);
const QString& TORRENT_EDITOR_PROG_PATH();
} // namespace SystemPath

#endif // SYSTEMPATH_H
