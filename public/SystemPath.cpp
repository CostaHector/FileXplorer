#include "SystemPath.h"
#include "Logger.h"
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QStandardPaths>

namespace SystemPath {
bool initDirectory() {
  QCoreApplication::setOrganizationName(ORGANIZATION_NAME);
  QCoreApplication::setApplicationName(APPLICATION_NAME);

  QString roamingPath = RoamingPath();
  if (roamingPath.isEmpty()) {
    return false;
  }
  QString castStudioListPath = CastStudioListPath();
  if (castStudioListPath.isEmpty()) {
    return false;
  }
  return true;
}

bool SafeCreateADirectory(const QString& path) {
  QFileInfo pathFi{path};
  if (pathFi.exists()) {
    if (pathFi.isDir()) {
      return true;
    }
    LOG_C("Path[%s] occupied by none-directory item", qPrintable(path));
    return false;
  }
  if (!QDir{}.mkpath(path)) {
    LOG_C("Create path[%s] failed.", qPrintable(path));
    return false;
  }
  LOG_D("Create path[%s] succeed.", qPrintable(path));
  return true;
}

const QString& HomePath() {
  static const QString path = QDir::homePath();
  return path;
}

const QString& LocalPath() {
  static const QString path{[](){
    const QString localPath{QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)};
    return SafeCreateADirectory(localPath) ? localPath : "";
  }()};
  return path;
}

// roaming path
const QString& RoamingPath() {
  static const QString path{[](){
    const QString roamingPath{QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)};
    return SafeCreateADirectory(roamingPath) ? roamingPath : "";
  }()};
  return path;
}

const QString& CastStudioListPath() {
  static const QString path{[](){
    const QString castStudioListPth{RoamingPath() + "/CastStudioList"};
    return SafeCreateADirectory(castStudioListPth) ? castStudioListPth : "";
  }()};
  return path;
}

const QString& GetVendorsTableFilePath() {
  static const QString path = SystemPath::CastStudioListPath() + "/VENDORS.txt";
  return path;
}

const QString& GetMononymVendorsListFilePath() {
  static const QString path = SystemPath::CastStudioListPath() + "/MONONYM_VENDORS.txt";
  return path;
}

const QString& GetActorsListFilePath() {
  static const QString path = SystemPath::CastStudioListPath() + "/ACTORS_LIST.txt";
  return path;
}

const QString& GetMononymActorsListFilePath() {
  static const QString path = SystemPath::CastStudioListPath() + "/MONONYM_ACTORS_LIST.txt";
  return path;
}

const QString& GetActorsAliasListFilePath() {
  static const QString path = SystemPath::CastStudioListPath() + "/ACTORS_ALIAS_LIST.txt";
  return path;
}

QString GetPathByApplicationDirPath(const QString& relativePath) {
  static const QDir appDir{QCoreApplication::applicationDirPath()};
  // QFileInfo{"."}.absoluteFilePath() may be same as appDir.
  // "." can be modified by QDir::setCurrent("newpwd"),
  // while appDir is not influenced by setCurrent. it reamins exe file path.
  return QDir::cleanPath(appDir.absoluteFilePath(relativePath));
}

const QString& TORRENT_EDITOR_PROG_PATH() {
  constexpr char TORRENT_EDITOR_PROG_REL_PATH[]{
#ifdef _WIN32
      "../../../torrent-file-editor/build/Debug/torrent-file-editor.exe"
#else
      "../../../torrent-file-editor/build/Debug/torrent-file-editor"
#endif
  };
  static const QString torrentEditorProg{GetPathByApplicationDirPath(TORRENT_EDITOR_PROG_REL_PATH)};
  return torrentEditorProg;
}

} // namespace SystemPath
