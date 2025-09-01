#include <QCoreApplication>
#include <QtTest>
#include "MyTestSuite.h"
#include "TDir.h"

#include "BeginToExposePrivateMember.h"
#include "RedunImgLibs.h"
#include "EndToExposePrivateMember.h"

const auto GetNames = [](const REDUNDANT_IMG_BUNCH& imgs) -> QStringList {
  QStringList imgNames;
  for (const auto& img : imgs) {
    imgNames.append(img.filePath);
  }
  imgNames.sort();
  return imgNames;
};

class RedundantImageFinderTest : public MyTestSuite {
  Q_OBJECT
public:
  RedundantImageFinderTest() : MyTestSuite{true} {}
  TDir mDir;
  const QString mWorkPath{mDir.path()};
  const QString mBenchmarkRedunFolder{mWorkPath + "/benchmark"};
  const QString mFolderToFindRedun{mWorkPath + "/ToFindRedun"};
private slots:
  void initTestCase() {
    QVERIFY(mDir.IsValid());
    const QList<FsNodeEntry> gNode{
        FsNodeEntry{"benchmark/a.jpg", false, "123"},           //
        FsNodeEntry{"benchmark/aDuplicate.png", false, "123"},  //
        FsNodeEntry{"benchmark/b.png", false, "456"},           //
        FsNodeEntry{"ToFindRedun/aRedun.jpg", false, "123"},    //
        FsNodeEntry{"ToFindRedun/bRedun.png", false, "456"},    //
        FsNodeEntry{"ToFindRedun/cEmpty.webp", false, ""},      //
    };
    QVERIFY(mDir.createEntries(gNode) >= gNode.size());
  }

  void test_duplicate_images_find_ok() {
    const REDUNDANT_IMG_BUNCH benchFolderDups = RedunImgLibs::FindDuplicateImgs(mBenchmarkRedunFolder);
    QCOMPARE(GetNames(benchFolderDups),  //
             (QStringList{
                 mBenchmarkRedunFolder + "/a.jpg",           //
                 mBenchmarkRedunFolder + "/aDuplicate.png",  //
             }));

    const REDUNDANT_IMG_BUNCH toFindFolderDups = RedunImgLibs::FindDuplicateImgs(mFolderToFindRedun);
    QCOMPARE(GetNames(toFindFolderDups),  //
             (QStringList{
                 mFolderToFindRedun + "/cEmpty.webp",  //
             }));

    const REDUNDANT_IMG_BUNCH workFolderDups = RedunImgLibs::FindDuplicateImgs(mWorkPath);
    QCOMPARE(GetNames(workFolderDups),  //
             (QStringList{
                 mFolderToFindRedun + "/aRedun.jpg",         //
                 mFolderToFindRedun + "/bRedun.png",         //
                 mFolderToFindRedun + "/cEmpty.webp",        //
                 mBenchmarkRedunFolder + "/a.jpg",           //
                 mBenchmarkRedunFolder + "/aDuplicate.png",  //
                 mBenchmarkRedunFolder + "/b.png",           //
             }));
  }

  void test_redundant_images_in_library_find_ok() {
    // procedure
    RedunImgLibs& redunImgLib = RedunImgLibs::GetInst(mBenchmarkRedunFolder);
    QCOMPARE(redunImgLib.m_commonFileHash.size(), 2);                                  // hash {hash1, hash1, hash2}
    QCOMPARE(redunImgLib.m_commonFileSizeSet.size(), 1);                               // size {3,3,3}

    const auto& itemsEmptyAlsoRedun = redunImgLib.FindRedunImgs(mFolderToFindRedun, true);
    QCOMPARE(GetNames(itemsEmptyAlsoRedun),  //
             (QStringList{
                 mFolderToFindRedun + "/aRedun.jpg",  //
                 mFolderToFindRedun + "/bRedun.png",  //
                 mFolderToFindRedun + "/cEmpty.webp",
             })  //
             );

    const auto& itemsEmptyNotRedun = redunImgLib.FindRedunImgs(mFolderToFindRedun, false);
    QCOMPARE(GetNames(itemsEmptyNotRedun),  //
             (QStringList{
                 mFolderToFindRedun + "/aRedun.jpg",  //
                 mFolderToFindRedun + "/bRedun.png",  //
             })                                       //
             );                                                //
  }
};

RedundantImageFinderTest g_RedundantImageFinderTest;
#include "RedundantImageFinderTest.moc"
