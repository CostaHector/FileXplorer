#include "MessageOutput.h"
#include "PublicVariable.h"

bool MessageOutput::IS_LOG_TO_FILE{true};
bool MessageOutput::IS_LOG_TO_FILE_AVAIL{false};
QFile MessageOutput::outFile;
QTextStream MessageOutput::ts(&outFile);

MessageOutput::MessageOutput() {
#ifdef _WIN32
  const QString& logPrePath = PreferenceSettings().value(MemoryKey::WIN32_RUNLOG.name).toString();
#else
  const QString& logPrePath = PreferenceSettings().value(MemoryKey::LINUX_RUNLOG.name).toString();
#endif
  outFile.setFileName(QString("%1/logs_info.log").arg(logPrePath));
  const QByteArray envVar = qgetenv("QTDIR");
  IS_LOG_TO_FILE = envVar.isEmpty();

  IS_LOG_TO_FILE_AVAIL = outFile.isOpen() or outFile.open(QIODevice::WriteOnly | QIODevice::Append);
  qCritical("log message cannot redirect to file[%s]", qPrintable(outFile.fileName()));
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

  if (not IS_LOG_TO_FILE) {
    printf("%s\n", qPrintable(logMsg));
    if (type >= QtWarningMsg) {
      fflush(stdout);
    }
    return;
  }
  if (not IS_LOG_TO_FILE_AVAIL) {
    printf("%s\n", qPrintable(logMsg));
    fflush(stdout);
    return;
  }
  ts << logMsg;
}
