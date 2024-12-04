#ifndef LOGHANDLER_H
#define LOGHANDLER_H
#include <QFile>
#include <QTextStream>

class LogHandler : QObject {
 public:
  explicit LogHandler(QObject* parent = nullptr);
  ~LogHandler();
  static void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg);
  static inline void ManualFlush() {
#ifdef QT_DEBUG
    fflush(stdout);
#else
    if (mLogTextStream.device() == nullptr || !mLogFile.isOpen()) {
      return;
    }
    mLogTextStream.flush();
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
  static bool mFlushLogInBufferInstantly;
  static QString mLogFolderPath;

  static QFile mLogFile;
  static QTextStream mLogTextStream;
};

#endif  // LOGHANDLER_H
