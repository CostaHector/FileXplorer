#include "MessageOutput.h"
#include "PublicVariable.h"

QFile MessageOutput::outFile;
QTextStream MessageOutput::ts(&outFile);
const QString MessageOutput::logTemplate("%1\t%2\t%3\t%4\t%5\t%6\n");

MessageOutput::MessageOutput() {
  qInstallMessageHandler(MessageOutput::myMessageOutput);
#ifdef _WIN32
  QString logPrePath = PreferenceSettings().value(MemoryKey::WIN32_RUNLOG.name).toString();
#else
  QString logPrePath = PreferenceSettings().value(MemoryKey::LINUX_RUNLOG.name).toString();
#endif
  outFile.setFileName(QString("%1/logs_info.log").arg(logPrePath));
}

void MessageOutput::myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
  printf("%s\n", qPrintable(msg));
  if (not(outFile.isOpen() or outFile.open(QIODevice::WriteOnly | QIODevice::Append))) {
    qWarning("logs will not be save to files");
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
