#include <QCoreApplication>
#include <QtTest>

// add necessary includes here
#include "Tools/ConflictsItemHelper.h"
#include "pub/FileSystemTestSuite.h"

#include <QSet>

class FinderTest : public FileSystemTestSuite {
  Q_OBJECT
 public:
  FinderTest() : FileSystemTestSuite{"TestEnv_Finder", false, false} {}

 private slots:
  void initTestCase() {
    // folder{folder{file.txt}, file.txt}, file.txt
    m_rootHelper << FileSystemNode{"folder"} << FileSystemNode{"file.txt", false, ""};
    m_rootHelper.GetSubHelper("folder") << FileSystemNode{"folder"} << FileSystemNode{"file.txt", false, ""};
    m_rootHelper.GetSubHelper("folder").GetSubHelper("folder") << FileSystemNode{"file.txt", false, ""};
  }
  void cleanupTestCase() {  //
    QVERIFY(m_rootHelper.EraseFileSystemTree(false));
  }

  void test_GetAllItemsWhenMoveItems() {
    QVERIFY2(QDir(mTestPath).exists("folder/folder/file.txt"), "Precondition not met.");
    QVERIFY2(QDir(mTestPath).exists("folder/file.txt"), "Precondition not met.");
    QVERIFY2(QDir(mTestPath).exists("file.txt"), "Precondition not met.");

    const QStringList fromList{"folder", "file.txt"};
    const auto& actualList = Finder(CCMMode::CUT_OP).FindAllItems(mTestPath, fromList);
    const QStringList expectList{"folder/folder/file.txt", "folder/folder", "folder/file.txt", "file.txt", "folder"};

    QSet<QString> actualSet{actualList.cbegin(), actualList.cend()};
    QSet<QString> expectSet{expectList.cbegin(), expectList.cend()};
    QCOMPARE(actualSet, expectSet);
  }

  void test_GetAllItemsWhenCopyItems() {
    QVERIFY2(QDir(mTestPath).exists("folder/folder/file.txt"), "Precondition not met.");
    QVERIFY2(QDir(mTestPath).exists("folder/file.txt"), "Precondition not met.");
    QVERIFY2(QDir(mTestPath).exists("file.txt"), "Precondition not met.");

    const QStringList fromList{"folder", "file.txt"};
    const auto& actualList = Finder(CCMMode::COPY_OP).FindAllItems(mTestPath, fromList);
    const QStringList expectList{"folder/folder/file.txt", "folder/folder", "folder/file.txt", "file.txt", "folder"};
    QSet<QString> actualSet{actualList.cbegin(), actualList.cend()};
    QSet<QString> expectSet{expectList.cbegin(), expectList.cend()};
    QCOMPARE(actualSet, expectSet);
  }
};

#include "FinderTest.moc"
FinderTest g_FinderTest;
