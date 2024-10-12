#include "MessageOutput.h"
#include "PublicVariable.h"

bool MessageOutput::IS_LOG_TO_FILE_AVAIL{false};
QFile MessageOutput::outFile;
QTextStream MessageOutput::ts(&outFile);

MessageOutput::MessageOutput() {
//  const QByteArray envVar = qgetenv("QTDIR");
//  IS_LOG_TO_FILE = envVar.isEmpty();
//  if (!IS_LOG_TO_FILE) {
//    qInfo("Log message will only display on command prompt");
//    return;
//  }
#ifdef _WIN32
  const QString& logPrePath = PreferenceSettings().value(MemoryKey::WIN32_RUNLOG.name).toString();
#else
  const QString& logPrePath = PreferenceSettings().value(MemoryKey::LINUX_RUNLOG.name).toString();
#endif
  outFile.setFileName(QString("%1/logs_info.log").arg(logPrePath));
  IS_LOG_TO_FILE_AVAIL = (outFile.isOpen() or outFile.open(QIODevice::WriteOnly | QIODevice::Append));
  if (!IS_LOG_TO_FILE_AVAIL) {
    qCritical("Log message CANNOT redirect to file[%s]", qPrintable(outFile.fileName()));
    return;
  }
  qInfo("Log message will be redirect to file[%s]", qPrintable(outFile.fileName()));
  qInstallMessageHandler(MessageOutput::myMessageOutput);
}

void MessageOutput::myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
  static const QChar DBG_TYPE_2_CHAR[QtInfoMsg + 1] = {'D', 'W', 'C', 'F', 'I'};
  static QString logMsg;
  logMsg.reserve(300);
  logMsg.clear();
  logMsg += QTime::currentTime().toString("hh:mm:ss.zzz");
  logMsg += ' ';
  logMsg += DBG_TYPE_2_CHAR[type];
  logMsg += ' ';
  logMsg += msg;
  logMsg += '[';
  logMsg += context.file;
  logMsg += ":";
  logMsg += QString::number(context.line);
  logMsg += ']';
  logMsg += context.function;
  logMsg += '\n';

#ifdef QT_DEBUG  // debug mode
  printf("%s", qPrintable(logMsg));
  fflush(stdout);
  return;
#else  // release mode
  if (IS_LOG_TO_FILE_AVAIL) {
    ts << logMsg;
    if (type >= QtWarningMsg) {
      ts.flush();
    }
    return;
  }
  printf("critical error, cannot write into log file");
#endif
}
