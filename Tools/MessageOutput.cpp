#include "MessageOutput.h"
#include "PublicVariable.h"

bool MessageOutput::IS_LOG_TO_FILE{true};
bool MessageOutput::IS_LOG_TO_FILE_AVAIL{false};
QFile MessageOutput::outFile;
QTextStream MessageOutput::ts(&outFile);
const QString MessageOutput::logTemplate("%1\t%2\t%3\t%4\t%5\t%6\n");

MessageOutput::MessageOutput() {
#ifdef _WIN32
  QString logPrePath = PreferenceSettings().value(MemoryKey::WIN32_RUNLOG.name).toString();
#else
  QString logPrePath = PreferenceSettings().value(MemoryKey::LINUX_RUNLOG.name).toString();
#endif
  outFile.setFileName(QString("%1/logs_info.log").arg(logPrePath));
  const QByteArray envVar = qgetenv("QTDIR");
  IS_LOG_TO_FILE = envVar.isEmpty();

  IS_LOG_TO_FILE_AVAIL = outFile.isOpen() or outFile.open(QIODevice::WriteOnly | QIODevice::Append);
  qCritical("log message cannot redirect to file[%s]", qPrintable(outFile.fileName()));
  qInstallMessageHandler(MessageOutput::myMessageOutput);
}

void MessageOutput::myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
  static const QChar DBG_TYPE_2_CHAR[QtInfoMsg+1] = {'D', 'I', 'W', 'C', 'F'};
  const QString& curTime = QTime::currentTime().toString("hh:mm:ss.zzz");
  const QString& logMsg = logTemplate.arg(curTime, DBG_TYPE_2_CHAR[type], qPrintable(msg), context.file).arg(context.line).arg(context.function);

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
