#include "SystemContextMenuControl.h"
#include "PublicTool.h"
#include "PathTool.h"
#include "NotificatorMacro.h"
#include <QCoreApplication>
#include <QProcess>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QStandardPaths>

namespace SystemContextMenuControl {
QString GetTempBatFileAbsPathForWrite(const QString& fileName) {
  QString tempath{QStandardPaths::writableLocation(QStandardPaths::TempLocation)};
  const QString batFileAbsPath = QDir{tempath}.absoluteFilePath(fileName);
  return PathTool::sysPath(batFileAbsPath);
}

bool RunBatAsAdmin(const QString& batFileAbsPath) {
#ifndef _WIN32
  LOG_D("Not support on other platform except windows");
  return false;
#endif
  if (!QFile::exists(batFileAbsPath)) {
    LOG_W("[Failed] file[%s] not found", qPrintable(batFileAbsPath));
    return false;
  }
  const QString batNativePath = PathTool::sysPath(QFileInfo{batFileAbsPath}.absoluteFilePath());
  const QString powershellCommand =
      QString("Start-Process cmd -ArgumentList '/c \"%1\"' -Verb RunAs").arg(batNativePath);//
  const QStringList args{"-Command", powershellCommand};

  QProcess process;
  process.start("powershell", args);
  process.waitForFinished();

  // or QProcess::execute("powershell", args) != 0;
  if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
    const QString& stdErr = process.readAllStandardError();
    LOG_ERR_P("[Failed] Cannot run with admin privileges", "batAbsPath[%s] \n%s",
              qPrintable(batNativePath), qPrintable(stdErr));
    return false;
  }
  LOG_OK_P("[Ok] Run with admin privileges succeed", "batAbsPath[%s]", qPrintable(batFileAbsPath));
  return true;
}

bool Add() {
#ifndef _WIN32
  LOG_D("Not support on other platform except windows");
  return false;
#endif
  static const QString ADD_CONTEXT_MENU_BAT_CONTENT_TEMPLATE {
      R"(@echo off
chcp 65001 >nul
echo Now adding "%1" to Right Click Menu...
reg add "HKEY_CLASSES_ROOT\Directory\Background\shell\%1" /ve /d "%1" /f
reg add "HKEY_CLASSES_ROOT\Directory\Background\shell\%1\command" /ve /d "\"%2\" \"%%V\"" /f
echo Adding succeed！
pause)"
  };
  const QString exePath = QCoreApplication::applicationFilePath();
  const QString exeNativePath = PathTool::sysPath(exePath);
  const QString addContextMenuBatContent = ADD_CONTEXT_MENU_BAT_CONTENT_TEMPLATE.arg(PROJECT_NAME).arg(exeNativePath);

  const QString batNativeFileAbsPath = GetTempBatFileAbsPathForWrite("AddContextMenuForThisProgram.bat");
  const bool writeResult = FileTool::TextWriter(batNativeFileAbsPath, addContextMenuBatContent, QIODevice::WriteOnly | QIODevice::Text);
  if (!writeResult) {
    LOG_W("[Failed] When write into file %s", qPrintable(batNativeFileAbsPath));
    return false;
  }

  return RunBatAsAdmin(batNativeFileAbsPath);
}
bool Rmv() {
#ifndef _WIN32
  LOG_D("Not support on other platform except windows");
  return false;
#endif
  static const QString RMV_CONTEXT_MENU_BAT_CONTENT_TEMPLATE {
      R"(@echo off
chcp 65001 >nul
echo Removing Right Click Menu for "%1"...
reg delete "HKEY_CLASSES_ROOT\Directory\Background\shell\%1" /f
echo Removing succeed.
pause)"
  };
  const QString rmvContextMenuBatContent = RMV_CONTEXT_MENU_BAT_CONTENT_TEMPLATE.arg(PROJECT_NAME);

  const QString batNativeFileAbsPath = GetTempBatFileAbsPathForWrite("RmvContextMenuForThisProgram.bat");
  const bool writeResult = FileTool::TextWriter(batNativeFileAbsPath, rmvContextMenuBatContent, QIODevice::WriteOnly | QIODevice::Text);
  if (!writeResult) {
    LOG_W("[Failed] When write into file %s", qPrintable(batNativeFileAbsPath));
    return false;
  }

  return RunBatAsAdmin(batNativeFileAbsPath);
}
}
