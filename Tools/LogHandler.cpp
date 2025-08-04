#include "LogHandler.h"
#include "public/MemoryKey.h"
#include "Actions/LogActions.h"
#include "public/PathTool.h"
#include "public/PublicVariable.h"

#include <QDesktopServices>
#include <QMessageLogContext>
#include <QDateTime>
#include <QFileInfo>
#include <QUrl>

void SetQtDebugMessagePattern() {
  qInfo("Log message will be display in prompt command window directly");
  qSetMessagePattern(
      "%{time hh:mm:ss.zzz} "
      "%{if-debug}D%{endif}"
      "%{if-info}I%{endif}"
      "%{if-warning}W%{endif}"
      "%{if-critical}C%{endif}"
      "%{if-fatal}F%{endif} "
      " %{function} "
      " %{message} "
      " [%{file}:%{line}]");
}

QtMsgType LogHandler::OUTPUT_LOG_LEVEL = QtMsgType::QtWarningMsg;

QString LogHandler::mLogFolderPath;
bool LogHandler::mAutoFlushLogBuffer{false};

QFile LogHandler::mLogFile;
QTextStream LogHandler::mLogTextStream(&mLogFile);

LogHandler::LogHandler(const QString& logPath, const int msgType, QObject* parent)  //
    : QObject{parent}                                                               //
{
  qSetMessagePattern(
      "%{time hh:mm:ss.zzz} "
      "%{if-debug}D%{endif}"
      "%{if-info}I%{endif}"
      "%{if-warning}W%{endif}"
      "%{if-critical}C%{endif}"
      "%{if-fatal}F%{endif}"
      " %{message}");
  if (msgType >= QtDebugMsg && msgType <= QtSystemMsg) {  // input valid log level type
    OUTPUT_LOG_LEVEL = static_cast<QtMsgType>(msgType);
  } else {
    const int debugLvl = PreferenceSettings().value(MemoryKey::LOG_LEVEL_PRINT_INSTANTLY.name, MemoryKey::LOG_LEVEL_PRINT_INSTANTLY.v).toInt();
    if (debugLvl >= QtDebugMsg && debugLvl <= QtSystemMsg) {  // memory valid log level type
      OUTPUT_LOG_LEVEL = static_cast<QtMsgType>(debugLvl);
    } else {
      OUTPUT_LOG_LEVEL = QtMsgType::QtWarningMsg;  // use befault
    }
  }

  if (!logPath.isEmpty()) {  // log path valid
    mLogFolderPath = logPath;
  } else {    
    mLogFolderPath = SystemPath::WORK_PATH + "/logs_info.log";
  }
  mLogFile.setFileName(mLogFolderPath);
  if (!mLogFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
    qCritical("Log message CANNOT redirect to file[%s]", qPrintable(mLogFile.fileName()));
    return;
  }
  qInfo("Log message will be redirect to file[%s]", qPrintable(mLogFile.fileName()));
  qInstallMessageHandler(LogHandler::myMessageOutput);
}

LogHandler::~LogHandler() {
  if (mLogFile.isOpen()) {
    mLogFile.close();
  }
}

void LogHandler::myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
  static QByteArray logMsg;
  logMsg.reserve(300);
  logMsg.clear();

  // Way 1: full log message "hh:mm:ss.zzz E functionName msg [fileName:fileLineNumber]"
  //  logMsg.append(QTime::currentTime().toString("hh:mm:ss.zzz").toUtf8())
  //      .append(' ')               //
  //      .append("DWCFI"[type])     //
  //      .append(' ')               //
  //      .append(context.function)  //
  //      .append(' ')               //
  //      .append(msg.toUtf8())      //
  //      .append('[')               //
  //      .append(context.file)      //
  //      .append(':');
  //  static constexpr int DIGIT_NUM = 5;  // linenumber: 0~99999
  //  static char lineStr[DIGIT_NUM + 1]{0};
  //  memset(lineStr, 0, DIGIT_NUM + 1);
  //  char* p = lineStr + DIGIT_NUM;
  //  int num = context.line;
  //  do {
  //    *(--p) = '0' + (num % 10);
  //    num /= 10;
  //  } while (num > 0);
  //  logMsg
  //      .append(p)    //
  //      .append(']')  //
  //      .append('\n');

  // Way2: simplified log message "hh:mm:ss.zzz E msg"
  logMsg.append(QTime::currentTime().toString("hh:mm:ss.zzz").toUtf8())
      .append(' ')            //
      .append("DWCFI"[type])  //
      .append(' ')            //
      .append(msg.toUtf8())   //
      .append('\n');

  mLogTextStream.operator<<(logMsg);
  // Way3: use specified pattern
  // mLogTextStream.operator<<(qFormatLogMessage(type, context, msg));
  if (mAutoFlushLogBuffer || type >= OUTPUT_LOG_LEVEL) {
    mLogTextStream.flush();
  }
}

bool LogHandler::OpenLogFile() {
  LogHandler::ManualFlush();
  const bool openLogFileResult{QDesktopServices::openUrl(QUrl::fromLocalFile(mLogFile.fileName()))};
  if (!openLogFileResult) {
    qDebug("Open log file[%s] failed", qPrintable(mLogFile.fileName()));
  }
  return openLogFileResult;
}

bool LogHandler::OpenLogFolder() {
  if (!QDesktopServices::openUrl(QUrl::fromLocalFile(mLogFolderPath))) {
    qDebug("Open log folder[%s] failed", qPrintable(mLogFolderPath));
    return false;
  }
  return true;
}

void LogHandler::SetLogLevelError() {
  OUTPUT_LOG_LEVEL = QtWarningMsg;
  PreferenceSettings().setValue(MemoryKey::LOG_LEVEL_PRINT_INSTANTLY.name, false);
}
void LogHandler::SetLogLevelDebug() {
  OUTPUT_LOG_LEVEL = QtDebugMsg;
  PreferenceSettings().setValue(MemoryKey::LOG_LEVEL_PRINT_INSTANTLY.name, true);
}

void LogHandler::SetFlushInstantly(bool flushInstant) {
  mAutoFlushLogBuffer = flushInstant;
}

bool LogHandler::AgingLogFiles(const int AGING_FILE_ABOVE_B, QString* pAgedLogFileName) {
  const QString logPath{mLogFile.fileName()};
  if (!mLogFile.exists()) {
    printf("Log file[%s] not exists", qPrintable(logPath));
    return false;
  }
  if (mLogFile.size() < AGING_FILE_ABOVE_B) {                                                // 2 MiB
    qDebug("Log file[%s] size[%lld] is bellow threshold[%d Byte(s)], no need to aging now",  //
           qPrintable(logPath), mLogFile.size(), AGING_FILE_ABOVE_B);
    return true;
  }
  ManualFlush();  // flush it right now
  if (mLogFile.isOpen()) {
    qDebug("Close log file succeed\n");
    mLogFile.close();
  }

  QString logFolderPath;
  const QString oldLogFileName = PathTool::GetPrepathAndFileName(logPath, logFolderPath);
  const QString newLogFileName{QDateTime::currentDateTime().toString("yyyy-MM-dd_hh_mm_ss_") + oldLogFileName};
  const QString newLogAbsPath = logFolderPath + '/' + newLogFileName;

  if (pAgedLogFileName != nullptr) {
    *pAgedLogFileName = newLogFileName;
  }
  if (!QFile::rename(logPath, newLogAbsPath)) {
    printf("log file aged [%s]->[%s] failed\n", qPrintable(logPath), qPrintable(newLogFileName));
    return false;
  }
  mLogFile.setFileName(logPath);
  printf("Aging log file [%s->%s] ok\n", qPrintable(oldLogFileName), qPrintable(newLogFileName));
  if (!mLogFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
    fprintf(stderr, "Log message CANNOT redirect to file[%s]\n", qPrintable(logPath));
    return false;
  }
  return true;
}

bool LogHandler::ManualFlush() {
  if (!IsLogModuleOk()) {
    return false;
  }
  mLogTextStream.flush();
  mLogFile.flush();
  return true;
}

bool LogHandler::IsLogModuleOk() {
  // in release mode, compile choice add "-g" to display function name and line
  if (mLogTextStream.device() == nullptr || !mLogFile.isOpen()) {
    fprintf(stderr, "Critical error, cannot write into log file[%s]\n", qPrintable(mLogFile.fileName()));
    return false;
  }
  return true;
}

void LogHandler::subscribe() {
  auto& ins = g_LogActions();
  connect(ins._LOG_FILE, &QAction::triggered, &LogHandler::OpenLogFile);
  connect(ins._LOG_FOLDER, &QAction::triggered, &LogHandler::OpenLogFolder);
  connect(ins._LOG_AGING, &QAction::triggered, []() { LogHandler::AgingLogFiles(); });
  connect(ins._LOG_LEVEL_DEBUG, &QAction::triggered, &LogHandler::SetLogLevelDebug);
  connect(ins._LOG_LEVEL_WARNING, &QAction::triggered, &LogHandler::SetLogLevelError);
  connect(ins._FLUSH_INSTANTLY, &QAction::triggered, &LogHandler::SetFlushInstantly);
}
