#include "Notificator.h"
#include "NotificatorMacro.h"
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

constexpr char NTY_GREY_BG_RGB[] = "#F5F5F5";
constexpr char NTY_BLUE_RGB[] = "#155DFF";
constexpr char NTY_BLUE_BG_RGB[] = "#ECF4FF";
constexpr char NTY_GREEN_RGB[] = "#01B328";
constexpr char NTY_GREEN_BG_RGB[] = "#EEFEF1";
constexpr char NTY_RED_RGB[] = "#F44336";
constexpr char NTY_RED_BG_RGB[] = "#FEEAEA";
constexpr char NTY_ORANGE_RGB[] = "#FF7D01";
constexpr char NTY_ORANGE_BG_RGB[] = "#FFF6ED";
}

bool Notificator::m_isTopToBottom = false; // for unit-test purpose only
std::list<std::unique_ptr<Notificator>> Notificator::instances;

Notificator::Notificator(int timeoutLength)//
  : QFrame{nullptr}, mAutoCloser{this}, mTimeOutLen{timeoutLength} {
  hide();
  m_icon = new (std::nothrow) QLabel{this};
  CHECK_NULLPTR_RETURN_VOID(m_icon);
  m_icon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  m_title = new (std::nothrow) QLabel{this};
  CHECK_NULLPTR_RETURN_VOID(m_title);
  m_title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  m_title->setStyleSheet(R"(QLabel { font-weight: bold; })");

  m_message = new (std::nothrow) QLabel{this};
  CHECK_NULLPTR_RETURN_VOID(m_message);
  m_message->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  m_message->setOpenExternalLinks(true);
  m_message->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
  m_message->setStyleSheet(R"(QLabel { color: black; font-family: Arial, Helvetica, sans-serif; })");

  m_closeBtn = new (std::nothrow) QToolButton{this};
  CHECK_NULLPTR_RETURN_VOID(m_closeBtn);
  m_closeBtn->setIcon(QIcon{":img/BALLOON_CLOSE"});
  m_closeBtn->setStyleSheet(R"(QToolButton { background: transparent; border: none; })");

  if (mTimeOutLen <= 0) {
    m_preloader = new (std::nothrow) QLabel{this};
    CHECK_NULLPTR_RETURN_VOID(m_preloader);
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
    m_progress->setRange(0, 100);
    m_progress->setValue(0);
    m_progress->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_progress->setStyleSheet(R"(QProgressBar { border: 1px solid #CCCCCC; background: #FF7D01; text-align: center;}
      QProgressBar::chunk { background: #01B328; })");
    //  QProgressBar::chunk { background: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #9bd66d, stop: 0.4999 #81d142, stop: 0.5 #81d143, stop: 1 #58bf08 );  }
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
  auto pTemp = new (std::nothrow) Notificator{timeLength};
  CHECK_NULLPTR_RETURN_VOID(pTemp);
  if (pTemp->mTimeOutLen > 0) {
    pTemp->mAutoCloser.setInterval(pTemp->mTimeOutLen);
    pTemp->mAutoCloser.setSingleShot(true);
    Notificator::connect(&pTemp->mAutoCloser, &QTimer::timeout, pTemp, &Notificator::FreeMe);
    pTemp->mAutoCloser.start();
  }
  auto pNty = std::unique_ptr<Notificator>(pTemp);
  pNty->notify(level, title, message);
  instances.push_back(std::move(pNty));
}

Notificator* Notificator::progress(LOG_LVL_E level, const QString& title, const QString& message, const QObject* sender, const char* finishedSignal) {
  LOG_E(LOG_TITLE_MESSAGE_PATTERN, qPrintable(title), qPrintable(message));
  freeHiddenInstance();

  auto pTemp = new (std::nothrow) Notificator{0};
  CHECK_NULLPTR_RETURN_NULLPTR(pTemp);
  if (sender != nullptr) {
    // attention if nullptr sender is, this ballon will only be free when progress set to 100
    connect(sender, finishedSignal, pTemp, SLOT(whenProgressFinished()));
  }

  auto pNty = std::unique_ptr<Notificator>(pTemp);
  pNty->notify(level, title, message);
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

void Notificator::setProgressFailed() {
  CHECK_NULLPTR_RETURN_VOID(m_progress);
  QString taskFailedTitle = "[Task Partial Failed] " + m_title->text();
  const QString& taskFailedMessage = m_message->text();
  LOG_E(LOG_TITLE_MESSAGE_PATTERN, qPrintable(taskFailedTitle), qPrintable(taskFailedMessage));

  notify(LOG_LVL_E::P, taskFailedTitle, taskFailedMessage);
}

void Notificator::whenProgressFinished() {
  CHECK_NULLPTR_RETURN_VOID(m_progress);
  QString finishedTitle = "[Finished] " + m_title->text();
  QString finishedMessage = m_message->text(); // copy it. because free me will release all memebers
  FreeMe();
#ifdef RUNNING_UNIT_TESTS
  showMessage(LOG_LVL_E::O, finishedTitle, finishedMessage, SHORT_MESSAGE_SHOW_TIME);
#else
  LOG_OK_NP(finishedTitle, finishedMessage);
#endif
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

void Notificator::notify(LOG_LVL_E level, const QString& title, const QString& message) {
  {
    static const QPixmap LOG_LEVEL_2_PIXMAP[(int)LOG_LVL_E::BUTT + 1]//
        {
         QPixmap{":img/LOG_LEVEL_DEBUG"}.scaledToWidth(NTY_ICON_SIZE, Qt::SmoothTransformation),
         QPixmap{":img/LOG_LEVEL_INFO"}.scaledToWidth(NTY_ICON_SIZE, Qt::SmoothTransformation),
         QPixmap{":img/LOG_LEVEL_WARNING"}.scaledToWidth(NTY_ICON_SIZE, Qt::SmoothTransformation),
         QPixmap{":img/LOG_LEVEL_ERROR"}.scaledToWidth(NTY_ICON_SIZE, Qt::SmoothTransformation),
         QPixmap{":img/LOG_LEVEL_CRITICAL"}.scaledToWidth(NTY_ICON_SIZE, Qt::SmoothTransformation),
         QPixmap{":img/LOG_LEVEL_FATAL"}.scaledToWidth(NTY_ICON_SIZE, Qt::SmoothTransformation),
         QPixmap{":img/LOG_LEVEL_QUESTION"}.scaledToWidth(NTY_ICON_SIZE, Qt::SmoothTransformation),
         QPixmap{":img/LOG_LEVEL_OK"}.scaledToWidth(NTY_ICON_SIZE, Qt::SmoothTransformation),
         QPixmap{":img/LOG_LEVEL_PARTIAL_FAILED"}.scaledToWidth(NTY_ICON_SIZE, Qt::SmoothTransformation),
         };
    m_icon->setPixmap(LOG_LEVEL_2_PIXMAP[(int)level]);
    m_title->setText(title);
    m_message->setText(message);
  }
  {
    static const QString styleSheet {R"(Notificator { background-color: %1; })"};
    static QString LOG_LEVEL_2_COLOR[(int)LOG_LVL_E::BUTT + 1] {
        styleSheet.arg(NTY_GREY_BG_RGB),   // D
        styleSheet.arg(NTY_BLUE_BG_RGB),   // I
        styleSheet.arg(NTY_ORANGE_BG_RGB), // W
        styleSheet.arg(NTY_RED_BG_RGB),    // E
        styleSheet.arg(NTY_RED_BG_RGB),    // C
        styleSheet.arg(NTY_RED_BG_RGB),    // F
        styleSheet.arg(NTY_BLUE_BG_RGB),   // Q
        styleSheet.arg(NTY_GREEN_BG_RGB),  // O
        styleSheet.arg(NTY_ORANGE_BG_RGB), // P
    };
    setStyleSheet(LOG_LEVEL_2_COLOR[(int)level]);
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
    layout->addWidget(m_progress, 2, 0, 1, 2, Qt::AlignLeft);
    layout->addWidget(m_preloader, 2, 2, 1, 1);
  }
  layout->setSpacing(0);
  layout->setContentsMargins(5, 2, 5, 2);
  setLayout(layout);
}

void Notificator::initializeUI() {
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

  // get the correct y coordinate
  static const auto GetYPosition = [](Notificator* itself, const int ntfRectBottom, const int hintSizeHeight) -> int {
    const Notificator* keyInstance = nullptr;
    for (const auto& ptr : instances) {
      if (ptr == nullptr) continue;
      if (ptr.get() == itself) {
        // 1. balloons usually should not exists in instances pool, unless this ballon is the one only need move left serveral pixels to show extra failed message,
        // In this situation, move up/down is unnecessary, inherit itself former y and return;
        return itself->geometry().y();
      }
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
    // 2. only new balloons need to adjust its y position, old ballon inherit its origin y position
    if (keyInstance != nullptr) {
      // isTopToBottom: Place below the lowest notification
      // !isTopToBottom: Place above the highest notification
      static constexpr int BETWEEN_NOTIFICATONS_MARGIN = 1;
      const int yReferredkeyInstance = m_isTopToBottom ? keyInstance->geometry().bottom() + BETWEEN_NOTIFICATONS_MARGIN
                                                       : keyInstance->geometry().top() - hintSizeHeight - BETWEEN_NOTIFICATONS_MARGIN;
      return (yReferredkeyInstance % MAX_HEIGHT + MAX_HEIGHT) % MAX_HEIGHT;
    }
    // 3. empty instance pool
    // isTopToBottom: preserve n pixel space at top for system UI
    // !isTopToBottom: preserve n pixel space at bottom for system UI
    static constexpr int FOR_SYS_UI_MARGIN = 20;
    const int initialY = m_isTopToBottom ? 0 + FOR_SYS_UI_MARGIN: ntfRectBottom- hintSizeHeight - FOR_SYS_UI_MARGIN;
    return initialY;
  };
  const QSize hintSize = sizeHint();
  const int x = ntfRect.right() - hintSize.width();
  const int y = GetYPosition(this, ntfRect.bottom(), hintSize.height());
  ntfRect.setX(x);
  ntfRect.setY(y);
  ntfRect.setSize(hintSize);

  setGeometry(ntfRect);
}

void Notificator::freeHiddenInstance() {
  instances.remove_if([](const std::unique_ptr<Notificator>& ptr) {
    return ptr == nullptr || ptr->isHidden();
  });
}

