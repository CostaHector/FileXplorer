#include "PlayVideo.h"
#include "PublicVariable.h"
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QString>
#include <QUrl>

bool PlayADir(const QString& dirPath) {
  QProcess process;
#ifdef _WIN32
  process.setProgram("C:/Program Files/DAUM/PotPlayer/PotPlayerMini64.exe");
#else
  process.setProgram("xdg-open");
#endif
  process.setArguments({QDir::toNativeSeparators(dirPath)});
  process.startDetached();  // Start the process in detached mode instead of start
  qWarning("Play folder program[%s], args[%s]...", qPrintable(process.program()), qPrintable(process.arguments().join(',')));
  return true;
}

bool on_ShiftEnterPlayVideo(const QString& path) {
  if (!QFile::exists(path)) {
    qWarning("path[%s] not exist skip play", qPrintable(path));
    return false;
  }
  const QFileInfo fi(path);
  if (fi.isDir()) {
    return PlayADir(path);
  }
  qWarning("Play file[%s]...", qPrintable(path));
  return QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absoluteFilePath()));
}
