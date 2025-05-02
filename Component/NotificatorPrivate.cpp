#include "NotificatorPrivate.h"
#include <QLabel>
#include <QString>
#include <QProgressBar>
#include <QMovie>

namespace {
const int ICON_SIZE = 32;
const int PRELOADER_SIZE = 20;
}  // namespace

NotificatorPrivate::NotificatorPrivate(bool autohide) : m_autoHide(autohide), m_icon(0), m_title(0), m_message(0), m_preloader(0), m_progress(0) {}

NotificatorPrivate::~NotificatorPrivate() {
  if (icon()->parent() == 0) {
    delete icon();
  }
  if (title()->parent() == 0) {
    delete title();
  }
  if (message()->parent() == 0) {
    delete message();
  }
  if (preloader()->parent() == 0) {
    delete preloader();
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
  if (m_icon == 0) {
    m_icon = new QLabel;
    m_icon->setObjectName("icon");
    m_icon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  }
  return m_icon;
}

QLabel* NotificatorPrivate::title() {
  if (m_title == 0) {
    m_title = new QLabel;
    m_title->setObjectName("title");
    m_title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  }
  return m_title;
}

QLabel* NotificatorPrivate::message() {
  if (m_message == 0) {
    m_message = new QLabel;
    m_message->setObjectName("message");
    m_message->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_message->setTextFormat(Qt::RichText);
    m_message->setOpenExternalLinks(true);
    m_message->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
  }
  return m_message;
}

QLabel* NotificatorPrivate::preloader() {
  if (m_preloader == 0) {
    m_preloader = new QLabel;
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
  if (m_progress == 0) {
    m_progress = new QProgressBar;
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
