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
  static inline void ManualFlush() {
#ifdef QT_DEBUG
    fflush(stdout);
#else
    if (ts.device() != nullptr) {
      ts.flush();
    }
#endif
  }

 private:
  static bool IS_LOG_TO_FILE_AVAIL;
  static QFile outFile;
  static QTextStream ts;
};

#endif  // MESSAGEOUTPUT_H
