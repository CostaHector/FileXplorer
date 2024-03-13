#include "MessageOutput.h"

QFile MessageOutput::outFile("logs_info.log");
QTextStream MessageOutput::ts(&outFile);
const QString MessageOutput::logTemplate("%1\t%2\t%3\t%4\t%5\t%6\n");

MessageOutput::MessageOutput() {
  qInstallMessageHandler(MessageOutput::myMessageOutput);
}

void MessageOutput::myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
  if (not(outFile.isOpen() or outFile.open(QIODevice::WriteOnly | QIODevice::Append))) {
    qWarning("logs will not be save to files");
    return;
  }
  const QString& curTime = QTime::currentTime().toString("hh:mm:ss.zzz");
  switch (type) {
    case QtDebugMsg:
      ts << logTemplate.arg(curTime, "Debug", qPrintable(msg), context.file).arg(context.line).arg(context.function);
      break;
    case QtInfoMsg:
      ts << logTemplate.arg(curTime, "Info", qPrintable(msg), context.file).arg(context.line).arg(context.function);
      break;
    case QtWarningMsg:
      ts << logTemplate.arg(curTime, "Warning", qPrintable(msg), context.file).arg(context.line).arg(context.function);
      break;
    case QtCriticalMsg:
      ts << logTemplate.arg(curTime, "Critical", qPrintable(msg), context.file).arg(context.line).arg(context.function);
      break;
    case QtFatalMsg:
      ts << logTemplate.arg(curTime, "Fatal", qPrintable(msg), context.file).arg(context.line).arg(context.function);
      abort();
    default:
      ts << logTemplate.arg(curTime, "Undefined", qPrintable(msg), context.file).arg(context.line).arg(context.function);
  }
}

