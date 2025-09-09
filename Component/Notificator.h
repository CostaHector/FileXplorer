#ifndef NOTIFICATOR_H
#define NOTIFICATOR_H
/* from https://gist.github.com/dimkanovikov/7cf62ff0fdf833512d33b1134d434de5
 * by dimkanovikov/notificator.cpp
 */

#include <QFrame>
#include <QTimer>
#include "Logger.h"

class QLabel;
class QProgressBar;
class QToolButton;
class QPropertyAnimation;

class Notificator : public QFrame {
  Q_OBJECT
public:
  // SYSTEM_OPERATIONAL_LOG
  static void debug(const QString& title, const QString& message);
  static void information(const QString& title, const QString& message);
  static void warning(const QString& title, const QString& message);
  static void error(const QString& title, const QString& message);
  static void critical(const QString& title, const QString& message);
  static void fatal(const QString& title, const QString& message);
  // USER_INTERACTION_LOG
  static void question(const QString& title, const QString& message);
  static void ok(const QString& title, const QString& message);
  static void partialSuccess(const QString& title, const QString& message);

  static void showMessage(LOG_LVL_E level, const QString& title, const QString& message, int timeLength);
  static Notificator* progress(LOG_LVL_E level, const QString& title, const QString& message, const QObject* sender = nullptr, const char* finishedSignal = nullptr);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  void hoverEnterEvent(QHoverEvent* event) override;
  void hoverLeaveEvent(QHoverEvent* event) override;
#else
  bool event(QEvent* event) override;
#endif

public slots:
  void setProgressValue(int _value);

private slots:
  void whenProgressFinished();

private:
  explicit Notificator(LOG_LVL_E lvl, int timeoutLength);
  void FreeMe(); // only when in unitest it's a slot
  void notify(const QString& title, const QString& message);
  void initializeLayout();
  void initializeUI();
  void moveToCorrectPosition();

  const int       mTimeOutLen;
  QTimer          mAutoCloser;
  QLabel*         m_icon {nullptr};
  QLabel*         m_title {nullptr};
  QLabel*         m_message {nullptr};
  QToolButton*    m_closeBtn {nullptr};
  QLabel*         m_preloader {nullptr};
  QProgressBar*   m_progress {nullptr};
  const LOG_LVL_E m_lvl {LOG_LVL_E::D};

  static bool   m_isTopToBottom;
  static void freeHiddenInstance();
  static std::list<std::unique_ptr<Notificator>> instances;
};

#endif  // NOTIFICATOR_H
