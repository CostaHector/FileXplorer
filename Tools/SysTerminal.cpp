#include "SysTerminal.h"
#include "Logger.h"
#include <QFileInfo>
#include <QDir>
#include <QProcess>

bool SysTerminal::operator()(const QString& path) {
  const QFileInfo fi{path};
  if (!fi.exists()) {
    LOG_D("path[%s] not exist", qPrintable(path));
    return false;
  }
  const QString pth = QDir::toNativeSeparators(fi.isFile() ? fi.absolutePath() : fi.absoluteFilePath());
  QStringList args;
  args << QString("--working-directory=%1").arg(pth);
#ifdef RUNNING_UNIT_TESTS
  return true;
#endif
#ifdef _WIN32
  LOG_W("WINDOWS not support now");
  return false;
#endif
  QProcess process;
  process.setProgram("gnome-terminal");
  process.setArguments(args);
  return process.startDetached();  // Start the process in detached mode instead of start
}
