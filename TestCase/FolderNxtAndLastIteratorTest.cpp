#include <QCoreApplication>
#include <QtTest>

// #include "pub/BeginToExposePrivateMember.h"
#include "Tools/FolderNxtAndLastIterator.h"
// #include "pub/EndToExposePrivateMember.h"
#include <QDir>

QDir NEXT_LAST_PATH{QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_FolderNxtAndLastIteratorTest")};

class FolderNxtAndLastIteratorTest : public QObject {
  Q_OBJECT
 public:
  QDir m0FolderDir{NEXT_LAST_PATH.absoluteFilePath("0 Folder Include"), "", QDir::SortFlag::DirsFirst,
                   QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot};
  QDir m1FolderDir{NEXT_LAST_PATH.absoluteFilePath("1 Folder Include"), "", QDir::SortFlag::DirsFirst,
                   QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot};
  QDir m5FoldersDir{NEXT_LAST_PATH.absoluteFilePath("5 Folders Include"), "", QDir::SortFlag::DirsFirst,
                    QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot};
 private slots:
  void initTestCase() {
    QStringList folder0;
    QCOMPARE(m0FolderDir.entryList(), folder0);
    QStringList folder1{"folder 0"};
    QCOMPARE(m1FolderDir.entryList(), folder1);
    QStringList folders5{"folder 0", "folder 1", "folder 2", "folder 3", "folder 4"};
    QCOMPARE(m5FoldersDir.entryList(), folders5);
  }
  void cleanupTestCase() {}

  void test_emptyFolderReturnEmpty_ok() {
    FolderNxtAndLastIterator nlIt;
    QCOMPARE(nlIt.next(m0FolderDir.absolutePath(), "any folder"), "");
    QCOMPARE(nlIt.last(m0FolderDir.absolutePath(), "any folder"), "");
  }

  void test_1ElementFolderReturnItself_ok() {
    FolderNxtAndLastIterator nlIt;
    QCOMPARE(nlIt.next(m1FolderDir.absolutePath(), "folder 0"), "folder 0");
    QCOMPARE(nlIt.last(m1FolderDir.absolutePath(), "folder 0"), "folder 0");
  }

  void test_conventionNxt_ok() {
    FolderNxtAndLastIterator nlIt;
    QCOMPARE(nlIt.next(m5FoldersDir.absolutePath(), "folder 4"), "folder 0");
    QCOMPARE(nlIt.next(m5FoldersDir.absolutePath(), "folder 1"), "folder 2");
  }

  void test_conventionLast_ok() {
    FolderNxtAndLastIterator nlIt;
    QCOMPARE(nlIt.last(m5FoldersDir.absolutePath(), "folder 0"), "folder 4");
    QCOMPARE(nlIt.last(m5FoldersDir.absolutePath(), "folder 2"), "folder 1");
  }
};

QTEST_MAIN(FolderNxtAndLastIteratorTest)
#include "FolderNxtAndLastIteratorTest.moc"
