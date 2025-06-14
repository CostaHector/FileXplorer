#include <QCoreApplication>
#include <QtTest>
#include "Tools/ToConsecutiveFileNameNo.h"
#include "TestCase/pub/MyTestSuite.h"

class ToConsecutiveFileNameNoTest : public MyTestSuite {
  Q_OBJECT
 public:
 private slots:
  void QRegularExpressionFileNameEndWithNoException() {
    QString newNameTemplate;

    QString fileNameScene = "movie name Scene 1.jpg";
    QVERIFY(not ToConsecutiveFileNameNo::nameNeedToProcess(fileNameScene, newNameTemplate));

    QString fileNameScDotSpaceSc = "movie name Sc. 1.jpg";
    QVERIFY(not ToConsecutiveFileNameNo::nameNeedToProcess(fileNameScDotSpaceSc, newNameTemplate));

    QString fileNameScDotNoSpace = "movie name Sc.1.jpg";
    QVERIFY(not ToConsecutiveFileNameNo::nameNeedToProcess(fileNameScDotNoSpace, newNameTemplate));

    QString fileNameScSpaceNoDot = "movie name Sc 1.jpg";
    QVERIFY(not ToConsecutiveFileNameNo::nameNeedToProcess(fileNameScSpaceNoDot, newNameTemplate));

    QString fileNameScNoDotNoSpace = "movie name Sc1.jpg";
    QVERIFY(not ToConsecutiveFileNameNo::nameNeedToProcess(fileNameScNoDotNoSpace, newNameTemplate));

    QString fileNameNotBeginingOfWord = "bankrupt 1.jpg";  // should not in exception list
    QVERIFY(ToConsecutiveFileNameNo::nameNeedToProcess(fileNameNotBeginingOfWord, newNameTemplate));
  }

  void QRegularExpressionEndWithTestTrue() {
    QString newNameTemplate;

    QString fileName = "A 1.jpg";
    QVERIFY(ToConsecutiveFileNameNo::nameNeedToProcess(fileName, newNameTemplate));
    QCOMPARE(newNameTemplate, "A 0.jpg");

    QString fileNameByHypen = "A - 1.jpg";
    QVERIFY(ToConsecutiveFileNameNo::nameNeedToProcess(fileNameByHypen, newNameTemplate));
    QCOMPARE(newNameTemplate, "A - 0.jpg");
  }

  void QRegularExpressionEndWithTestFalse() {
    QString unreliableTemplate;
    QString fileName = "A 1.pjson";
    QVERIFY(not ToConsecutiveFileNameNo::nameNeedToProcess(fileName, unreliableTemplate));
    QString fileNameByHypen = "A - 1.pjson";
    QVERIFY(not ToConsecutiveFileNameNo::nameNeedToProcess(fileNameByHypen, unreliableTemplate));
  }

  void renameA02ToA01() {
    ToConsecutiveFileNameNo tcno;
    const auto& ans = tcno({"A 0.jpg", "A 2.jpg"});
    const QStringList& expect{"A 0.jpg", "A 1.jpg"};
    QCOMPARE(ans, expect);
  }
};

#include "ToConsecutiveFileNameNoTest.moc"
ToConsecutiveFileNameNoTest g_ToConsecutiveFileNameNoTest;
