#include "Notificator.h"
#include "PublicMacro.h"
#include <QProgressBar>
#include <QToolButton>
#include <QGridLayout>
#include <QLabel>
#include <QPixmap>
#include <QMovie>
#include <QScreen>
#include <QGuiApplication>

namespace {
constexpr int SHORT_MESSAGE_SHOW_TIME = 3000;
constexpr int LONG_MESSAGE_SHOW_TIME = 5000;
constexpr float WINDOW_TRANSPARENT_OPACITY = 0.85;
constexpr float WINDOW_NONTRANSPARENT_OPACITY = 1.0;
constexpr int NTY_ICON_SIZE = 20;
}

bool Notificator::m_isTopToBottom = false;
std::list<std::unique_ptr<Notificator>> Notificator::instances;

Notificator::Notificator(LOG_LVL_E lvl, int timeoutLength)//
  : QFrame{nullptr}, m_lvl{lvl}, mAutoCloser{this}, mTimeOutLen{timeoutLength} {
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

  m_closeBtn = new (std::nothrow) QToolButton{this};
  CHECK_NULLPTR_RETURN_VOID(m_closeBtn);
  m_closeBtn->setIcon(QIcon{":img/BALLOON_CLOSE"});

  if (mTimeOutLen <= 0) {
    m_preloader = new (std::nothrow) QLabel{this};
    CHECK_NULLPTR_RETURN_VOID(m_preloader);
    m_preloader->setObjectName("preloader");
    m_preloader->setMinimumSize(NTY_ICON_SIZE, NTY_ICON_SIZE);
    m_preloader->setMaximumSize(NTY_ICON_SIZE, NTY_ICON_SIZE);
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
  }

  initializeLayout();
  initializeUI();
  hide();
  connect(m_closeBtn, &QToolButton::clicked, this, &Notificator::FreeMe);
}

void Notificator::debug(const QString& title, const QString& message) {
#ifndef RUNNING_UNIT_TESTS
  showMessage(LOG_LVL_E::D, title, message, SHORT_MESSAGE_SHOW_TIME);
#endif
}

void Notificator::information(const QString& title, const QString& message) {
#ifndef RUNNING_UNIT_TESTS
  showMessage(LOG_LVL_E::I, title, message, SHORT_MESSAGE_SHOW_TIME);
#endif
}

void Notificator::warning(const QString& title, const QString& message) {
#ifndef RUNNING_UNIT_TESTS
  showMessage(LOG_LVL_E::W, title, message, LONG_MESSAGE_SHOW_TIME);
#endif
}

void Notificator::error(const QString& title, const QString& message) {
#ifndef RUNNING_UNIT_TESTS
  showMessage(LOG_LVL_E::E, title, message, LONG_MESSAGE_SHOW_TIME);
#endif
}

void Notificator::critical(const QString& title, const QString& message) {
#ifndef RUNNING_UNIT_TESTS
  showMessage(LOG_LVL_E::C, title, message, LONG_MESSAGE_SHOW_TIME);
#endif
}

void Notificator::fatal(const QString& title, const QString& message) {
#ifndef RUNNING_UNIT_TESTS
  showMessage(LOG_LVL_E::F, title, message, LONG_MESSAGE_SHOW_TIME);
#endif
}

void Notificator::question(const QString& title, const QString& message) {
#ifndef RUNNING_UNIT_TESTS
  showMessage(LOG_LVL_E::Q, title, message, SHORT_MESSAGE_SHOW_TIME);
#endif
}

void Notificator::ok(const QString& title, const QString& message) {
#ifndef RUNNING_UNIT_TESTS
  showMessage(LOG_LVL_E::O, title, message, SHORT_MESSAGE_SHOW_TIME);
#endif
}

void Notificator::partialSuccess(const QString& title, const QString& message) {
#ifndef RUNNING_UNIT_TESTS
  showMessage(LOG_LVL_E::P, title, message, SHORT_MESSAGE_SHOW_TIME);
#endif
}

void Notificator::FreeMe() {
  instances.remove_if([this](const std::unique_ptr<Notificator>& ptr) {
    return ptr != nullptr && ptr.get() == this;
  });
}

void Notificator::showMessage(LOG_LVL_E level, const QString& title, const QString& message, int timeLength) {
  freeHiddenInstance();
  auto pTemp = new (std::nothrow) Notificator{level, timeLength};
  CHECK_NULLPTR_RETURN_VOID(pTemp);
  if (pTemp->mTimeOutLen > 0) {
    pTemp->mAutoCloser.setInterval(pTemp->mTimeOutLen);
    pTemp->mAutoCloser.setSingleShot(true);
    Notificator::connect(&pTemp->mAutoCloser, &QTimer::timeout, pTemp, &Notificator::FreeMe);
    pTemp->mAutoCloser.start();
  }
  auto pNty = std::unique_ptr<Notificator>(pTemp);
  pNty->notify(title, message);
  instances.push_back(std::move(pNty));
}

Notificator* Notificator::progress(LOG_LVL_E level, const QString& title, const QString& message, const QObject* sender, const char* finishedSignal) {
  freeHiddenInstance();

  auto pTemp = new (std::nothrow) Notificator{level, 0};
  CHECK_NULLPTR_RETURN_NULLPTR(pTemp);
  if (sender != nullptr) {
    // attention if nullptr sender is, this ballon will only be free when progress set to 100
    connect(sender, finishedSignal, pTemp, SLOT(whenProgressFinished()));
  }

  auto pNty = std::unique_ptr<Notificator>(pTemp);
  pNty->notify(title, message);
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
  Notificator::showMessage(LOG_LVL_E::O, finishedTitle, finishedMessage, LONG_MESSAGE_SHOW_TIME);
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

void Notificator::notify(const QString& title, const QString& message) {
  {
    static const QPixmap LOG_LEVEL_2_PIXMAP[(int)LOG_LVL_E::BUTT + 1]//
        {
         QPixmap{":img/LOG_LEVEL_DEBUG"}.scaledToWidth(NTY_ICON_SIZE),
         QPixmap{":img/LOG_LEVEL_INFO"}.scaledToWidth(NTY_ICON_SIZE),
         QPixmap{":img/LOG_LEVEL_WARNING"}.scaledToWidth(NTY_ICON_SIZE),
         QPixmap{":img/LOG_LEVEL_ERROR"}.scaledToWidth(NTY_ICON_SIZE),
         QPixmap{":img/LOG_LEVEL_CRITICAL"}.scaledToWidth(NTY_ICON_SIZE),
         QPixmap{":img/LOG_LEVEL_FATAL"}.scaledToWidth(NTY_ICON_SIZE),
         QPixmap{":img/LOG_LEVEL_QUESTION"}.scaledToWidth(NTY_ICON_SIZE),
         QPixmap{":img/LOG_LEVEL_OK"}.scaledToWidth(NTY_ICON_SIZE),
         QPixmap{":img/PARTIALLY_FAILED"}.scaledToWidth(NTY_ICON_SIZE),
         };
    m_icon->setPixmap(LOG_LEVEL_2_PIXMAP[(int)m_lvl]);
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
  layout->addWidget(m_icon, 0, 0, 1, 1);
  layout->addWidget(m_title, 0, 1, 1, 1, Qt::AlignLeft);
  layout->addWidget(m_closeBtn, 0, 2, 1, 1, Qt::AlignRight);
  layout->addWidget(m_message, 1, 0, 1, 3, Qt::AlignLeft);
  if (mTimeOutLen <= 0) {
    layout->addWidget(m_progress, 2, 0, 1, 2, Qt::AlignHCenter);
    layout->addWidget(m_preloader, 2, 2, 1, 1);
  }
  setLayout(layout);
}

void Notificator::initializeUI() {
  const QString styleSheet//
      {
       R"(Notificator { border: none; background-color: %1; }
        QLabel { color: black; }
        QLabel#title { font-weight: bold; })"};
  static QString LOG_LEVEL_2_COLOR[(int)LOG_LVL_E::BUTT + 1] {
      styleSheet.arg("#F5F5F5"), // D
      styleSheet.arg("#E3F2FD"), // I
      styleSheet.arg("#FFF3E0"), // W
      styleSheet.arg("#FFEBEE"), // E
      styleSheet.arg("#FFEBEE"), // C
      styleSheet.arg("#FFEBEE"), // F
      styleSheet.arg("#E3F2FD"), // Q
      styleSheet.arg("#E8F5E9"), // O
      styleSheet.arg("#FFF3E0"), // P
  };
  setStyleSheet(LOG_LEVEL_2_COLOR[(int)m_lvl]);
  //     "QProgressBar { border: 1px solid black; text-align: top; background: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #fff, stop: 0.4999 #eee, stop: 0.5 #ddd, stop: 1 #eee ); }"
  //     "QProgressBar::chunk { background: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #b5e2f9, stop: 0.4999 #68c2f3, stop: 0.5 #67bff0, stop: 1 #1e9bda );  }"
  //     "QProgressBar::chunk { background: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #9bd66d, stop: 0.4999 #81d142, stop: 0.5 #81d143, stop: 1 #58bf08 );  }"
  setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
  setAttribute(Qt::WA_Hover, true);
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

