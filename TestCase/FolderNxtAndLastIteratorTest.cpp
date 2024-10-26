#include <QCoreApplication>
#include <QtTest>

#include "pub/FileSystemRelatedTest.h"
#include "Tools/FolderNxtAndLastIterator.h"
#include <QDir>

class FolderNxtAndLastIteratorTest : public FileSystemRelatedTest {
  Q_OBJECT
 public:
  FolderNxtAndLastIteratorTest() : FileSystemRelatedTest{"TestEnv_FolderNxtAndLastIteratorTest"} {}
 private slots:
  void initTestCase() {}
  void cleanupTestCase() {}
  void init() {
    m_rootHelper << FileSystemNode{"0 Folder Include"} << FileSystemNode{"1 Folder Include"} << FileSystemNode{"5 Folder Include"};
    m_rootHelper.GetSubHelper("1 Folder Include") << FileSystemNode{"folder 0"};
    m_rootHelper.GetSubHelper("5 Folder Include")
        << FileSystemNode{"folder 0"}
        << FileSystemNode{"folder 1"}
        << FileSystemNode{"folder 2"}
        << FileSystemNode{"folder 3"}
        << FileSystemNode{"folder 4"};
  }

  void test_emptyFolderReturnEmpty_ok() {
    FolderNxtAndLastIterator nlIt;
    QCOMPARE(nlIt.next(ROOT_DIR+"/0 Folder Include", "any nonexist folder"), "");
    QCOMPARE(nlIt.last(ROOT_DIR+"/0 Folder Include", "any nonexist folder"), "");
  }

  void test_1ElementFolderReturnItself_ok() {
    FolderNxtAndLastIterator nlIt;
    QCOMPARE(nlIt.next(ROOT_DIR+"/1 Folder Include", "folder 0"), "folder 0");
    QCOMPARE(nlIt.last(ROOT_DIR+"/1 Folder Include", "folder 0"), "folder 0");
  }

  void test_conventionNxt_ok() {
    FolderNxtAndLastIterator nlIt;
    QCOMPARE(nlIt.next(ROOT_DIR+"/5 Folder Include", "folder 4"), "folder 0");
    QCOMPARE(nlIt.next(ROOT_DIR+"/5 Folder Include", "folder 1"), "folder 2");
  }

  void test_conventionLast_ok() {
    FolderNxtAndLastIterator nlIt;
    QCOMPARE(nlIt.last(ROOT_DIR+"/5 Folder Include", "folder 0"), "folder 4");
    QCOMPARE(nlIt.last(ROOT_DIR+"/5 Folder Include", "folder 2"), "folder 1");
  }
};

//QTEST_MAIN(FolderNxtAndLastIteratorTest)
#include "FolderNxtAndLastIteratorTest.moc"
