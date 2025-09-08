#include "Notificator.h"
#include "PublicMacro.h"
#include "Logger.h"
#include <QProgressBar>
#include <QGridLayout>
#include <QLabel>
#include <QIcon>
#include <QMovie>
#include <QScreen>
#include <QGuiApplication>

namespace {
constexpr int SHORT_MESSAGE_SHOW_TIME = 3000;
constexpr int LONG_MESSAGE_SHOW_TIME = 5000;
constexpr float WINDOW_TRANSPARENT_OPACITY = 0.7;
constexpr float WINDOW_NONTRANSPARENT_OPACITY = 1.0;
constexpr int ICON_SIZE = 32;
constexpr int PRELOADER_SIZE = 20;
}

bool Notificator::m_isTopToBottom = false;
std::list<std::unique_ptr<Notificator>> Notificator::instances;

Notificator::Notificator(int timeoutLength)//
  : QFrame{nullptr}, mAutoCloser{this}, mTimeOutLen{timeoutLength} {
  hide();
  m_icon = new (std::nothrow) QLabel{this};
  CHECK_NULLPTR_RETURN_VOID(m_icon);
  m_icon->setObjectName("icon");
  m_icon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  m_title = new (std::nothrow) QLabel{this};
  CHECK_NULLPTR_RETURN_VOID(m_title);
  m_title->setObjectName("title");
  m_title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  m_message = new (std::nothrow) QLabel{this};
  CHECK_NULLPTR_RETURN_VOID(m_message);
  m_message->setObjectName("message");
  m_message->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  m_message->setTextFormat(Qt::RichText);
  m_message->setOpenExternalLinks(true);
  m_message->setTextInteractionFlags(Qt::LinksAccessibleByMouse);

  if (mTimeOutLen <= 0) {
    m_preloader = new (std::nothrow) QLabel{this};
    CHECK_NULLPTR_RETURN_VOID(m_preloader);
    m_preloader->setObjectName("preloader");
    m_preloader->setMinimumSize(PRELOADER_SIZE, PRELOADER_SIZE);
    m_preloader->setMaximumSize(PRELOADER_SIZE, PRELOADER_SIZE);
    m_preloader->setScaledContents(true);
    // Настройка анимации прелоадера
    QMovie* preloaderAnimation = new (std::nothrow) QMovie(":img/LOADING", {}, this);
    CHECK_NULLPTR_RETURN_VOID(preloaderAnimation);
    preloaderAnimation->setScaledSize(m_preloader->size());
    m_preloader->setMovie(preloaderAnimation);
    preloaderAnimation->start();

    m_progress = new (std::nothrow) QProgressBar{this};
    CHECK_NULLPTR_RETURN_VOID(m_progress);
    m_progress->setObjectName("progress");
    m_progress->setRange(0, 100);
    m_progress->setValue(0);
#ifdef Q_OS_WIN
    m_progress->setMaximumHeight(20);
#else
    m_progress->setMaximumHeight(26);
#endif
    m_progress->setTextVisible(false);
  }

  initializeLayout();
  initializeUI();
  hide();
}

void Notificator::goodNews(const QString& title, const QString& message) {
#ifndef RUNNING_UNIT_TESTS
  static const QIcon CORRECT_ICON{":img/CORRECT"};
  showMessage(CORRECT_ICON, title, message, SHORT_MESSAGE_SHOW_TIME);
#endif
}

void Notificator::badNews(const QString& title, const QString& message) {
#ifndef RUNNING_UNIT_TESTS
  static const QIcon WRONG_ICON{":img/WRONG"};
  showMessage(WRONG_ICON, title, message, LONG_MESSAGE_SHOW_TIME);
#endif
}

void Notificator::critical(const QString& title, const QString& message) {
#ifndef RUNNING_UNIT_TESTS
  static const QIcon CRITICAL_ICON{":img/LOG_LEVEL_CRITICAL"};
  showMessage(CRITICAL_ICON, title, message, LONG_MESSAGE_SHOW_TIME);
#endif
}

void Notificator::warning(const QString& title, const QString& message) {
#ifndef RUNNING_UNIT_TESTS
  static const QIcon WARNING_ICON{":img/LOG_LEVEL_WARNING"};
  showMessage(WARNING_ICON, title, message, LONG_MESSAGE_SHOW_TIME);
#endif
}

void Notificator::information(const QString& title, const QString& message) {
#ifndef RUNNING_UNIT_TESTS
  static const QIcon INFO_ICON{":img/LOG_LEVEL_INFO"};
  showMessage(INFO_ICON, title, message, SHORT_MESSAGE_SHOW_TIME);
#endif
}

void Notificator::question(const QString& title, const QString& message) {
#ifndef RUNNING_UNIT_TESTS
  static const QIcon QUESTION_ICON{":img/LOG_LEVEL_QUESTION"};
  showMessage(QUESTION_ICON, title, message, SHORT_MESSAGE_SHOW_TIME);
#endif
}

void Notificator::FreeMe() {
  instances.remove_if([this](const std::unique_ptr<Notificator>& ptr) {
    return ptr != nullptr && ptr.get() == this;
  });
}

void Notificator::showMessage(const QIcon& icon, const QString& title, const QString& message, int timeLength) {
  freeHiddenInstance();
  auto pTemp = new (std::nothrow) Notificator{timeLength};
  CHECK_NULLPTR_RETURN_VOID(pTemp);
  if (pTemp->mTimeOutLen > 0) {
    pTemp->mAutoCloser.setInterval(pTemp->mTimeOutLen);
    pTemp->mAutoCloser.setSingleShot(true);
    Notificator::connect(&pTemp->mAutoCloser, &QTimer::timeout, pTemp, &Notificator::FreeMe);
    pTemp->mAutoCloser.start();
  }
  auto pNty = std::unique_ptr<Notificator>(pTemp);
  pNty->notify(icon, title, message);
  instances.push_back(std::move(pNty));
}

Notificator* Notificator::showMessage(const QIcon& icon, const QString& title, const QString& message, const QObject* sender, const char* finishedSignal) {
  freeHiddenInstance();

  auto pTemp = new (std::nothrow) Notificator{0};
  CHECK_NULLPTR_RETURN_NULLPTR(pTemp);
  if (sender != nullptr) {
    // attention if nullptr sender is, this ballon will only be free when progress set to 100
    connect(sender, finishedSignal, pTemp, SLOT(whenProgressFinished()));
  }

  auto pNty = std::unique_ptr<Notificator>(pTemp);
  pNty->notify(icon, title, message);
  instances.push_back(std::move(pNty));
  return pTemp;
}

void Notificator::setProgressValue(int _value) {
  CHECK_NULLPTR_RETURN_VOID(m_progress);
  m_progress->setValue(_value);
  if (_value >= 100) {
    whenProgressFinished();
  }
}

void Notificator::whenProgressFinished() {
  if (mTimeOutLen > 0) {
    LOG_W("It should freed by one time timer. not me");
    return;
  }
  QString finishedTitle = "[Finished] " + m_title->text();
  QString finishedMessage = m_message->text(); // copy it. because free me will release all memebers
  FreeMe();
  static const QIcon TASK_FINISHED_ICON{":img/TASK_FINISHED"};
  Notificator::showMessage(TASK_FINISHED_ICON, finishedTitle, finishedMessage, LONG_MESSAGE_SHOW_TIME);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void Notificator::hoverEnterEvent(QHoverEvent* event) {
  setWindowOpacity(WINDOW_NONTRANSPARENT_OPACITY);
  if (m_timeOutLen > 0) {
    mAutoCloser.stop();
  }
  event->accept(); // prevent parent widget process this event
}
void Notificator::hoverLeaveEvent(QHoverEvent* event) {
  setWindowOpacity(WINDOW_TRANSPARENT_OPACITY);
  if (m_timeOutLen > 0) {
    mAutoCloser.start();
  }
  event->accept();
}
#else
bool Notificator::event(QEvent* event) {
  switch(event->type()) {
    case QEvent::HoverEnter:  {
      setWindowOpacity(WINDOW_NONTRANSPARENT_OPACITY);
      if (mTimeOutLen > 0) {
        mAutoCloser.stop();
      }
      return true; // all event are finished
    }
    case QEvent::HoverLeave: {
      setWindowOpacity(WINDOW_TRANSPARENT_OPACITY);
      if (mTimeOutLen > 0) {
        mAutoCloser.start();
      }
      return true;
    }
    case QEvent::MouseButtonDblClick: {
      Logger::OpenLogFile();
      return true;
    }
    default:
      return QFrame::event(event);
  }
}
#endif

void Notificator::notify(const QIcon& icon, const QString& title, const QString& message) {
  {
    m_icon->setPixmap(icon.pixmap(ICON_SIZE));
    m_title->setVisible(!title.isEmpty());
    m_title->setText(title);
    m_message->setText(message);
  }

  moveToCorrectPosition();
  show();
}

void Notificator::initializeLayout() {
  QGridLayout* layout = new (std::nothrow) QGridLayout;
  CHECK_NULLPTR_RETURN_VOID(layout);
  layout->addWidget(m_icon, 0, 0, 2, 1, Qt::AlignTop);
  layout->addWidget(m_title, 0, 1);
  if (mTimeOutLen <= 0) {
    layout->addWidget(m_preloader, 0, 2, 1, 1, Qt::AlignRight);
  }
  layout->addWidget(m_message, 1, 1, 1, 2);
  if (mTimeOutLen <= 0) {
    layout->addWidget(m_progress, 2, 1, 1, 2);
  }
  setLayout(layout);
}

void Notificator::initializeUI() {
  setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
  setAttribute(Qt::WA_Hover, true);
  setStyleSheet(
      "Notificator { background-color: black; border: none; }"
      "QLabel { color: white; }"
      "QLabel#title { font-weight: bold; }"
      "QProgressBar { border: 1px solid black; text-align: top; background: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #fff, stop: 0.4999 "
      "#eee, stop: 0.5 #ddd, stop: 1 #eee ); }"
      "QProgressBar::chunk { background: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #b5e2f9, stop: 0.4999 #68c2f3, stop: 0.5 #67bff0, "
      "stop: 1 #1e9bda );  }"
      //				"QProgressBar::chunk { background: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #9bd66d, stop: 0.4999
      // #81d142, stop: 0.5 #81d143, stop: 1 #58bf08 );  }"
      );
  setAutoFillBackground(true);
  setWindowOpacity(WINDOW_TRANSPARENT_OPACITY);
}

void Notificator::moveToCorrectPosition() {
  static const auto screens = QGuiApplication::screens();
  if (screens.isEmpty()) {return;}

  QRect ntfRect = screens.front()->geometry();
  static const int MAX_HEIGHT = ntfRect.height();
  const QSize size = sizeHint();

  static constexpr int FOR_SYS_UI_MARGIN = 20;
  // set initial (x, y) coordinates
  // isTopToBottom: preserve n pixel space at top for system UI
  // !isTopToBottom: preserve n pixel space at bottom for system UI
  ntfRect.setX(ntfRect.right() - size.width());
  ntfRect.setY(m_isTopToBottom ? 0 : ntfRect.bottom() - size.height());
  ntfRect.translate(0, (m_isTopToBottom ? FOR_SYS_UI_MARGIN : -FOR_SYS_UI_MARGIN));

  const Notificator* keyInstance = nullptr;
  for (const auto& ptr : instances) {
    if (!ptr || ptr.get() == this) continue;
    const Notificator* curInstance = ptr.get();
    if (m_isTopToBottom) { // Track the lowest notification
      if (keyInstance == nullptr || curInstance->geometry().bottom() > keyInstance->geometry().bottom()) {
        keyInstance = curInstance;
      }
    } else { // Track the highest notification
      if (keyInstance == nullptr || curInstance->geometry().top() < keyInstance->geometry().top()) {
        keyInstance = curInstance;
      }
    }
  }
  if (keyInstance != nullptr) {
    // adjust y coordinate
    // isTopToBottom: Place below the lowest notification
    // !isTopToBottom: Place above the highest notification
    static constexpr int BETWEEN_NOTIFICATONS_MARGIN = 1;
    const int calculatedY = m_isTopToBottom ? keyInstance->geometry().bottom() + BETWEEN_NOTIFICATONS_MARGIN
                                            : keyInstance->geometry().top() - size.height() - BETWEEN_NOTIFICATONS_MARGIN;
    const int modulus = calculatedY % MAX_HEIGHT;
    ntfRect.setY(modulus >= 0 ?  modulus: modulus + MAX_HEIGHT);
  }
  ntfRect.setSize(size);
  setGeometry(ntfRect);
}

void Notificator::freeHiddenInstance() {
  instances.remove_if([](const std::unique_ptr<Notificator>& ptr) {
    return ptr == nullptr || ptr->isHidden();
  });
}

