#include <QCoreApplication>
#include <QtTest>
#include "pub/MyTestSuite.h"
#include "Tools/FolderNxtAndLastIterator.h"

QStringList Invoke_GetSortedDirNames(const QString& parentPath) {
  if (parentPath == "0 Folder Include") {
    return {};
  } else if (parentPath == "1 Folder Include") {
    return {"folder 0"};
  } else if (parentPath == "5 Folder Include") {
    return {"folder 0", "folder 1", "folder 2", "folder 3", "folder 4"};
  } else {
    return {};
  }
}

class FolderNxtAndLastIteratorTest : public MyTestSuite {
  Q_OBJECT
 public:
 private slots:
  void test_emptyFolderReturnEmpty_ok() {
    FolderNxtAndLastIterator nlIt{Invoke_GetSortedDirNames};
    QCOMPARE(nlIt.next("0 Folder Include", "any nonexist folder"), "");
    QCOMPARE(nlIt.last("0 Folder Include", "any nonexist folder"), "");
  }

  void test_1ElementFolderReturnItself_ok() {
    FolderNxtAndLastIterator nlIt{Invoke_GetSortedDirNames};
    QCOMPARE(nlIt.next("1 Folder Include", "folder 0"), "folder 0");
    QCOMPARE(nlIt.last("1 Folder Include", "folder 0"), "folder 0");
  }

  void test_conventionNxt_ok() {
    FolderNxtAndLastIterator nlIt{Invoke_GetSortedDirNames};
    QCOMPARE(nlIt.next("5 Folder Include", "folder 4"), "folder 0");
    QCOMPARE(nlIt.next("5 Folder Include", "folder 1"), "folder 2");
  }

  void test_conventionLast_ok() {
    FolderNxtAndLastIterator nlIt{Invoke_GetSortedDirNames};
    QCOMPARE(nlIt.last("5 Folder Include", "folder 0"), "folder 4");
    QCOMPARE(nlIt.last("5 Folder Include", "folder 2"), "folder 1");
  }

  void test_parentPathLvlsFolder_noNeedUpdate() {
    FolderNxtAndLastIterator nlIt{Invoke_GetSortedDirNames};
    QVERIFY(nlIt("5 Folder Include"));
    QVERIFY(!nlIt("5 Folder Include"));
  }

  void test_parentPathLvlsFolder_needUpdate() {
    FolderNxtAndLastIterator nlIt{Invoke_GetSortedDirNames};
    QVERIFY(nlIt("5 Folder Include"));
    QVERIFY(nlIt("1 Folder Include"));
  }
};

FolderNxtAndLastIteratorTest g_FolderNxtAndLastIteratorTest;
#include "FolderNxtAndLastIteratorTest.moc"
