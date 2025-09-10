#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <memory>

enum class LOG_LVL_E : int {
  BEGIN = 0, // begin
  D = 0,     // debug
  I,         // information
  W,         // warning
  E,         // error
  C,         // critical
  F,         // fatal
  Q,         // question
  O,         // OK(TASK_FINISHED)
  P,         // PARTIAL_SUCCESS
  BUTT,      //
};
static const char LVLS_CHAR_ARRAY[] = "DIWECFQOP";

inline bool operator<(LOG_LVL_E a, LOG_LVL_E b) {
  return static_cast<int>(a) < static_cast<int>(b);
}

struct Logger {
public:
  static inline FILE*& out() {
    static FILE* pLogStream = GetFILEStream(); // cross unit compile protection!
    return pLogStream; // alway valid pointer here
  }
  static const QString& GetLogFileAbsPath();
  static LOG_LVL_E m_printLevel; // log which level below this will mute
  static LOG_LVL_E m_autoFflushLevel; // log which level not below this will flush into file instantly

  static bool AgingLogFiles(const QString& logFileAbsPath, const int AGING_FILE_ABOVE_B = 104857600 /*100MB*/, QString* pAgedLogFileName = nullptr);
  static QByteArray GetLastNLinesOfLogs(const int maxLines = 100);
  static bool OpenLogFile();
  static bool OpenLogFolder();
  static void SetPrintLevel(LOG_LVL_E newLevel);
  static void SetAutoFlushAllLevel(bool allLevelChecked);
private:
  static FILE* SwitchLogToALocalFile(const QString& logFileAbsPath = "");
  static FILE* GetFILEStream();
  static bool CloseLogFile(FILE* pFile);
  static std::unique_ptr<FILE, decltype(Logger::CloseLogFile)*> mLogFILEStreamUniquePtr;
  static constexpr char CONSTANT_LOG_FILE_NAME[] = "logs_info.log";
};

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <chrono>
#include <ctime>

#define LOG_TIME_PATTERN "%04d/%02d/%02d %02d:%02d:%02d.%03d"
#define LOG_TIME_PATTERN_LEN (23+1) // 4+2+2 +2+2+2+3 + 2+1+2+1 + 1=24

inline void get_timestamp(char* buf, size_t len) {
  using namespace std::chrono;
  auto now = system_clock::now();
  time_t t = system_clock::to_time_t(now);
  auto ms = duration_cast<milliseconds>(now.time_since_epoch()).count() % 1000;

  struct tm tm;
#ifdef _WIN32
  localtime_s(&tm, &t);
#else
  localtime_r(&t, &tm);  // Linux/POSIX 安全版本
#endif
  snprintf(buf, len, LOG_TIME_PATTERN,
           tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, (int)ms);
}

#ifndef PARENT_PATH_LEN
#define PARENT_PATH_LEN 0
#endif

#define LOG_OUT(level, patternCstr, ...) do { \
if (level < Logger::m_printLevel) break;\
    char timestamp[LOG_TIME_PATTERN_LEN]{0}; \
    get_timestamp(timestamp, LOG_TIME_PATTERN_LEN); \
    fprintf(Logger::out(), patternCstr, \
            timestamp, LVLS_CHAR_ARRAY[(int)level], __func__, \
            ##__VA_ARGS__,\
                              static_cast<const char*>(__FILE__) + PARENT_PATH_LEN, __LINE__); \
    if (!(level < Logger::m_autoFflushLevel)) fflush(Logger::out()); \
} while(0)

// 具体级别日志宏
#define LOG_D(fmt, ...) LOG_OUT(LOG_LVL_E::D, "%s %c [%s] " fmt " [%s:%d]\n", ##__VA_ARGS__)  // DEBUG
#define LOG_I(fmt, ...) LOG_OUT(LOG_LVL_E::I, "%s %c [%s] " fmt " [%s:%d]\n", ##__VA_ARGS__)  // INFO
#define LOG_W(fmt, ...) LOG_OUT(LOG_LVL_E::W, "%s %c [%s] " fmt " [%s:%d]\n", ##__VA_ARGS__)  // WARNING
#define LOG_E(fmt, ...) LOG_OUT(LOG_LVL_E::E, "%s %c [%s] " fmt " [%s:%d]\n", ##__VA_ARGS__)  // ERROR
#define LOG_C(fmt, ...) LOG_OUT(LOG_LVL_E::C, "%s %c [%s] " fmt " [%s:%d]\n", ##__VA_ARGS__)  // CRITICAL
#define LOG_F(fmt, ...) LOG_OUT(LOG_LVL_E::F, "%s %c [%s] " fmt " [%s:%d]\n", ##__VA_ARGS__)  // FATAL
#define LOG_Q(fmt, ...) LOG_OUT(LOG_LVL_E::Q, "%s %c [%s] " fmt " [%s:%d]\n", ##__VA_ARGS__)  // QUESTION
#define LOG_O(fmt, ...) LOG_OUT(LOG_LVL_E::O, "%s %c [%s] " fmt " [%s:%d]\n", ##__VA_ARGS__)  // OK(TASK_FINISHED)
#define LOG_P(fmt, ...) LOG_OUT(LOG_LVL_E::P, "%s %c [%s] " fmt " [%s:%d]\n", ##__VA_ARGS__)  // PARTIAL_SUCCESS

#endif // LOGGER_H
