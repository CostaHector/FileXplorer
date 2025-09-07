#ifndef NOTIFICATOR_H
#define NOTIFICATOR_H
/* from https://gist.github.com/dimkanovikov/7cf62ff0fdf833512d33b1134d434de5
 * by dimkanovikov/notificator.cpp
 */

#include <QFrame>
#include <QTimer>
#include <memory>

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
  static void showMessage(const QIcon& icon, const QString& title, const QString& message, int timeLength = 3000);
  static Notificator* showMessage(const QIcon& icon, const QString& title, const QString& message, const QObject* sender, const char* finishedSignal);

public slots:
  void setProgressValue(int _value);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  void hoverEnterEvent(QHoverEvent* event) override;
  void hoverLeaveEvent(QHoverEvent* event) override;
#else
  bool event(QEvent* event) override;
#endif

private:
  explicit Notificator(int timeoutLength);
  void FreeMe();
  void notify(const QIcon& icon, const QString& title, const QString& message);
  void initializeLayout();
  void initializeUI();
  void moveToCorrectPosition();

  const int     mTimeOutLen;
  QTimer        mAutoCloser;
  QLabel*       m_icon {nullptr};
  QLabel*       m_title {nullptr};
  QLabel*       m_message {nullptr};
  QLabel*       m_preloader {nullptr};
  QProgressBar* m_progress {nullptr};

  static void freeHiddenInstance();
  static std::list<std::unique_ptr<Notificator>> instances;
};

#endif  // NOTIFICATOR_H
