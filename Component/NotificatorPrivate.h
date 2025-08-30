#ifndef NOTIFICATORPRIVATE_H
#define NOTIFICATORPRIVATE_H

#include <QtGui/QIcon>

class QLabel;
class QIcon;
class QProgressBar;
class QPropertyAnimation;

class NotificatorPrivate {
public:
  NotificatorPrivate(bool autoHide = true);
  ~NotificatorPrivate();

  void initialize(const QIcon& icon, const QString& title, const QString& message);

public:
  bool autoHide() const;
  QLabel* icon();
  QLabel* title();
  QLabel* message();
  QLabel* preloader();
  QProgressBar* progress();

private:
  bool m_autoHide;
  QLabel* m_icon {nullptr};
  QLabel* m_title {nullptr};
  QLabel* m_message {nullptr};
  QLabel* m_preloader {nullptr};
  QProgressBar* m_progress {nullptr};
};

#endif  // NOTIFICATORPRIVATE_H
