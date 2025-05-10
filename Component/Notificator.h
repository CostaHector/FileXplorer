#ifndef NOTIFICATOR_H
#define NOTIFICATOR_H
/* from https://gist.github.com/dimkanovikov/7cf62ff0fdf833512d33b1134d434de5
 * by dimkanovikov/notificator.cpp
 */

#include <QFrame>
#include "NotificatorPrivate.h"

class QLabel;
class QIcon;
class QProgressBar;
class QPropertyAnimation;

class Notificator : public QFrame {
  Q_OBJECT

 public:
  static void goodNews(const QString& title, const QString& message);
  static void badNews(const QString& title, const QString& message);
  static void critical(const QString& title, const QString& message);
  static void warning(const QString& title, const QString& message);
  static void information(const QString& title, const QString& message);
  static void question(const QString& title, const QString& message);
  static void showMessage(const QIcon& icon, const QString& title, const QString& message);
  static Notificator* showMessage(const QIcon& icon, const QString& title, const QString& message, const QObject* sender, const char* finishedSignal);

 public slots:
  void setMessage(const QString& _message);
  void setProgressValue(int _value);

 protected:
  bool event(QEvent*);

 private:
  Notificator(bool autohide = true);
  ~Notificator();

  void notify(const QIcon& icon, const QString& title, const QString& message);

 private:
  void initializeLayout();
  void initializeUI();
  void correctPosition();

 private:
  NotificatorPrivate* d;

  static void configureInstance(Notificator* notificator);
  static QList<Notificator*> instances;
};

#define LOG_GOOD(title, message)                           \
  qDebug("%s:%s", qPrintable(title), qPrintable(message)); \
  Notificator::goodNews(title, message);

#define LOG_BAD(title, message)                              \
  qWarning("%s:%s", qPrintable(title), qPrintable(message)); \
  Notificator::badNews(title, message);

#define LOG_INFO(title, message)                          \
  qInfo("%s:%s", qPrintable(title), qPrintable(message)); \
  Notificator::information(title, message);

#define LOG_WARN(title, message)                             \
  qWarning("%s:%s", qPrintable(title), qPrintable(message)); \
  Notificator::warning(title, message);

#define LOG_CRITICAL(title, message)                          \
  qCritical("%s:%s", qPrintable(title), qPrintable(message)); \
  Notificator::critical(title, message);

#endif  // NOTIFICATOR_H
