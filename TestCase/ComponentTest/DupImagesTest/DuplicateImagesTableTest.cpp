#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "DuplicateImagesTable.h"
#include "RedundantImageFinderActions.h"
#include "ImagesInfoManager.h"
#include "EndToExposePrivateMember.h"

#include "RedunImgFinderKey.h"
#include "Configuration.h"
#include "UndoRedo.h"
#include "FileTool.h"
#include "TDir.h"
using namespace DuplicateImageDetectionCriteria;

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class DuplicateImagesTableTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir mDir;
  const QString mWorkPath{mDir.path()};
private slots:
  void initTestCase() {
    QVERIFY(mDir.IsValid());
    const QList<FsNodeEntry> gNode{
        FsNodeEntry{"a.jpg", false, "012"},     // same hash and same size
        FsNodeEntry{"redun.png", false, "012"}, // same hash and same size
    };
    QCOMPARE(mDir.createEntries(gNode), 2);
  }
  void init() { GlobalMockObject::reset(); }
  void cleanup() { GlobalMockObject::verify(); }

  void default_ok() {
    DuplicateImagesTable view;
    QVERIFY(view.m_imgModel != nullptr);
    QVERIFY(view.m_imgProxy != nullptr);
    QCOMPARE(view.m_imgProxy->rowCount(), 0);

    QCOMPARE(view("inexist/folder"), false);
    QCOMPARE(view(""), false);
    view.onFindByChanged(DICriteriaE::MD5); // depends on if path/findBy/includeEmpty changed
    view.onIncludeEmptyImgChanged(true);    // depends on if path/findBy/includeEmpty changed

    QCOMPARE(view.GetWinTitle(), "Redundant Images Finder | Path:  | 0 item(s)");

    view.clearSelection();
    QCOMPARE(view.RecycleSelection(), 0);

    QCOMPARE(view.onOpenImageDoubleClicked({}), false);
  }

  void onOpenImageDoubleClicked_ok() {
    auto& findActInst = g_redunImgFinderAg();
    findActInst.mDecideByIntAction.setCheckedIfActionExist(DICriteriaE::MD5);
    QCOMPARE(findActInst.GetCurFindDupBy(), DICriteriaE::MD5);

    DuplicateImagesTable view;
    QCOMPARE(view(mWorkPath), true);

    DuplicateImagesModel* srcModel = view.m_imgModel;
    QSortFilterProxyModel* proModel = view.m_imgProxy;
    QCOMPARE(srcModel->rowCount(), 2);
    QCOMPARE(proModel->rowCount(), 2);

    { // onOpenImageDoubleClicked ok
      QModelIndex src0 = srcModel->index(0, DuplicateImageMetaInfo::DIColumnE::Name);
      QModelIndex pro0 = proModel->mapFromSource(src0);
      QCOMPARE(pro0.isValid(), true);
      const QString fileAbsPath{mDir.itemPath("a.jpg")};
      QCOMPARE(srcModel->filePath(src0), fileAbsPath);

      MOCKER(FileTool::OpenLocalImageFile) //
          .expects(exactly(1))             //
          .with(eq(fileAbsPath))           //
          .will(returnValue(true));

      QCOMPARE(view.onOpenImageDoubleClicked(pro0), true);
    }

    { // _DISABLE_IMAGE_DECORATION ok
      QSignalSpy decorationRoleChangedSpy{srcModel, &DuplicateImagesModel::dataChanged};
      findActInst._DISABLE_IMAGE_DECORATION->toggle();
      QCOMPARE(getConfig(RedunImgFinderKey::DISABLE_IMAGE_DECORATION), findActInst.isDiabledImageDecoration());
      QCOMPARE(decorationRoleChangedSpy.count(), 1);
      decorationRoleChangedSpy.takeLast();

      findActInst._DISABLE_IMAGE_DECORATION->toggle();
      QCOMPARE(getConfig(RedunImgFinderKey::DISABLE_IMAGE_DECORATION), findActInst.isDiabledImageDecoration());
      QCOMPARE(decorationRoleChangedSpy.count(), 1);
      decorationRoleChangedSpy.takeLast();
    }

    { // clearSelection no row selected
      view.clearSelection();
      QCOMPARE(view.RecycleSelection(), 0);
    }

    {
      // remove all rows
      ImagesInfoManager& redunImgLib = ImagesInfoManager::getInst();
      redunImgLib.InitializeImpl(mWorkPath);
      QCOMPARE(redunImgLib.count(), 2); // same hash and same size, hash1+size1=2

      view.selectAll();
      QCOMPARE(view.selectedRowsSource().size(), 2);
      QCOMPARE(view.RecycleSelection(), 2);
      QCOMPARE(srcModel->rowCount(), 0);

      QVERIFY(!mDir.fileExists("a.jpg", false));
      QVERIFY(!mDir.fileExists("redun.png", false));

      findActInst.RELOAD_BENCHMARK_LIB->trigger();
      QCOMPARE(redunImgLib.count(), 0); // hash0+size0=0

      // recover all files removed
      QVERIFY(UndoRedo::on_Undo());

      QVERIFY(mDir.fileExists("a.jpg", false));
      QVERIFY(mDir.fileExists("redun.png", false));

      redunImgLib.ForceReloadImpl();
      QCOMPARE(redunImgLib.count(), 2); // same hash and same size, hash1+size1=2
    }
  }

  void onOpenBenchmarkFolder_ok() {
    ImagesInfoManager& redunImgLib = ImagesInfoManager::getInst();
    redunImgLib.InitializeImpl(mWorkPath); // init path here

    QCOMPARE(ImagesInfoManager::GetDynRedunPath(), mWorkPath);

    MOCKER(FileTool::OpenLocalFileUsingDesktopService) //
        .expects(exactly(1))                           //
        .with(eq(mWorkPath))
        .will(returnValue(true));
    QCOMPARE(RedundantImageFinderActions::onOpenBenchmarkFolder(), true);
  }

  void SelectRowsToDelete_ok() {
    auto& findActInst = g_redunImgFinderAg();
    findActInst.mDecideByIntAction.setCheckedIfActionExist(DICriteriaE::MD5);
    QCOMPARE(findActInst.GetCurFindDupBy(), DICriteriaE::MD5);

    DuplicateImagesTable view;
    QCOMPARE(view.SelectRowsToDelete(), false);
    QCOMPARE(view(mWorkPath), true);

    DuplicateImagesModel* srcModel = view.m_imgModel;
    QSortFilterProxyModel* proModel = view.m_imgProxy;
    QCOMPARE(srcModel->rowCount(), 2);
    QCOMPARE(proModel->rowCount(), 2);

    QCOMPARE(view.SelectRowsToDelete(), true);
    QModelIndexList srcIndexes = view.selectedRowsSource();
    QCOMPARE(srcIndexes.size(), 1); // expect one line get selected

    // 0. "a.jpg" keep, not selected
    // 1. "redun.png" selected
    QCOMPARE(srcIndexes.front().data(Qt::DisplayRole).toString(), "redun.png");
  }
};

#include "DuplicateImagesTableTest.moc"
REGISTER_TEST(DuplicateImagesTableTest, false)
