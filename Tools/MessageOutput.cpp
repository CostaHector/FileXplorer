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
  qInstallMessageHandler(MessageOutput::myMessageOutput);
}

void MessageOutput::myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
  if (not IS_LOG_TO_FILE) {
    printf("%s\n", qPrintable(msg));
    fflush(stdout);
    return;
  }
  if (not IS_LOG_TO_FILE_AVAIL) {
    printf("[ERROR] Logs will not be save to files");
    fflush(stdout);
    return;
  }
  const QString& curTime = QTime::currentTime().toString("hh:mm:ss.zzz");
  switch (type) {
    case QtDebugMsg:
      ts << logTemplate.arg(curTime, "D", qPrintable(msg), context.file).arg(context.line).arg(context.function);
      break;
    case QtInfoMsg:
      ts << logTemplate.arg(curTime, "I", qPrintable(msg), context.file).arg(context.line).arg(context.function);
      break;
    case QtWarningMsg:
      ts << logTemplate.arg(curTime, "W", qPrintable(msg), context.file).arg(context.line).arg(context.function);
      break;
    case QtCriticalMsg:
      ts << logTemplate.arg(curTime, "C", qPrintable(msg), context.file).arg(context.line).arg(context.function);
      break;
    case QtFatalMsg:
      ts << logTemplate.arg(curTime, "F", qPrintable(msg), context.file).arg(context.line).arg(context.function);
      abort();
    default:
      ts << logTemplate.arg(curTime, "U", qPrintable(msg), context.file).arg(context.line).arg(context.function);
  }
}
