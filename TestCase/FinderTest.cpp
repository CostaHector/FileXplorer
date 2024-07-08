#include <QCoreApplication>
#include <QtTest>

// add necessary includes here
#include "Tools/ConflictsItemHelper.h"
#include "pub/FileSystemRelatedTest.h"

#include <QSet>

class FinderTest : public FileSystemRelatedTest {
  Q_OBJECT
 public:
  FinderTest() : FileSystemRelatedTest{"TestEnv_Finder", "BasicItemFind"} {}

 private slots:
  void test_GetAllItemsWhenMoveItems() {
    QVERIFY2(QDir(TEST_DIR).exists("folder/folder/file.txt"), "Precondition not met.");
    QVERIFY2(QDir(TEST_DIR).exists("folder/file.txt"), "Precondition not met.");
    QVERIFY2(QDir(TEST_DIR).exists("file.txt"), "Precondition not met.");

    const QStringList fromList{"folder", "file.txt"};
    const auto& actualList = Finder(CCMMode::CUT_OP).FindAllItems(TEST_DIR, fromList);
    const QStringList expectList{"folder/folder/file.txt", "folder/folder", "folder/file.txt", "file.txt", "folder"};

    QSet<QString> actualSet{actualList.cbegin(), actualList.cend()};
    QSet<QString> expectSet{expectList.cbegin(), expectList.cend()};
    QCOMPARE(actualSet, expectSet);
  }
  void test_GetAllItemsWhenCopyItems() {
    QVERIFY2(QDir(TEST_DIR).exists("folder/folder/file.txt"), "Precondition not met.");
    QVERIFY2(QDir(TEST_DIR).exists("folder/file.txt"), "Precondition not met.");
    QVERIFY2(QDir(TEST_DIR).exists("file.txt"), "Precondition not met.");

    const QStringList fromList{"folder", "file.txt"};
    const auto& actualList = Finder(CCMMode::COPY_OP).FindAllItems(TEST_DIR, fromList);
    const QStringList expectList{"folder/folder/file.txt", "folder/folder", "folder/file.txt", "file.txt", "folder"};
    QSet<QString> actualSet{actualList.cbegin(), actualList.cend()};
    QSet<QString> expectSet{expectList.cbegin(), expectList.cend()};
    QCOMPARE(actualSet, expectSet);
  }
};

//QTEST_MAIN(FinderTest)
//#include "FinderTest.moc"
