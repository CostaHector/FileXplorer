#ifndef NOTIFICATORMACRO_H
#define NOTIFICATORMACRO_H

#include "Notificator.h"

// uniform
#define LOG_GOOD(title, msgFormat, ...) \
do { \
      QString _msg = (sizeof(#__VA_ARGS__) > 1) ? QString::asprintf(msgFormat, ##__VA_ARGS__) : msgFormat; \
      qDebug("%s: %s", qPrintable(title), qPrintable(_msg)); \
      Notificator::goodNews(title, _msg); \
} while(0)
#define LOG_BAD(title, msgFormat, ...) \
do { \
      QString _msg = (sizeof(#__VA_ARGS__) > 1) ? QString::asprintf(msgFormat, ##__VA_ARGS__) : msgFormat; \
      qWarning("%s: %s", qPrintable(title), qPrintable(_msg)); \
      Notificator::badNews(title, _msg); \
} while(0)
#define LOG_WARN(title, msgFormat, ...) \
do { \
      QString _msg = (sizeof(#__VA_ARGS__) > 1) ? QString::asprintf(msgFormat, ##__VA_ARGS__) : msgFormat; \
      qWarning("%s: %s", qPrintable(title), qPrintable(_msg)); \
      Notificator::warning(title, _msg); \
} while(0)
#define LOG_INFO(title, msgFormat, ...) \
do { \
      QString _msg = (sizeof(#__VA_ARGS__) > 1) ? QString::asprintf(msgFormat, ##__VA_ARGS__) : msgFormat; \
      qInfo("%s: %s", qPrintable(title), qPrintable(_msg)); \
      Notificator::information(title, _msg); \
} while(0)
#define LOG_CRIT(title, msgFormat, ...) \
do { \
      QString _msg = (sizeof(#__VA_ARGS__) > 1) ? QString::asprintf(msgFormat, ##__VA_ARGS__) : msgFormat; \
      qCritical("%s: %s", qPrintable(title), qPrintable(_msg)); \
      Notificator::critical(title, _msg); \
} while(0)

// with params
#define LOG_GOOD_P(title, msgFormat, ...) \
do { \
      QString _msg = QString::asprintf(msgFormat, ##__VA_ARGS__); \
      qDebug("%s: %s", qPrintable(title), qPrintable(_msg)); \
      Notificator::goodNews(title, _msg); \
} while(0)
#define LOG_BAD_P(title, msgFormat, ...) \
do { \
      QString _msg = QString::asprintf(msgFormat, ##__VA_ARGS__); \
      qWarning("%s: %s", qPrintable(title), qPrintable(_msg)); \
      Notificator::badNews(title, _msg); \
} while(0)
#define LOG_WARN_P(title, msgFormat, ...) \
do { \
      QString _msg = QString::asprintf(msgFormat, ##__VA_ARGS__); \
      qWarning("%s: %s", qPrintable(title), qPrintable(_msg)); \
      Notificator::warning(title, _msg); \
} while(0)
#define LOG_INFO_P(title, msgFormat, ...) \
do { \
      QString _msg = QString::asprintf(msgFormat, ##__VA_ARGS__); \
      qInfo("%s: %s", qPrintable(title), qPrintable(_msg)); \
      Notificator::information(title, _msg); \
} while(0)
#define LOG_CRIT_P(title, msgFormat, ...) \
do { \
      QString _msg = QString::asprintf(msgFormat, ##__VA_ARGS__); \
      qCritical("%s: %s", qPrintable(title), qPrintable(_msg)); \
      Notificator::critical(title, _msg); \
} while(0)

// with no params
#define LOG_GOOD_NP(title, msg) \
do { \
      qDebug("%s: %s", qPrintable(title), qPrintable(msg)); \
      Notificator::goodNews(title, msg); \
} while(0)
#define LOG_BAD_NP(title, msg) \
do { \
      qWarning("%s: %s", qPrintable(title), qPrintable(msg)); \
      Notificator::badNews(title, msg); \
} while(0)
#define LOG_WARN_NP(title, msg) \
do { \
      qWarning("%s: %s", qPrintable(title), qPrintable(msg)); \
      Notificator::warning(title, msg); \
} while(0)

#define LOG_INFO_NP(title, msg) \
do { \
      qInfo("%s: %s", qPrintable(title), qPrintable(msg)); \
      Notificator::information(title, msg); \
} while(0)
#define LOG_CRIT_NP(title, msg) \
do { \
      qCritical("%s: %s", qPrintable(title), qPrintable(msg)); \
      Notificator::critical(title, msg); \
} while(0)


#endif // NOTIFICATORMACRO_H
