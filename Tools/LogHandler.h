#ifndef LOGHANDLER_H
#define LOGHANDLER_H
#include <QFile>
#include <QMessageLogContext>

#include <QTextStream>
#include <QTime>
class LogHandler : QObject {
 public:
  explicit LogHandler(QObject* parent = nullptr);
  ~LogHandler();
  static void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg);
  static inline void ManualFlush() {
#ifdef QT_DEBUG
    fflush(stdout);
#else
    if (ts.device() == nullptr || !outFile.isOpen()) {
      return;
    }
    ts.flush();
#endif
  }
  static bool subscribe();

 private:
  static bool OpenLogFile();
  static bool OpenLogFolder();
  static void SetLogLevelError();
  static void SetLogLevelDebug();
  static void SetFlushInstantly(bool flushInstant);
  static bool AgingLogFiles();

  static const QString LOG_ABSFILENAME_TEMPLATE;
  static QtMsgType OUTPUT_LOG_LEVEL;
  static bool m_flushInstantly;
  static QString logPrePath;
  static QFile outFile;
  static QTextStream ts;
};

#endif  // LOGHANDLER_H
