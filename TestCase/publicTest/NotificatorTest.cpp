#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>
#include <QLabel>
#include <QToolButton>
#include <QProgressBar>
#include <QGuiApplication>
#include <QScreen>
#include "Logger.h"
#include "BeginToExposePrivateMember.h"
#include "Notificator.h"
#include "EndToExposePrivateMember.h"

class NotificatorTest : public PlainTestSuite {
  Q_OBJECT
public:
  NotificatorTest() : PlainTestSuite{} {
    LOG_D("NotificatorTest object created\n");
  }
  const bool mFromToBottomBkp{Notificator::m_isTopToBottom};
  static constexpr int DEFAULT_NTY_TIMEOUT{500};
private slots:
  void init() {
    Notificator::m_isTopToBottom = mFromToBottomBkp;
    Notificator::instances.clear();
  }
  void cleanup() {
    Notificator::m_isTopToBottom = mFromToBottomBkp;
    Notificator::instances.clear();
  }

  void balloons_tile_from_top_to_bottom_wrapped_ok() {
    const auto screens = QGuiApplication::screens();
    QVERIFY(!screens.isEmpty());
    Notificator::m_isTopToBottom = true;

    QRect screenRect = screens.front()->geometry();
    const int CURRENT_SCEEN_HEIGHT = screenRect.height();

    int heightOcuppied = 0;

    Notificator::showMessage(LOG_LVL_E::I, "top to balloon", "tiles", DEFAULT_NTY_TIMEOUT);
    QVERIFY(Notificator::instances.size() == 1);

    // firstNtyRect is in the right-top cornner of current sceen
    const QRect firstNtyRect = Notificator::instances.back()->geometry();
    heightOcuppied += firstNtyRect.height();
    QVERIFY(firstNtyRect.top() - screenRect.top() <= 100);
    QVERIFY(screenRect.right() - firstNtyRect.right() <= 100);

    // push_back until wrap happend
    for (int cardIndex = 1; cardIndex < 100; ++cardIndex) {
      QString titleStr{"title" + QString::number(cardIndex)};
      QString msgStr{"message" + QString::number(cardIndex)};
      Notificator::showMessage(LOG_LVL_E::I, titleStr, msgStr, DEFAULT_NTY_TIMEOUT);
      QVERIFY2(!Notificator::instances.empty(), qPrintable(titleStr));
      heightOcuppied += Notificator::instances.back()->geometry().height();
      while (heightOcuppied >= CURRENT_SCEEN_HEIGHT) {
        break;
      }
    }
    Notificator::showMessage(LOG_LVL_E::I, "wrapped one", "", DEFAULT_NTY_TIMEOUT);
    QVERIFY(Notificator::instances.size() >= 2);

    // wrappedNtyRect is also in the right-top cornner of current sceen
    const QRect wrappedNtyRect = Notificator::instances.back()->geometry();
    QVERIFY(wrappedNtyRect.top() - screenRect.top() <= 100);
    QVERIFY(screenRect.right() - wrappedNtyRect.right() <= 100);

    QVERIFY(!Notificator::instances.empty());
  }

  void balloons_tile_from_bottom_to_top_wrapped_ok() {
    const auto screens = QGuiApplication::screens();
    QVERIFY(!screens.isEmpty());
    Notificator::m_isTopToBottom = false;

    QRect screenRect = screens.front()->geometry();
    const int CURRENT_SCEEN_HEIGHT = screenRect.height();

    int heightOcuppied = 0;

    Notificator::showMessage(LOG_LVL_E::I, "bottom to top", "tiles", DEFAULT_NTY_TIMEOUT);
    QVERIFY(Notificator::instances.size() == 1);

    // firstNtyRect is in the right-bottom cornner of current sceen
    const QRect firstNtyRect = Notificator::instances.back()->geometry();
    heightOcuppied += firstNtyRect.height();
    QVERIFY(screenRect.bottom() - firstNtyRect.bottom() <= 100);
    QVERIFY(screenRect.right() - firstNtyRect.right() <= 100);

    // push_back until wrap happend
    for (int cardIndex = 1; cardIndex < 100; ++cardIndex) {
      QString titleStr{"title" + QString::number(cardIndex)};
      QString msgStr{"message" + QString::number(cardIndex)};
      Notificator::showMessage(LOG_LVL_E::I, titleStr, msgStr, DEFAULT_NTY_TIMEOUT);
      QVERIFY2(!Notificator::instances.empty(), qPrintable(titleStr));
      heightOcuppied += Notificator::instances.back()->geometry().height();
      while (heightOcuppied >= CURRENT_SCEEN_HEIGHT) {
        break;
      }
    }
    Notificator::showMessage(LOG_LVL_E::I, "wrapped one", "", DEFAULT_NTY_TIMEOUT);
    QVERIFY(Notificator::instances.size() >= 2);

    // wrappedNtyRect is also in the right-bottom cornner of current sceen
    const QRect wrappedNtyRect = Notificator::instances.back()->geometry();
    QVERIFY(screenRect.bottom() - wrappedNtyRect.bottom() <= 100);
    QVERIFY(screenRect.right() - wrappedNtyRect.right() <= 100);
  }

  void click_closeBtn_freeMe_ok() {
    Notificator::showMessage(LOG_LVL_E::I, "CloseButton", "Clicked will Call FreeMe", DEFAULT_NTY_TIMEOUT);
    QCOMPARE(Notificator::instances.size(), 1);
    QVERIFY(Notificator::instances.front() != nullptr);
    auto* pFirst = Notificator::instances.front().get();
    QVERIFY( pFirst != nullptr);
    QCOMPARE(pFirst->m_title->text(), "CloseButton");
    QCOMPARE(pFirst->m_message->text(), "Clicked will Call FreeMe");
    QVERIFY(pFirst->m_progress == nullptr);

    emit pFirst->m_closeBtn->clicked();
    QVERIFY(Notificator::instances.empty());
  }

  void timeout_will_call_FreeMe_ok() {
    Notificator::showMessage(LOG_LVL_E::I, "Auto Close Timer", "Timeout will Call FreeMe", DEFAULT_NTY_TIMEOUT);
    QCOMPARE(Notificator::instances.size(), 1);
    QVERIFY(Notificator::instances.front() != nullptr);
    auto* pFirst = Notificator::instances.front().get();
    QVERIFY( pFirst != nullptr);
    QCOMPARE(pFirst->m_title->text(), "Auto Close Timer");
    QCOMPARE(pFirst->m_message->text(), "Timeout will Call FreeMe");
    QCOMPARE(pFirst->mTimeOutLen, DEFAULT_NTY_TIMEOUT);
    QCOMPARE(pFirst->mAutoCloser.isActive(), true);
    QCOMPARE(pFirst->mAutoCloser.isSingleShot(), true);
    QCOMPARE(pFirst->mAutoCloser.interval(), DEFAULT_NTY_TIMEOUT);
    QVERIFY(pFirst->m_progress == nullptr);

    QTest::qWait(DEFAULT_NTY_TIMEOUT + 50);  // wait 1.5s until timeout
    QVERIFY(Notificator::instances.empty());
  }

  void finished_signal_will_call_FreeMe_ok() {
    QToolButton btn;
    btn.setText("Finished btn");
    Notificator::progress(LOG_LVL_E::O, "Signal", "Emit will Call FreeMe", &btn, SIGNAL(clicked(bool)));

    QCOMPARE(Notificator::instances.size(), 1);
    QVERIFY(Notificator::instances.front() != nullptr);
    auto* pFirst = Notificator::instances.front().get();
    QVERIFY( pFirst != nullptr);
    QCOMPARE(pFirst->m_title->text(), "Signal");
    QCOMPARE(pFirst->m_message->text(), "Emit will Call FreeMe");
    QCOMPARE(pFirst->mTimeOutLen, 0); //
    QCOMPARE(pFirst->mAutoCloser.isActive(), false);

    QVERIFY(pFirst->m_progress != nullptr);
    QCOMPARE(pFirst->m_progress->value(), 0);
    pFirst->setProgressValue(50);
    QCOMPARE(pFirst->m_progress->value(), 50);

    emit btn.clicked(false); // finished -> freeMe then popup finishedBalloon
    QCOMPARE(Notificator::instances.size(), 1);

    Notificator::instances.clear();
    emit btn.clicked(false); // nothing should happened
    QCOMPARE(Notificator::instances.size(), 0);
  }

  void progress100_will_call_FreeMe_ok() {
    QToolButton btn;
    btn.setText("Finished btn");

    Notificator::progress(LOG_LVL_E::O, "Progess 100", "Will Call FreeMe", nullptr, nullptr);

    QCOMPARE(Notificator::instances.size(), 1);
    QVERIFY(Notificator::instances.front() != nullptr);
    auto* pFirst = Notificator::instances.front().get();
    QVERIFY( pFirst != nullptr);
    QCOMPARE(pFirst->m_title->text(), "Progess 100");
    QCOMPARE(pFirst->m_message->text(), "Will Call FreeMe");
    QCOMPARE(pFirst->mTimeOutLen, 0); //
    QCOMPARE(pFirst->mAutoCloser.isActive(), false);

    QVERIFY(pFirst->m_progress != nullptr);
    QCOMPARE(pFirst->m_progress->value(), 0);

    emit btn.clicked(false); // nothing should happened
    auto* pFirstAgain = Notificator::instances.front().get();
    QCOMPARE(pFirstAgain, pFirst);

    pFirst->setProgressValue(50);
    QCOMPARE(pFirst->m_progress->value(), 50);
    QCOMPARE(Notificator::instances.size(), 1);

    // FreeMe then "finshed hint" Balloon pop out
    pFirst->setProgressValue(100);
    QCOMPARE(Notificator::instances.size(), 1);

    QVERIFY(Notificator::instances.front() != nullptr);
    auto* pFinshedBalloon = Notificator::instances.front().get();
    QVERIFY( pFinshedBalloon != nullptr);
    QVERIFY(pFinshedBalloon->m_title != nullptr);
    const QString titleOfBalloon = pFinshedBalloon->m_title->text();
    QVERIFY(titleOfBalloon.contains("finished", Qt::CaseInsensitive));
  }
};
constexpr int NotificatorTest::DEFAULT_NTY_TIMEOUT;

#include "NotificatorTest.moc"
REGISTER_TEST(NotificatorTest, false)
