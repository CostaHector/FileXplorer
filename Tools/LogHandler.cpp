#include "LogHandler.h"
#include "public/PublicVariable.h"
#include "public/MemoryKey.h"
#include "Actions/LogActions.h"

#include <QDesktopServices>
#include <QMessageLogContext>
#include <QDateTime>
#include <QUrl>

QtMsgType LogHandler::OUTPUT_LOG_LEVEL = QtWarningMsg;

QString LogHandler::mLogFolderPath;
bool LogHandler::mFlushLogInBufferInstantly{false};

QFile LogHandler::mLogFile;
QTextStream LogHandler::mLogTextStream(&mLogFile);

LogHandler::LogHandler(QObject* parent, const QString& logPath) : QObject{parent} {
  const bool debugLvel{PreferenceSettings().value(MemoryKey::LOG_DEVEL_DEBUG.name, MemoryKey::LOG_DEVEL_DEBUG.v).toBool()};
  OUTPUT_LOG_LEVEL = debugLvel ? QtDebugMsg : QtWarningMsg;
  if (logPath.isEmpty()) {
    QString logFileName;
#ifdef _WIN32
    logFileName = PreferenceSettings().value(MemoryKey::WIN32_RUNLOG.name).toString();
#else
    logFileName = PreferenceSettings().value(MemoryKey::LINUX_RUNLOG.name).toString();
#endif
    mLogFolderPath = logFileName + "/logs_info.log";
  } else {
    mLogFolderPath = logPath;
  }

  mLogFile.setFileName(mLogFolderPath);
  if (!mLogFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
    qCritical("Log message CANNOT redirect to file[%s]", qPrintable(mLogFile.fileName()));
    return;
  }
#ifdef QT_DEBUG  // logs will redirect to local file
  qInfo("Log message will be display in prompt command window");
#else
  qInfo("Log message will be redirect to file[%s]", qPrintable(mLogFile.fileName()));
#endif

  qInstallMessageHandler(LogHandler::myMessageOutput);
}

LogHandler::~LogHandler() {
  if (mLogFile.isOpen()) {
    mLogFile.close();
  }
}

void LogHandler::myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
  static const char DBG_TYPE_2_CHAR[QtInfoMsg + 1] = {'D', 'W', 'C', 'F', 'I'};
  static QString logMsg;
  // `hh:mm:ss.zzz E functionName msg [fileName:fileNo]`
  logMsg.reserve(300);
  logMsg.clear();
  logMsg += QTime::currentTime().toString("hh:mm:ss.zzz");
  logMsg += ' ';
  logMsg += DBG_TYPE_2_CHAR[type];
  logMsg += ' ';
  logMsg += context.function;
  logMsg += ' ';
  logMsg += msg;
  logMsg += '[';
  logMsg += context.file;
  logMsg += ":";
  logMsg += QString::number(context.line);
  logMsg += ']';

#ifdef QT_DEBUG  // logs will redirect to local file
  printf("%s\n", qPrintable(logMsg));
  if (mFlushLogInBufferInstantly || type >= QtWarningMsg) {
    std::fflush(stdout);
  }
  return;
#endif

  // in release mode, compile choice add "-g" to display function name and line
  if (mLogTextStream.device() == nullptr || !mLogFile.isOpen()) {
    printf("critical error, cannot write into log file");
    return;
  }
  mLogTextStream << logMsg << '\n';
  if (mFlushLogInBufferInstantly || type >= QtWarningMsg) {
    mLogTextStream.flush();
  }
}

bool LogHandler::OpenLogFile() {
  LogHandler::ManualFlush();
  const bool openLogFileResult{QDesktopServices::openUrl(QUrl::fromLocalFile(mLogFile.fileName()))};
  qDebug("open log file[%s] bResult:%d", qPrintable(mLogFile.fileName()), openLogFileResult);
  return openLogFileResult;
}

bool LogHandler::OpenLogFolder() {
  const bool openLogFolderResult{QDesktopServices::openUrl(QUrl::fromLocalFile(mLogFolderPath))};
  qDebug("open log folder[%s] bResult:%d", qPrintable(mLogFolderPath), openLogFolderResult);
  return openLogFolderResult;
}

void LogHandler::SetLogLevelError() {
  OUTPUT_LOG_LEVEL = QtWarningMsg;
  PreferenceSettings().setValue(MemoryKey::LOG_DEVEL_DEBUG.name, false);
}
void LogHandler::SetLogLevelDebug() {
  OUTPUT_LOG_LEVEL = QtDebugMsg;
  PreferenceSettings().setValue(MemoryKey::LOG_DEVEL_DEBUG.name, true);
}

void LogHandler::SetFlushInstantly(bool flushInstant) {
  mFlushLogInBufferInstantly = flushInstant;
}

bool LogHandler::AgingLogFiles(const int AGING_FILE_ABOVE_B, QString* pAgedLogPath) {
  const QString logPath{mLogFile.fileName()};
  if (!mLogFile.exists()) {
    qWarning("Log file[%s] not exists", qPrintable(logPath));
    return false;
  }
  if (mLogFile.size() < AGING_FILE_ABOVE_B) {                                                  // 2 MiB
    qWarning("Log file[%s] size[%lld] is bellow threshold[%d Byte(s)], no need to aging now",  //
             qPrintable(logPath), mLogFile.size(), AGING_FILE_ABOVE_B);
    return true;
  }
  ManualFlush();  // flush it right now
  if (mLogFile.isOpen()) {
    mLogFile.close();
    printf("Close log file succeed\n");
  }

  const QString agedLogPath{logPath + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh_mm_ss") + ".log"};
  if (pAgedLogPath != nullptr) {
    *pAgedLogPath = agedLogPath;
  }
  if (!QFile::rename(logPath, agedLogPath)) {
    printf("log file should aged [%s]->[%s] failed\n", qPrintable(logPath), qPrintable(agedLogPath));
    return false;
  }
  mLogFile.setFileName(logPath);
  printf("Aging log file [%s] ok\n", qPrintable(agedLogPath));

  if (!mLogFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
    printf("Log message CANNOT redirect to file[%s]\n", qPrintable(logPath));
    return false;
  }
  qWarning("Log message will redirect to file[%s]", qPrintable(logPath));
  return true;
}

bool LogHandler::ManualFlush() {
#ifdef QT_DEBUG
  std::fflush(stdout);
#endif
  if (mLogTextStream.device() == nullptr || !mLogFile.isOpen()) {
    return false;
  }
  mLogTextStream.flush();
  return true;
}

bool LogHandler::subscribe() {
  auto& ins = g_LogActions();
  connect(ins._LOG_FILE, &QAction::triggered, &LogHandler::OpenLogFile);
  connect(ins._LOG_FOLDER, &QAction::triggered, &LogHandler::OpenLogFolder);
  connect(ins._LOG_AGING, &QAction::triggered, []() { LogHandler::AgingLogFiles(); });

  connect(ins._LOG_LEVEL_DEBUG, &QAction::triggered, &LogHandler::SetLogLevelDebug);
  connect(ins._LOG_LEVEL_WARNING, &QAction::triggered, &LogHandler::SetLogLevelError);

  connect(ins._FLUSH_INSTANTLY, &QAction::triggered, &LogHandler::SetFlushInstantly);
  return true;
}
