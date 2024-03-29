#ifndef NOTIFICATORFRAME_H
#define NOTIFICATORFRAME_H
/* from https://gist.github.com/dimkanovikov/7cf62ff0fdf833512d33b1134d434de5
 * by dimkanovikov/notificator.cpp
 */

#include <QFrame>
#include "NotificatorFrame_p.h"

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

#endif  // NOTIFICATORFRAME_H
