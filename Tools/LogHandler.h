#ifndef LOGHANDLER_H
#define LOGHANDLER_H
#include <QFile>
#include <QTextStream>

void SetQtDebugMessagePattern();

class LogHandler: QObject {
 public:
  explicit LogHandler(const QString& logPath = "", const int msgType = -1, QObject* parent = nullptr);
  ~LogHandler();
  static bool IsLogModuleOk();
  static void subscribe();
  static bool AgingLogFiles(const int AGING_FILE_ABOVE_B = 2 * 1024 * 1024, QString* pAgedLogFileName = nullptr);

 private:
  static void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg);
  static bool OpenLogFile();
  static bool OpenLogFolder();
  static void SetLogLevelError();
  static void SetLogLevelDebug();
  static void SetFlushInstantly(bool flushInstant);
  static bool ManualFlush();

  static QtMsgType OUTPUT_LOG_LEVEL;
  static bool mAutoFlushLogBuffer;
  static QString mLogFilePath;

  static QFile mLogFile;
  static QTextStream mLogTextStream;
};

#endif  // LOGHANDLER_H
