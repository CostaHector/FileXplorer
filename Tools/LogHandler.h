#ifndef LOGHANDLER_H
#define LOGHANDLER_H
#include <QFile>
#include <QTextStream>

class LogHandler : QObject {
 public:
  explicit LogHandler(QObject* parent = nullptr, const QString& logPath = "");
  ~LogHandler();
  static bool subscribe();
  static bool AgingLogFiles(const int AGING_FILE_ABOVE_B = 2 * 1024 * 1024, QString* pAgedLogPath = nullptr);

 private:
  static void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg);
  static bool OpenLogFile();
  static bool OpenLogFolder();
  static void SetLogLevelError();
  static void SetLogLevelDebug();
  static void SetFlushInstantly(bool flushInstant);
  static bool ManualFlush();

  static QtMsgType OUTPUT_LOG_LEVEL;
  static bool mFlushLogInBufferInstantly;
  static QString mLogFolderPath;

  static QFile mLogFile;
  static QTextStream mLogTextStream;
};

#endif  // LOGHANDLER_H
