#include "Logger.h"
#include "MemoryKey.h"
#include "NotificatorMacro.h"
#include "PathTool.h"
#include "FileTool.h"
#include "PublicVariable.h"

#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDateTime>

std::unique_ptr<FILE, decltype(Logger::CloseLogFile)*> Logger::mLogFILEStreamUniquePtr{nullptr, CloseLogFile};
LOG_LVL_E Logger::m_autoFflushLevel = LOG_LVL_E::W;
LOG_LVL_E Logger::m_printLevel = LOG_LVL_E::D;
constexpr char Logger::CONSTANT_LOG_FILE_NAME[];

const QString& Logger::GetLogFileAbsPath() {
  static QString logFilePath = QDir::homePath() + "/" + PROJECT_NAME + "/" + CONSTANT_LOG_FILE_NAME;
  return logFilePath;
}

FILE* Logger::SwitchLogToALocalFile(const QString& logFileAbsPath) {
  QByteArray logOutputToPath = logFileAbsPath.toLocal8Bit();
  FILE* pTempFile = fopen(logOutputToPath.constData(), "a");
  if (pTempFile == nullptr) {
#ifdef QT_DEBUG
    fprintf(stdout, "ERROR: Cannot open log file, fallback to stdout\n");
#else // in release mode log file should exist
    fprintf(stderr, "ERROR: Cannot open log file, fallback to stdout\n");
    fflush(stderr);
#endif
    return stdout;
  }
  decltype(mLogFILEStreamUniquePtr) new_ptr(pTempFile, CloseLogFile);
  mLogFILEStreamUniquePtr = std::move(new_ptr);  // 先转移所有权
  return pTempFile;
}

FILE* Logger::GetFILEStream() {
#ifdef QT_DEBUG // in debug mode, output to std flow
  return stdout;
#else // otherwise, output to append to local file
  return SwitchLogToALocalFile(GetLogFileAbsPath());
#endif
}

bool Logger::CloseLogFile(FILE* pFile) {
  if (pFile == nullptr || pFile == stdout || pFile == stderr) return false;
  LOG_I("Local log file[%s] opened before closing now...", qPrintable(GetLogFileAbsPath()));
  fflush(pFile);
  fclose(pFile);
  return true;
}

bool Logger::AgingLogFiles(const QString& logFileAbsPath, const int AGING_FILE_ABOVE_B, QString* pAgedLogFileName) {
  const bool isLogFileOpened {mLogFILEStreamUniquePtr != nullptr};
  // check: skip if log file not exist
  QFile oldFile{logFileAbsPath};
  if (!oldFile.exists()) {
    LOG_WARN_NP("[Abort] Logger file not exists", qPrintable(logFileAbsPath));
    return false;
  }

  // check: skip if file size below threshold
  if (oldFile.size() < AGING_FILE_ABOVE_B) { // by default 200 MiB
    LOG_WARN_P("[Skip] Logger file too less", "size(%s)=%lld<%d byte(s)",
               qPrintable(logFileAbsPath), oldFile.size(), AGING_FILE_ABOVE_B);
    return true;
  }

  // 1. close current file manually right now. (if log file opened)
  if (isLogFileOpened) {
    mLogFILEStreamUniquePtr.reset(nullptr);
  }

  // 2. get aged file name
  QString logsUnder;
  const QString curLogName = PathTool::GetPrepathAndFileName(logFileAbsPath, logsUnder);
  const QString agedName{QDateTime::currentDateTime().toString("yyyy-MM-dd_hh_mm_ss_") + curLogName};

  // 3. for test usage. don't forget to check it in unittest
  if (pAgedLogFileName != nullptr) {
    *pAgedLogFileName = agedName;
  }

  // 3. rename log file to aged log file
  bool agedResult = QDir{logsUnder}.rename(curLogName, agedName);
  if (!agedResult) {
    LOG_CRIT_P("[Abort] Aging log file failed", "[%s]->[%s] under path[%s]",
               qPrintable(curLogName), qPrintable(agedName), qPrintable(logsUnder));
  } else {
    LOG_WARN_P("[Ok] Aging log file succeed", "[%s]->[%s] under path[%s]",
               qPrintable(curLogName), qPrintable(agedName), qPrintable(logsUnder));
  }
  // when rename log file failed, roll back to the constant name log file.
  // when rename log file succeed, also need to open the constant name log file.
  if (isLogFileOpened) {
    out() = SwitchLogToALocalFile(logFileAbsPath);
  }
  return true;
}

bool Logger::OpenLogFile() {
  fflush(Logger::out());
  const QString& logAbsPath = GetLogFileAbsPath();
  if (!QFile::exists(logAbsPath)) {
    LOG_W("log file[%s] not exist", qPrintable(logAbsPath));
    return false;
  }
  return FileTool::OpenLocalFileUsingDesktopService(logAbsPath);
}

bool Logger::OpenLogFolder() {
  const QString& logAbsPath = GetLogFileAbsPath();
  const QString logsFolderPath = QFileInfo{logAbsPath}.absolutePath();
  if (!QFile::exists(logsFolderPath)) {
    LOG_W("log file located in folder[%s] not exist", qPrintable(logsFolderPath));
    return false;
  }
#ifdef RUNNING_UNIT_TESTS
  return true;
#endif
  return FileTool::OpenLocalFileUsingDesktopService(logsFolderPath);
}

void Logger::SetPrintLevel(LOG_LVL_E newLevel) {
  LOG_W("Log print level set from [%d] to [%d]", static_cast<int>(m_printLevel), static_cast<int>(newLevel));
  m_printLevel = newLevel;
}

void Logger::SetAutoFlushAllLevel(bool allLevelChecked) {
  m_autoFflushLevel = allLevelChecked ? LOG_LVL_E::D : LOG_LVL_E::W;
  Configuration().setValue(MemoryKey::ALL_LOG_LEVEL_AUTO_FFLUSH.name, allLevelChecked);
}
