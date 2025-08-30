#include "SysTerminal.h"
#include "PathTool.h"
#include "PublicVariable.h"
#include "MemoryKey.h"
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QUrl>
#include <QProcess>

bool SysTerminal::operator()(const QString& path) {
  const QFileInfo fi{path};
  if (!fi.exists()) {
    qDebug("path[%s] not exist", qPrintable(path));
    return false;
  }
  const QString pth = QDir::toNativeSeparators(fi.isFile() ? fi.absolutePath() : fi.absoluteFilePath());
#ifdef _WIN32
  qWarning("WINDOWS not support now");
  return false;
#else
  QProcess process;
  QStringList args;
  process.setProgram("gnome-terminal");
  args << QString("--working-directory=%1").arg(pth);
  process.setArguments(args);
  return process.startDetached();  // Start the process in detached mode instead of start
#endif
}
