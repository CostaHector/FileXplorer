#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"
#include "TDir.h"
#include "RedunImgFinderKey.h"
#include "Configuration.h"

#include "BeginToExposePrivateMember.h"
#include "ImagesInfoManager.h"
#include "RedundantImageFinder.h"
#include "EndToExposePrivateMember.h"
#include "RedundantImageFinderActions.h"
#include "FileTool.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

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

  void init() { GlobalMockObject::reset(); }
  void cleanup() { GlobalMockObject::verify(); }

  void default_ok() {
    RedundantImageFinder rif;
    rif.showEvent(nullptr);
    QShowEvent defaultShowEvent;
    rif.showEvent(&defaultShowEvent);

    QVERIFY(rif.m_table != nullptr);

    const QString openMarkPath{ImagesInfoManager::GetDynRedunPath()};
    QCOMPARE(QFile::exists(openMarkPath), true);

    MOCKER(FileTool::OpenLocalFileUsingDesktopService).expects(exactly(2)).with(eq(openMarkPath)).will(returnValue(true));
    auto& inst = g_redunImgFinderAg();
    inst.OPEN_BENCHMARK_FOLDER->trigger();
    QCOMPARE(rif.onOpenBenchmarkFolder(), true);

    MOCKER(FileTool::OpenLocalImageFile).expects(never()).will(ignoreReturnValue());
    emit rif.m_table->doubleClicked({});
    QCOMPARE(rif.onOpenImageDoubleClicked({}), false);
  }

  void md5_duplicate_images_find_ok() {
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

  void redundant_images_in_library_find_ok() {
    // procedure
    ImagesInfoManager& redunImgLib = ImagesInfoManager::getInst();
    redunImgLib.InitializeImpl(mBenchmarkRedunFolder);
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
    redunImgLib.InitializeImpl(mBenchmarkRedunFolder);

    Configuration().clear();
    setConfig(RedunImgFinderKey::ALSO_RECYCLE_EMPTY_IMAGE, true);

    using namespace DuplicateImageDetectionCriteria;
    auto& redunInst = g_redunImgFinderAg();
    QCOMPARE(redunInst.GetCurFindDupBy(), DEFAULT_DI_CRITERIA_E);

    RedundantImageFinder rif;
    rif.m_table->sortByColumn(0, Qt::SortOrder::AscendingOrder);

    QCOMPARE(DEFAULT_DI_CRITERIA_E, DICriteriaE::LIBRARY);
    QCOMPARE(rif(""), false);  // empty path
    QCOMPARE(rif(mFolderToFindRedun), true);
    QCOMPARE(rif.m_imgModel->rowCount(), 3);  // all under mFolderToFindRedun

    QVERIFY(rif(mBenchmarkRedunFolder));
    QCOMPARE(rif.m_imgModel->rowCount(), 3);  // all mBenchmarkRedunFolder itself

    QVERIFY(rif.m_imgProxy != nullptr);
    const QModelIndex proInd = rif.m_imgProxy->index(0, 0);
    QCOMPARE(proInd.data(Qt::DisplayRole).toString(), "a.jpg");
    const QString imgPath{mDir.itemPath("benchmark/a.jpg")};
    MOCKER(FileTool::OpenLocalImageFile).expects(exactly(2)).with(eq(imgPath)).will(returnValue(true));
    emit rif.m_table->doubleClicked(proInd);
    QCOMPARE(rif.onOpenImageDoubleClicked(proInd), true);
  }

  void find_redun_decide_by_md5_dup() {
    // clear libs
    ImagesInfoManager& redunImgLib = ImagesInfoManager::getInst();
    redunImgLib.InitializeImpl("");  // has nothing to do with a library

    Configuration().clear();  //
    QCOMPARE(RedunImgFinderKey::ALSO_RECYCLE_EMPTY_IMAGE.v.data.b, true);
    auto& inst = g_redunImgFinderAg();
    QCOMPARE(inst.INCLUDING_EMPTY_IMAGES->isChecked(), true);

    using namespace DuplicateImageDetectionCriteria;
    auto& redunInst = g_redunImgFinderAg();
    redunInst.FIND_DUPLICATE_IMGS_IN_A_PATH->setChecked(true);
    emit redunInst.mDecideByIntAction.getActionGroup()->triggered(redunInst.FIND_DUPLICATE_IMGS_IN_A_PATH);

    QCOMPARE(redunInst.GetCurFindDupBy(), DICriteriaE::MD5);
    RedundantImageFinder rif;

    // 1.1 also empty image regard as redundant
    QVERIFY(rif(mFolderToFindRedun));
    QCOMPARE(rif.m_imgModel->rowCount(), 1);  // only 1 empty file
    QVERIFY(rif(mBenchmarkRedunFolder));
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
    redunImgLib.InitializeImpl(mBenchmarkRedunFolder);  // has nothing to do with a library

    auto& inst = g_redunImgFinderAg();
    inst.INCLUDING_EMPTY_IMAGES->setChecked(true);
    QCOMPARE(inst.INCLUDING_EMPTY_IMAGES->isChecked(), true);

    RedundantImageFinder rif;
    QVERIFY(rif(mBenchmarkRedunFolder));
    QCOMPARE(rif.m_imgModel->rowCount(), 3);  // all 3 under folder itself

    rif.m_table->clearSelection();
    emit inst.RECYLE_NOW->triggered();

    rif.m_table->selectAll();
    QCOMPARE(rif.RecycleSelection(), 3);

    QDir dir{mBenchmarkRedunFolder, "", QDir::SortFlag::NoSort, QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot};
    const QStringList items = dir.entryList();
    QVERIFY(items.isEmpty());

    emit inst.RELOAD_BENCHMARK_LIB->triggered();      // force reload from an no items path
    QCOMPARE(redunImgLib.ImgDataStruct().size(), 0);  // 0+0

    setConfig(RedunImgFinderKey::RUND_IMG_PATH, mFolderToFindRedun);
    emit inst.RELOAD_BENCHMARK_LIB->triggered();          // force reload from mFolderToFindRedun path
    QCOMPARE(redunImgLib.ImgDataStruct().size(), 3 + 2);  // 3 different hash + 2 different size

    rif.close();
  }
};

#include "RedundantImageFinderTest.moc"
REGISTER_TEST(RedundantImageFinderTest, false)
