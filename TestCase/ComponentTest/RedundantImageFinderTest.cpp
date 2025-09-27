#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"
#include "TDir.h"
#include "MemoryKey.h"

#include "BeginToExposePrivateMember.h"
#include "ImagesInfoManager.h"
#include "RedundantImageFinder.h"
#include "EndToExposePrivateMember.h"
#include "RedundantImageFinderActions.h"

using namespace DuplicateImageMetaInfo;
const QStringList GetNames(const RedundantImagesList& imgs) {
  QStringList imgNames;
  for (const auto& img : imgs) {
    imgNames.append(img.m_AbsPath);
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
    const RedundantImagesList benchFolderDups = FindDuplicateImgs(mBenchmarkRedunFolder);
    QCOMPARE(GetNames(benchFolderDups),  //
             (QStringList{
                 mBenchmarkRedunFolder + "/a.jpg",           //
                 mBenchmarkRedunFolder + "/aDuplicate.png",  //
             }));

    const RedundantImagesList toFindFolderDups = FindDuplicateImgs(mFolderToFindRedun);
    QCOMPARE(GetNames(toFindFolderDups),  //
             (QStringList{
                 mFolderToFindRedun + "/cEmpty.webp",  //
             }));

    const RedundantImagesList workFolderDups = FindDuplicateImgs(mWorkPath);
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
    ImagesInfoManager& redunImgLib = ImagesInfoManager::getInst();
    redunImgLib.ResetStateForTestImpl(mBenchmarkRedunFolder);
    QCOMPARE(redunImgLib.count(), 2 + 1);
    // hash {hash1, hash1, hash2}, // size {3,3,3} = 2+1 = 3

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
    ImagesInfoManager& redunImgLib = ImagesInfoManager::getInst();
    redunImgLib.ResetStateForTestImpl(mBenchmarkRedunFolder);

    Configuration().clear();
    Configuration().setValue(RedunImgFinderKey::ALSO_RECYCLE_EMPTY_IMAGE.name, true);

    using namespace DuplicateImageDetectionCriteria;
    auto& redunInst = g_redunImgFinderAg();
    QCOMPARE(redunInst.GetCurFindDupBy(), DEFAULT_DI_CRITERIA_E);

    RedundantImageFinder rif;

    QCOMPARE(DEFAULT_DI_CRITERIA_E, DICriteriaE::LIBRARY);
    rif(mFolderToFindRedun);
    QCOMPARE(rif.m_imgModel->rowCount(), 3);  // all under mFolderToFindRedun

    rif(mBenchmarkRedunFolder);
    QCOMPARE(rif.m_imgModel->rowCount(), 3);  // all mBenchmarkRedunFolder itself
  }

  void find_redun_decide_by_md5_dup() {
    // clear libs
    ImagesInfoManager& redunImgLib = ImagesInfoManager::getInst();
    redunImgLib.ResetStateForTestImpl("");  // has nothing to do with a library

    Configuration().clear();  //
    QCOMPARE(RedunImgFinderKey::ALSO_RECYCLE_EMPTY_IMAGE.v.toBool(), true);
    auto& inst = g_redunImgFinderAg();
    QCOMPARE(inst.INCLUDING_EMPTY_IMAGES->isChecked(), true);

    using namespace DuplicateImageDetectionCriteria;
    auto& redunInst = g_redunImgFinderAg();
    redunInst.FIND_DUPLICATE_IMGS_IN_A_PATH->setChecked(true);
    emit redunInst.mDecideByIntAction.getActionGroup()->triggered(redunInst.FIND_DUPLICATE_IMGS_IN_A_PATH);

    QCOMPARE(redunInst.GetCurFindDupBy(), DICriteriaE::MD5);
    RedundantImageFinder rif;

    // 1.1 also empty image regard as redundant
    rif(mFolderToFindRedun);
    QCOMPARE(rif.m_imgModel->rowCount(), 1);  // only 1 empty file
    rif(mBenchmarkRedunFolder);
    QCOMPARE(rif.m_imgModel->rowCount(), 2);  // a.jpg, aDuplicate.png

    // 1.2 empty image not regard ...
    inst.INCLUDING_EMPTY_IMAGES->setChecked(false);
    emit inst.INCLUDING_EMPTY_IMAGES->toggled(false);
    rif(mFolderToFindRedun);
    QCOMPARE(rif.m_imgModel->rowCount(), 0);  // nothing
    rif(mBenchmarkRedunFolder);
    QCOMPARE(rif.m_imgModel->rowCount(), 2);  // a.jpg, aDuplicate.png

    // now lib is still empty, result will always be empty
    inst.FIND_DUPLICATE_IMGS_BY_LIBRARY->setChecked(true);
    emit inst.mDecideByIntAction.getActionGroup()->triggered(inst.FIND_DUPLICATE_IMGS_BY_LIBRARY);
    rif(mFolderToFindRedun);
    QCOMPARE(rif.m_imgModel->rowCount(), 0);  // nothing
    rif(mBenchmarkRedunFolder);
    QCOMPARE(rif.m_imgModel->rowCount(), 0);  // nothing
  }

  void last_test_current_file_recycle_all_ok() {
    ImagesInfoManager& redunImgLib = ImagesInfoManager::getInst();
    redunImgLib.ResetStateForTestImpl(mBenchmarkRedunFolder);  // has nothing to do with a library

    auto& inst = g_redunImgFinderAg();
    inst.INCLUDING_EMPTY_IMAGES->setChecked(true);
    QCOMPARE(inst.INCLUDING_EMPTY_IMAGES->isChecked(), true);

    RedundantImageFinder rif;
    rif(mBenchmarkRedunFolder);
    QCOMPARE(rif.m_imgModel->rowCount(), 3);  // all 3 under folder itself

    rif.m_table->selectAll();
    emit inst.RECYLE_NOW->triggered();

    QDir dir{mBenchmarkRedunFolder, "", QDir::SortFlag::NoSort, QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot};
    const QStringList items = dir.entryList();
    QVERIFY(items.isEmpty());

    emit inst.RELOAD_BENCHMARK_LIB->triggered();      // force reload from an no items path
    QCOMPARE(redunImgLib.ImgDataStruct().size(), 0);  // 0+0

    Configuration().setValue(RedunImgFinderKey::RUND_IMG_PATH.name, mFolderToFindRedun);
    emit inst.RELOAD_BENCHMARK_LIB->triggered();      // force reload from mFolderToFindRedun path
    QCOMPARE(redunImgLib.ImgDataStruct().size(), 3 + 2);  // 3 different hash + 2 different size

    rif.close();
  }
};

#include "RedundantImageFinderTest.moc"
REGISTER_TEST(RedundantImageFinderTest, false)
