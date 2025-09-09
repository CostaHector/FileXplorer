#ifndef NOTIFICATORMACRO_H
#define NOTIFICATORMACRO_H

#include "Notificator.h"

// with params
#define LOG_INFO_P(title, msgFormat, ...) \
do { \
      QString _msg = QString::asprintf(msgFormat, ##__VA_ARGS__); \
      LOG_I("%s: %s", qPrintable(title), qPrintable(_msg)); \
      Notificator::information(title, _msg); \
} while(0)
#define LOG_WARN_P(title, msgFormat, ...) \
do { \
      QString _msg = QString::asprintf(msgFormat, ##__VA_ARGS__); \
      LOG_W("%s: %s", qPrintable(title), qPrintable(_msg)); \
      Notificator::warning(title, _msg); \
} while(0)
#define LOG_ERR_P(title, msgFormat, ...) \
do { \
      QString _msg = QString::asprintf(msgFormat, ##__VA_ARGS__); \
      LOG_E("%s: %s", qPrintable(title), qPrintable(_msg)); \
      Notificator::error(title, _msg); \
} while(0)
#define LOG_CRIT_P(title, msgFormat, ...) \
do { \
      QString _msg = QString::asprintf(msgFormat, ##__VA_ARGS__); \
      LOG_C("%s: %s", qPrintable(title), qPrintable(_msg)); \
      Notificator::critical(title, _msg); \
} while(0)
#define LOG_QUEST_P(title, msgFormat, ...) \
do { \
      QString _msg = QString::asprintf(msgFormat, ##__VA_ARGS__); \
      LOG_C("%s: %s", qPrintable(title), qPrintable(_msg)); \
      Notificator::question(title, _msg); \
} while(0)
#define LOG_OK_P(title, msgFormat, ...) \
do { \
      QString _msg = QString::asprintf(msgFormat, ##__VA_ARGS__); \
      LOG_D("%s: %s", qPrintable(title), qPrintable(_msg)); \
      Notificator::ok(title, _msg); \
} while(0)
#define LOG_PARTIAL_P(title, msgFormat, ...) \
do { \
      QString _msg = QString::asprintf(msgFormat, ##__VA_ARGS__); \
      LOG_C("%s: %s", qPrintable(title), qPrintable(_msg)); \
      Notificator::partialSuccess(title, _msg); \
} while(0)

// with no params
#define LOG_INFO_NP(title, msg) \
do { \
      LOG_I("%s: %s", qPrintable(title), qPrintable(msg)); \
      Notificator::information(title, msg); \
} while(0)
#define LOG_WARN_NP(title, msg) \
do { \
      LOG_W("%s: %s", qPrintable(title), qPrintable(msg)); \
      Notificator::warning(title, msg); \
} while(0)
#define LOG_ERR_NP(title, msg) \
do { \
      LOG_W("%s: %s", qPrintable(title), qPrintable(msg)); \
      Notificator::error(title, msg); \
} while(0)
#define LOG_CRIT_NP(title, msg) \
do { \
      LOG_C("%s: %s", qPrintable(title), qPrintable(msg)); \
      Notificator::critical(title, msg); \
} while(0)
#define LOG_OK_NP(title, msg) \
do { \
      LOG_D("%s: %s", qPrintable(title), qPrintable(msg)); \
      Notificator::ok(title, msg); \
} while(0)
#endif // NOTIFICATORMACRO_H
