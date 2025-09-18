#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"
#include "TDir.h"
#include "MemoryKey.h"

#include "BeginToExposePrivateMember.h"
#include "RedunImgLibs.h"
#include "RedundantImageFinder.h"
#include "EndToExposePrivateMember.h"
#include "RedundantImageFinderActions.h"

const QStringList GetNames(const REDUNDANT_IMG_BUNCH& imgs) {
  QStringList imgNames;
  for (const auto& img : imgs) {
    imgNames.append(img.filePath);
  }
  imgNames.sort();
  return imgNames;
}

class RedundantImageFinderTest : public PlainTestSuite {
  Q_OBJECT
 public:
  RedundantImageFinderTest() : PlainTestSuite{} {}
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

  void test_md5_duplicate_images_find_ok() {
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
    const RedunImgLibs& redunImgLib = RedunImgLibs::GetInst(mBenchmarkRedunFolder);
    QCOMPARE(redunImgLib.m_commonFileHash.size(), 2);     // hash {hash1, hash1, hash2}
    QCOMPARE(redunImgLib.m_commonFileSizeSet.size(), 1);  // size {3,3,3}

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

  // the above testcase set library to mBenchmarkRedunFolder already
  void find_redun_decide_by_Lib() {
    const RedunImgLibs& redunImgLib = RedunImgLibs::GetInst(mBenchmarkRedunFolder);

    Configuration().clear();
    Configuration().setValue(RedunImgFinderKey::ALSO_RECYCLE_EMPTY_IMAGE.name, true);

    using namespace RedundantImageTool;
    auto& redunInst = g_redunImgFinderAg();
    QCOMPARE(redunInst.GetCurFindDupBy(), DEFAULT_DECIDE_BY);

    RedundantImageFinder rif;

    QCOMPARE(DEFAULT_DECIDE_BY, DecideByE::LIBRARY);
    rif(mFolderToFindRedun);
    QCOMPARE(rif.m_imgModel->rowCount(), 3);  // all under mFolderToFindRedun

    rif(mBenchmarkRedunFolder);
    QCOMPARE(rif.m_imgModel->rowCount(), 3);  // all mBenchmarkRedunFolder itself
  }

  void find_redun_decide_by_md5_dup() {
    // clear libs
    RedunImgLibs::ResetForInitStateForTest();
    RedunImgLibs::GetInst("");  // has nothing to do with a library

    Configuration().clear(); //
    QCOMPARE(RedunImgFinderKey::ALSO_RECYCLE_EMPTY_IMAGE.v.toBool(), true);
    auto& inst = g_redunImgFinderAg();
    QCOMPARE(inst.ALSO_EMPTY_IMAGE->isChecked(), true);

    using namespace RedundantImageTool;
    auto& redunInst = g_redunImgFinderAg();
    redunInst.FIND_DUPLICATE_IMGS_IN_A_PATH->setChecked(true);
    emit redunInst.mDecideByIntAction.getActionGroup()->triggered(redunInst.FIND_DUPLICATE_IMGS_IN_A_PATH);

    QCOMPARE(redunInst.GetCurFindDupBy(), DecideByE::MD5);
    RedundantImageFinder rif;

    // 1.1 also empty image regard as redundant
    rif(mFolderToFindRedun);
    QCOMPARE(rif.m_imgModel->rowCount(), 1); // only 1 empty file
    rif(mBenchmarkRedunFolder);
    QCOMPARE(rif.m_imgModel->rowCount(), 2); // a.jpg, aDuplicate.png

    // 1.2 empty image not regard ...
    inst.ALSO_EMPTY_IMAGE->setChecked(false);
    emit inst.ALSO_EMPTY_IMAGE->toggled(false);
    rif(mFolderToFindRedun);
    QCOMPARE(rif.m_imgModel->rowCount(), 0); // nothing
    rif(mBenchmarkRedunFolder);
    QCOMPARE(rif.m_imgModel->rowCount(), 2); // a.jpg, aDuplicate.png

    // now lib is still empty, result will always be empty
    inst.FIND_DUPLICATE_IMGS_BY_LIBRARY->setChecked(true);
    emit inst.mDecideByIntAction.getActionGroup()->triggered(inst.FIND_DUPLICATE_IMGS_BY_LIBRARY);
    rif(mFolderToFindRedun);
    QCOMPARE(rif.m_imgModel->rowCount(), 0); // nothing
    rif(mBenchmarkRedunFolder);
    QCOMPARE(rif.m_imgModel->rowCount(), 0); // nothing
  }

  void last_test_current_file_recycle_all_ok() {
    RedunImgLibs::ResetForInitStateForTest();
    RedunImgLibs::GetInst(mBenchmarkRedunFolder);

    auto& inst = g_redunImgFinderAg();
    inst.ALSO_EMPTY_IMAGE->setChecked(true);
    QCOMPARE(inst.ALSO_EMPTY_IMAGE->isChecked(), true);

    RedundantImageFinder rif;
    rif(mBenchmarkRedunFolder);
    QCOMPARE(rif.m_imgModel->rowCount(), 3); // all 3 under folder itself

    rif.m_table->selectAll();
    emit inst.RECYLE_NOW->triggered();

    QDir dir{mBenchmarkRedunFolder, "", QDir::SortFlag::NoSort, QDir::Filter::Files | QDir::Filter::Dirs| QDir::Filter::NoDotAndDotDot};
    const QStringList items = dir.entryList();
    QVERIFY(items.isEmpty());

    rif.close();
  }
};

#include "RedundantImageFinderTest.moc"
REGISTER_TEST(RedundantImageFinderTest, false)
