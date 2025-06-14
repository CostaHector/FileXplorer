#include <QCoreApplication>
#include <QtTest>
#include "TestCase/pub/MyTestSuite.h"
#include "TestCase/pub/TDir.h"
#include "Tools/RedunImgLibs.h"

class RedundantImageFinderTest : public MyTestSuite {
  Q_OBJECT
 public:
  RedundantImageFinderTest() : MyTestSuite{false} {}
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

  void test_redundant_images_find_ok() {
    // procedure
    RedunImgLibs redunImgLib;
    QCOMPARE(redunImgLib.LearnSizeAndHashFromRedunImgPath(mBenchmarkRedunFolder), 3);  // 3 files
    QCOMPARE(redunImgLib.m_commonFileHash.size(), 2);                                  // hash {hash1, hash1, hash2}
    QCOMPARE(redunImgLib.m_commonFileSizeSet.size(), 1);                               // size {3,3,3}

    static const auto GetNames = [](const REDUNDANT_IMG_BUNCH& imgs) -> QStringList {
      QStringList imgNames;
      for (const auto& img : imgs) {
        imgNames.append(img.filePath);
      }
      imgNames.sort();
      return imgNames;
    };

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
