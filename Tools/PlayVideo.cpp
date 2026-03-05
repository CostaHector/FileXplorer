#include "PlayVideo.h"
#include "PublicVariable.h"
#include "FileTool.h"
#include "Logger.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>

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
  process.setArguments({QDir::toNativeSeparators(dirPath)});
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
