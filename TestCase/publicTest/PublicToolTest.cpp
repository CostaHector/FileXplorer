#include <QCoreApplication>
#include <QtTest>
#include "TestCase/pub/MyTestSuite.h"
#include "public/PublicTool.h"

class PublicToolTest : public MyTestSuite {
  Q_OBJECT
 public:
  PublicToolTest() : MyTestSuite{false} {}
 private slots:
  void test_mvToANewPath() {
    QString mvToANewPath = "5";
    QAction act1{"1", this};
    QAction act2{"2", this};
    QAction act3{"3", this};

    QActionGroup ag{this};
    ag.addAction(&act1);
    ag.addAction(&act2);
    ag.addAction(&act3);

    const QStringList& expectList{"5", "1", "2"};
    const QString& expectStr = expectList.join('\n');
    QCOMPARE(MoveToNewPathAutoUpdateActionText(mvToANewPath, &ag), expectStr);
  }

  void test_mvToANewPath_again() {
    QString mvToANewPath = "3";
    QAction act1{"1", this};
    QAction act2{"2", this};
    QAction act3{"3", this};
    QAction act4{"4", this};
    QAction act5{"5", this};

    QActionGroup ag{this};
    ag.addAction(&act1);
    ag.addAction(&act2);
    ag.addAction(&act3);
    ag.addAction(&act4);
    ag.addAction(&act5);

    const QStringList& expectList{"3", "1", "2", "4", "5"};
    const QString& expectStr = expectList.join('\n');
    QCOMPARE(MoveToNewPathAutoUpdateActionText(mvToANewPath, &ag), expectStr);
  }
};

#include "PublicToolTest.moc"
PublicToolTest g_PublicToolTest;
