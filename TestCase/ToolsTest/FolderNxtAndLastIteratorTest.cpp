#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "FolderNxtAndLastIterator.h"
#include "EndToExposePrivateMember.h"

#include "PublicVariable.h"
#include "TDir.h"

class FolderNxtAndLastIteratorTest : public PlainTestSuite {
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

  void images_navigator() {
    TDir tDir;
    QVERIFY(tDir.IsValid());
    QList<FsNodeEntry> nodes{
        {"a/img 1.jpg", false, ""},   //
        {"a/img 0.jpg", false, ""},   //
        {"0/img 2.gif", false, ""},   //
        {"0/img 3.mp4", false, ""},   //
        {"b/img 4.webp", false, ""},  //
        {"img 6.png", false, ""},     //
        {"img 5.png", false, ""},     //
    };
    QCOMPARE(tDir.createEntries(nodes), nodes.size());

    const QStringList expectsFolders{
        "0",  //
        "a",  //
        "b",  //
    };

    const QStringList expectsImagesIncludingSub{
        "0/img 2.gif",   //
        "a/img 0.jpg",   //
        "a/img 1.jpg",   //
        "b/img 4.webp",  //
        "img 5.png",     //
        "img 6.png",     //
    };
    const QStringList expectsImagesNotIncludingSub{
        "img 5.png",  //
        "img 6.png",  //
    };

    const QString pth = tDir.path();
    {  // folders navigate
      FolderNxtAndLastIterator folderIt{FolderNxtAndLastIterator::GetInstsNaviFolders()};
      folderIt(pth);
      QCOMPARE(folderIt.sameLevelPaths, expectsFolders);

      QCOMPARE(folderIt.next(pth, "0"), "a");
      QCOMPARE(folderIt.next(pth, "a"), "b");
      QCOMPARE(folderIt.next(pth, "b"), "0");

      QCOMPARE(folderIt.last(pth, "0"), "b");
      QCOMPARE(folderIt.last(pth, "a"), "0");
      QCOMPARE(folderIt.last(pth, "b"), "a");
    }

    {  // images navigate including directory
      FolderNxtAndLastIterator imgIt{FolderNxtAndLastIterator::GetInstsNaviImages(true)};
      QVERIFY(imgIt(tDir.path()));
      QCOMPARE(imgIt.sameLevelPaths, expectsImagesIncludingSub);

      QCOMPARE(imgIt.next(pth, "0/img 2.gif"), "a/img 0.jpg");
      QCOMPARE(imgIt.next(pth, "a/img 0.jpg"), "a/img 1.jpg");
      QCOMPARE(imgIt.next(pth, "b/img 4.webp"), "img 5.png");
      QCOMPARE(imgIt.next(pth, "img 6.png"), "0/img 2.gif");

      QCOMPARE(imgIt.last(pth, "a/img 0.jpg"), "0/img 2.gif");
      QCOMPARE(imgIt.last(pth, "a/img 1.jpg"), "a/img 0.jpg");
      QCOMPARE(imgIt.last(pth, "img 5.png"), "b/img 4.webp");
      QCOMPARE(imgIt.last(pth, "0/img 2.gif"), "img 6.png");

      imgIt.setIncludingSubDirectory(false);
      QVERIFY(!imgIt(tDir.path()));  // not force
      QCOMPARE(imgIt.sameLevelPaths, expectsImagesIncludingSub);
      QVERIFY(imgIt(tDir.path(), true));  // force
      QCOMPARE(imgIt.sameLevelPaths, expectsImagesNotIncludingSub);
    }

    {  // images navigate not including directory
      FolderNxtAndLastIterator imgIt{FolderNxtAndLastIterator::GetInstsNaviImages(false)};
      imgIt(tDir.path());
      QCOMPARE(imgIt.sameLevelPaths, expectsImagesNotIncludingSub);

      QCOMPARE(imgIt.last(pth, "img 5.png"), "img 6.png");
      QCOMPARE(imgIt.next(pth, "img 5.png"), "img 6.png");
      QCOMPARE(imgIt.next(pth, "img 6.png"), "img 5.png");

      imgIt.setIncludingSubDirectory(true);
      QVERIFY(!imgIt(tDir.path()));  // not force
      QCOMPARE(imgIt.sameLevelPaths, expectsImagesNotIncludingSub);

      QVERIFY(imgIt(tDir.path(), true));  // force
      QCOMPARE(imgIt.sameLevelPaths, expectsImagesIncludingSub);
    }
  }
};

#include "FolderNxtAndLastIteratorTest.moc"
REGISTER_TEST(FolderNxtAndLastIteratorTest, false)
