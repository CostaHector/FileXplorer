#include "LogHandler.h"
#include "PublicVariable.h"
#include "Actions/LogActions.h"
#include <QDesktopServices>

const QString LogHandler::LOG_ABSFILENAME_TEMPLATE{"%1/logs_info%2.log"};
QtMsgType LogHandler::OUTPUT_LOG_LEVEL = QtWarningMsg;
QFile LogHandler::outFile;
QTextStream LogHandler::ts(&outFile);
QString LogHandler::logPrePath;
bool LogHandler::m_flushInstantly{false};
LogHandler::LogHandler(QObject* parent) : QObject{parent} {
  //  const QByteArray envVar = qgetenv("QTDIR");
  //  IS_LOG_TO_FILE = envVar.isEmpty();
  //  if (!IS_LOG_TO_FILE) {
  //    qInfo("Log message will only display on command prompt");
  //    return;
  //  }
  OUTPUT_LOG_LEVEL = PreferenceSettings().value(MemoryKey::LOG_DEVEL_DEBUG.name, MemoryKey::LOG_DEVEL_DEBUG.v).toBool() ? QtDebugMsg : QtWarningMsg;
#ifdef _WIN32
  logPrePath = PreferenceSettings().value(MemoryKey::WIN32_RUNLOG.name).toString();
#else
  logPrePath = PreferenceSettings().value(MemoryKey::LINUX_RUNLOG.name).toString();
#endif
  outFile.setFileName(LOG_ABSFILENAME_TEMPLATE.arg(logPrePath, ""));
  if (!outFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
    qCritical("Log message CANNOT redirect to file[%s]", qPrintable(outFile.fileName()));
    return;
  }
  qInfo("Log message will be redirect to file[%s]", qPrintable(outFile.fileName()));
  qInstallMessageHandler(LogHandler::myMessageOutput);
}

LogHandler::~LogHandler() {
  if (outFile.isOpen()) {
    outFile.close();
  }
}

void LogHandler::myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
#ifdef QT_NO_DEBUG
  if (type < OUTPUT_LOG_LEVEL) {
    return;
  }
#endif

  static const QChar DBG_TYPE_2_CHAR[QtInfoMsg + 1] = {'D', 'W', 'C', 'F', 'I'};
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

#ifdef QT_DEBUG  // debug mode
  printf("%s\n", qPrintable(logMsg));
  fflush(stdout);
  return;
#else  // release mode, compile choice add "-g" to display function name and line
  if (ts.device() == nullptr || !outFile.isOpen()) {
    printf("critical error, cannot write into log file");
    return;
  }
  ts << logMsg << '\n';
  if (m_flushInstantly || type >= QtWarningMsg) {
    ts.flush();
  }
#endif
}

bool LogHandler::OpenLogFile() {
  LogHandler::ManualFlush();
  const bool openLogFileResult{QDesktopServices::openUrl(QUrl::fromLocalFile(outFile.fileName()))};
  qDebug("open log file[%s] bResult:%d", qPrintable(outFile.fileName()), openLogFileResult);
  return openLogFileResult;
}

bool LogHandler::OpenLogFolder() {
  const bool openLogFolderResult{QDesktopServices::openUrl(QUrl::fromLocalFile(logPrePath))};
  qDebug("open log folder[%s] bResult:%d", qPrintable(logPrePath), openLogFolderResult);
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
  m_flushInstantly = flushInstant;
}

bool LogHandler::AgingLogFiles() {
  const QString oldLogFileName{outFile.fileName()};
  if (!outFile.exists()) {
    qWarning("Log file[%s] not exists", qPrintable(oldLogFileName));
    return false;
  }
  if (outFile.size() < 10 * 1024 * 1024) {  // 10 MiB
    qWarning("Log file[%s] size[%lld] is bellow 10 MiB, no need to aging now", qPrintable(oldLogFileName), outFile.size());
    return true;
  }
  ManualFlush(); // flush it right now
  if (outFile.isOpen()) {
    outFile.close();
    qWarning("Close log file succeed");
  }

  const QString newLogFileName{LOG_ABSFILENAME_TEMPLATE.arg(logPrePath, QTime::currentTime().toString("_hh_mm_ss"))};
  if (!QFile::rename(oldLogFileName, newLogFileName)) {
    qWarning("Aging log file name failed:\n[%s]\n[%s]", qPrintable(oldLogFileName), qPrintable(newLogFileName));
    return false;
  }
  qWarning("Aging log file ok");
  outFile.setFileName(newLogFileName);
  if (!outFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
    qCritical("Log message CANNOT redirect to file[%s]", qPrintable(newLogFileName));
    return false;
  }
  qWarning("Log message will redirect to file[%s]", qPrintable(newLogFileName));
  return true;
}

bool LogHandler::subscribe() {
  auto& ins = g_LogActions();
  connect(ins._LOG_FILE, &QAction::triggered, &LogHandler::OpenLogFile);
  connect(ins._LOG_FOLDER, &QAction::triggered, &LogHandler::OpenLogFolder);
  connect(ins._LOG_AGING, &QAction::triggered, &LogHandler::AgingLogFiles);

  connect(ins._LOG_LEVEL_DEBUG, &QAction::triggered, &LogHandler::SetLogLevelDebug);
  connect(ins._LOG_LEVEL_ERROR, &QAction::triggered, &LogHandler::SetLogLevelError);

  connect(ins._FLUSH_INSTANTLY, &QAction::triggered, &LogHandler::SetFlushInstantly);
  return true;
}
