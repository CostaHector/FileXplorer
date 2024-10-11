#ifndef MESSAGEOUTPUT_H
#define MESSAGEOUTPUT_H
#include <QFile>
#include <QMessageLogContext>

#include <QTextStream>
#include <QTime>
class MessageOutput {
 public:
  MessageOutput();
  static void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg);
  static void flush() {
    if (ts.device() != nullptr)
      ts.flush();
  }

 private:
  static bool IS_LOG_TO_FILE;
  static bool IS_LOG_TO_FILE_AVAIL;
  static QFile outFile;
  static QTextStream ts;
};

#endif  // MESSAGEOUTPUT_H
