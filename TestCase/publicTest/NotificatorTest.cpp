#include <QtTest/QtTest>
#include "MyTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QGuiApplication>
#include <QScreen>
#include "BeginToExposePrivateMember.h"
#include "Notificator.h"
#include "EndToExposePrivateMember.h"

class NotificatorTest : public MyTestSuite {
  Q_OBJECT
public:
  NotificatorTest() : MyTestSuite{false} {
    fprintf(stdout, "NotificatorTest object created\n");
    std::fflush(stdout);
  }
  const bool mFromToBottomBkp{Notificator::m_isTopToBottom};
private slots:
  void init() {
    Notificator::m_isTopToBottom = mFromToBottomBkp;
    Notificator::instances.clear();
  }
  void cleanup() {
    Notificator::m_isTopToBottom = mFromToBottomBkp;
    Notificator::instances.clear();
  }

  void cards_tile_from_top_to_bottom_wrapped_ok() {
    const auto screens = QGuiApplication::screens();
    QVERIFY(!screens.isEmpty());
    Notificator::m_isTopToBottom = true;

    QRect screenRect = screens.front()->geometry();
    const int CURRENT_SCEEN_HEIGHT = screenRect.height();

    int heightOcuppied = 0;

    Notificator::showMessage(QIcon{}, "first one", "", 1000);
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
      Notificator::showMessage(QIcon{}, titleStr, msgStr, 1000);
      QVERIFY2(!Notificator::instances.empty(), qPrintable(titleStr));
      heightOcuppied += Notificator::instances.back()->geometry().height();
      while (heightOcuppied >= CURRENT_SCEEN_HEIGHT) {
        break;
      }
    }
    Notificator::showMessage(QIcon{}, "wrapped one", "", 1000);
    QVERIFY(Notificator::instances.size() >= 2);

    // wrappedNtyRect is also in the right-top cornner of current sceen
    const QRect wrappedNtyRect = Notificator::instances.back()->geometry();
    QVERIFY(wrappedNtyRect.top() - screenRect.top() <= 100);
    QVERIFY(screenRect.right() - wrappedNtyRect.right() <= 100);

    QTest::qWait(1000 + 500);  // wait 1.5s until all one time timer timeout
    QVERIFY(Notificator::instances.empty());
  }

  void cards_tile_from_bottom_to_top_wrapped_ok() {
    const auto screens = QGuiApplication::screens();
    QVERIFY(!screens.isEmpty());
    Notificator::m_isTopToBottom = false;

    QRect screenRect = screens.front()->geometry();
    const int CURRENT_SCEEN_HEIGHT = screenRect.height();

    int heightOcuppied = 0;

    Notificator::showMessage(QIcon{}, "first one", "", 1000);
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
      Notificator::showMessage(QIcon{}, titleStr, msgStr, 1000);
      QVERIFY2(!Notificator::instances.empty(), qPrintable(titleStr));
      heightOcuppied += Notificator::instances.back()->geometry().height();
      while (heightOcuppied >= CURRENT_SCEEN_HEIGHT) {
        break;
      }
    }
    Notificator::showMessage(QIcon{}, "wrapped one", "", 1000);
    QVERIFY(Notificator::instances.size() >= 2);

    // wrappedNtyRect is also in the right-bottom cornner of current sceen
    const QRect wrappedNtyRect = Notificator::instances.back()->geometry();
    QVERIFY(screenRect.bottom() - wrappedNtyRect.bottom() <= 100);
    QVERIFY(screenRect.right() - wrappedNtyRect.right() <= 100);
  }

  void timeoutLenGT0_AutoHideTimerActive_ok() {
    Notificator::showMessage(QIcon{}, "Title: Hello", "Message: world", 1000);
    QCOMPARE(Notificator::instances.size(), 1);
    QVERIFY(Notificator::instances.front() != nullptr);
    auto* pFirst = Notificator::instances.front().get();
    QVERIFY( pFirst != nullptr);
    QCOMPARE(pFirst->m_title->text(), "Title: Hello");
    QCOMPARE(pFirst->m_message->text(), "Message: world");
    QCOMPARE(pFirst->mTimeOutLen, 1000);
    QCOMPARE(pFirst->mAutoCloser.isActive(), true);
    QCOMPARE(pFirst->mAutoCloser.isSingleShot(), true);
    QCOMPARE(pFirst->mAutoCloser.interval(), 1000);
    QVERIFY(pFirst->m_progress == nullptr);
  }

  void finished_signal_drive_FreeMe_ok() {
    QPushButton btn;
    btn.setText("Finished btn");

    Notificator::showMessage(QIcon{}, "Title: Hello", "Message: world", &btn, SIGNAL(clicked(bool)));

    QCOMPARE(Notificator::instances.size(), 1);
    QVERIFY(Notificator::instances.front() != nullptr);
    auto* pFirst = Notificator::instances.front().get();
    QVERIFY( pFirst != nullptr);
    QCOMPARE(pFirst->m_title->text(), "Title: Hello");
    QCOMPARE(pFirst->m_message->text(), "Message: world");
    QCOMPARE(pFirst->mTimeOutLen, 0); //
    QCOMPARE(pFirst->mAutoCloser.isActive(), false);

    QVERIFY(pFirst->m_progress != nullptr);
    QCOMPARE(pFirst->m_progress->value(), 0);
    pFirst->setProgressValue(50);
    QCOMPARE(pFirst->m_progress->value(), 50);

    emit btn.clicked(false); // finished -> freeMe then popup finishedBallon
    QCOMPARE(Notificator::instances.size(), 1);

    Notificator::instances.clear();
    emit btn.clicked(false); // nothing should happened
    QCOMPARE(Notificator::instances.size(), 0);
  }

  void progress100_drive_FreeMe_ok() {
    QPushButton btn;
    btn.setText("Finished btn");

    Notificator::showMessage(QIcon{}, "Title: Hello", "Message: world", nullptr, nullptr);

    QCOMPARE(Notificator::instances.size(), 1);
    QVERIFY(Notificator::instances.front() != nullptr);
    auto* pFirst = Notificator::instances.front().get();
    QVERIFY( pFirst != nullptr);
    QCOMPARE(pFirst->m_title->text(), "Title: Hello");
    QCOMPARE(pFirst->m_message->text(), "Message: world");
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

    // FreeMe then "finshed hint" ballon pop out
    pFirst->setProgressValue(100);
    QCOMPARE(Notificator::instances.size(), 1);

    QVERIFY(Notificator::instances.front() != nullptr);
    auto* pFinshedBallon = Notificator::instances.front().get();
    QVERIFY( pFinshedBallon != nullptr);
    QVERIFY(pFinshedBallon->m_title != nullptr);
    const QString titleOfBallon = pFinshedBallon->m_title->text();
    QVERIFY(titleOfBallon.contains("finished", Qt::CaseInsensitive));
  }
};

#include "NotificatorTest.moc"
NotificatorTest g_NotificatorTest;
