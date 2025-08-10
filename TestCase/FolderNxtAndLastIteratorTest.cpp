#include <QCoreApplication>
#include <QtTest>
#include "TestCase/pubTestTool/MyTestSuite.h"
#include "Tools/FolderNxtAndLastIterator.h"

class FolderNxtAndLastIteratorTest : public MyTestSuite {
  Q_OBJECT
 public:
 private slots:
  void test_emptyFolderReturnEmpty_ok() {
    FolderNxtAndLastIterator nlIt;
    QStringList emptyLst;
    QVERIFY(nlIt("0 Folder Include", emptyLst));
    QCOMPARE(nlIt.next("0 Folder Include", "any nonexist folder"), "");
    QCOMPARE(nlIt.last("0 Folder Include", "any nonexist folder"), "");
  }

  void test_1ElementFolderReturnItself_ok() {
    FolderNxtAndLastIterator nlIt;
    QStringList pathLst{"folder 0"};
    QVERIFY(nlIt("1 Folder Include", pathLst));
    QCOMPARE(nlIt.next("1 Folder Include", "folder 0"), "folder 0");
    QCOMPARE(nlIt.last("1 Folder Include", "folder 0"), "folder 0");
  }

  void test_conventionNxt_ok() {
    FolderNxtAndLastIterator nlIt;
    QStringList pathLst{"folder 0", "folder 1", "folder 2", "folder 3", "folder 4"};
    QVERIFY(nlIt("5 Folder Include", pathLst));
    QCOMPARE(nlIt.next("5 Folder Include", "folder 4"), "folder 0");
    QCOMPARE(nlIt.next("5 Folder Include", "folder 1"), "folder 2");
  }

  void test_conventionLast_ok() {
    FolderNxtAndLastIterator nlIt;
    QStringList pathLst{"folder 0", "folder 1", "folder 2", "folder 3", "folder 4"};
    QVERIFY(nlIt("5 Folder Include", pathLst));
    QCOMPARE(nlIt.last("5 Folder Include", "folder 0"), "folder 4");
    QCOMPARE(nlIt.last("5 Folder Include", "folder 2"), "folder 1");
  }

  void test_parentPathLvlsFolder_noNeedUpdate() {
    FolderNxtAndLastIterator nlIt;
    QVERIFY(nlIt("5 Folder Include"));
    QVERIFY(!nlIt("5 Folder Include"));
  }

  void test_parentPathLvlsFolder_needUpdate() {
    FolderNxtAndLastIterator nlIt;
    QVERIFY(nlIt("5 Folder Include"));
    QVERIFY(nlIt("1 Folder Include"));
  }
};

FolderNxtAndLastIteratorTest g_FolderNxtAndLastIteratorTest;
#include "FolderNxtAndLastIteratorTest.moc"
