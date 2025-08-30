#include "NotificatorPrivate.h"
#include <QLabel>
#include <QString>
#include <QProgressBar>
#include <QMovie>

namespace {
const int ICON_SIZE = 32;
const int PRELOADER_SIZE = 20;
}  // namespace

NotificatorPrivate::NotificatorPrivate(bool autohide) : m_autoHide(autohide) {}

NotificatorPrivate::~NotificatorPrivate() {
  if (m_icon != nullptr && m_icon->parent() == nullptr) {
    delete m_icon;
    m_icon = nullptr;
  }
  if (m_title != nullptr && m_title->parent() == nullptr) {
    delete m_title;
    m_title = nullptr;
  }
  if (m_message != nullptr && m_message->parent() == nullptr) {
    delete m_message;
    m_message = nullptr;
  }
  if (m_preloader != nullptr && m_preloader->parent() == nullptr) {
    delete m_preloader;
    m_preloader = nullptr;
  }

  if (m_progress != nullptr && m_preloader->parent() == nullptr) {
    delete m_progress;
    m_progress = nullptr;
  }
}

void NotificatorPrivate::initialize(const QIcon& icon, const QString& title, const QString& message) {
  this->icon()->setPixmap(icon.pixmap(ICON_SIZE));
  this->title()->setVisible(!title.isEmpty());
  this->title()->setText(title);
  this->message()->setText(QString(message).replace("\n", "<br/>"));
  // Если сообщение не настроено на отображение какого-либо процесса,
  // то и ни к чему отображать прелоадер
  this->preloader()->setVisible(!autoHide());
  this->progress()->hide();
}

bool NotificatorPrivate::autoHide() const {
  return m_autoHide;
}

QLabel* NotificatorPrivate::icon() {
  if (m_icon == nullptr) {
    m_icon = new (std::nothrow) QLabel;
    m_icon->setObjectName("icon");
    m_icon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  }
  return m_icon;
}

QLabel* NotificatorPrivate::title() {
  if (m_title == nullptr) {
    m_title = new (std::nothrow) QLabel;
    m_title->setObjectName("title");
    m_title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  }
  return m_title;
}

QLabel* NotificatorPrivate::message() {
  if (m_message == nullptr) {
    m_message = new (std::nothrow) QLabel;
    m_message->setObjectName("message");
    m_message->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_message->setTextFormat(Qt::RichText);
    m_message->setOpenExternalLinks(true);
    m_message->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
  }
  return m_message;
}

QLabel* NotificatorPrivate::preloader() {
  if (m_preloader == nullptr) {
    m_preloader = new (std::nothrow) QLabel;
    m_preloader->setObjectName("preloader");
    m_preloader->setMinimumSize(PRELOADER_SIZE, PRELOADER_SIZE);
    m_preloader->setMaximumSize(PRELOADER_SIZE, PRELOADER_SIZE);
    m_preloader->setScaledContents(true);
    // Настройка анимации прелоадера
    QMovie* preloaderAnimation = new QMovie(":/images/Icons/preloader.gif");
    preloaderAnimation->setParent(m_preloader);
    preloaderAnimation->setScaledSize(preloader()->size());
    m_preloader->setMovie(preloaderAnimation);
    preloaderAnimation->start();
  }
  return m_preloader;
}

QProgressBar* NotificatorPrivate::progress() {
  if (m_progress == nullptr) {
    m_progress = new (std::nothrow) QProgressBar;
    m_progress->setObjectName("progress");
#ifdef Q_OS_WIN
    m_progress->setMaximumHeight(20);
#else
    m_progress->setMaximumHeight(26);
#endif
    m_progress->setTextVisible(false);
  }
  return m_progress;
}
