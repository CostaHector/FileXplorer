#include "PlayVideo.h"
#include "FileTool.h"
#include "Logger.h"
#include "PathTool.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>

bool isDvdDirectory(const QString& rawPath, QString& stdDvdPath) {
  if (!QFileInfo{rawPath}.isDir()) {
    return false;
  }
  if (PathTool::fileName(rawPath).compare("VIDEO_TS", Qt::CaseInsensitive) == 0) {
    stdDvdPath = rawPath;
    return true;
  }
  if (QFileInfo{PathTool::Path2Join(rawPath, "VIDEO_TS")}.isDir()) {
    stdDvdPath = PathTool::Path2Join(rawPath, "VIDEO_TS");
    return true;
  }
  return false;
}

bool getPrimaryIFOFile(const QString& stdDvdPath, QString& primaryIfoFile) {
  const QDir dvdDir{stdDvdPath, "*.ifo", QDir::SortFlag::Size, QDir::Filter::Files}; // size in descending
  if (dvdDir.count() > 0) {
    return false;
  }
  primaryIfoFile = PathTool::Path2Join(stdDvdPath, dvdDir.entryList().front());
  return true;
}

bool PlayADir(const QString& dirPath) {
#ifdef RUNNING_UNIT_TESTS
  return true;
#endif
  QProcess process;
#ifdef _WIN32
  process.setProgram("C:/Program Files/DAUM/PotPlayer/PotPlayerMini64.exe");
#else
  process.setProgram("xdg-open");
#endif
  QString playArg{dirPath};

  QString stdDvdPath;
  if (isDvdDirectory(dirPath, stdDvdPath)) {
    QString primaryIfoFile;
    if (getPrimaryIFOFile(stdDvdPath, primaryIfoFile)) {
      playArg.swap(primaryIfoFile);
    }
  }
  process.setArguments({QDir::toNativeSeparators(playArg)});

  process.startDetached();  // Start the process in detached mode instead of start
  LOG_W("Play folder program[%s], args[%s]...", qPrintable(process.program()), qPrintable(process.arguments().join(',')));
  return true;
}

bool on_ShiftEnterPlayVideo(const QString& path) {
  if (!QFile::exists(path)) {
    LOG_W("path[%s] not exist skip play", qPrintable(path));
    return false;
  }
  const QFileInfo fi(path);
  if (fi.isDir()) {
    return PlayADir(path);
  }
  LOG_W("Play file[%s]...", qPrintable(path));
  return FileTool::OpenLocalFileUsingDesktopService(fi.absoluteFilePath());
}
