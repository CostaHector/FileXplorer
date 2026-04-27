#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "DuplicateImagesModel.h"
#include "ImagesInfoManager.h"
#include "EndToExposePrivateMember.h"

#include "TDir.h"
#include "RedunImgFinderKey.h"
#include "Configuration.h"
#include "DataFormatter.h"
using namespace DuplicateImageMetaInfo;
using namespace DuplicateImageDetectionCriteria;

QByteArray GetSvgRawByteArray();

class DuplicateImagesModelTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir mDir;
  const QString mWorkPath{mDir.path()};
  const QString mBenchmarkRedunFolder{mWorkPath + "/Benchmark"};
  const QString mFolderToFindRedun{mWorkPath + "/ToFindRedun"};
  int SVG_FILE_SIZE_IN_BYTES{0};
private slots:
  void initTestCase() {
    QVERIFY(mDir.IsValid());
    QByteArray validImgContents = GetSvgRawByteArray();
    SVG_FILE_SIZE_IN_BYTES = validImgContents.size();
    QCOMPARE(SVG_FILE_SIZE_IN_BYTES, 129);
    QCOMPARE(DataFormatter::formatFileSizeGMKB(SVG_FILE_SIZE_IN_BYTES), "0'0'0'129");
    const QList<FsNodeEntry> gNode{
        FsNodeEntry{"Benchmark/a.jpg", false, validImgContents},          // bench
        FsNodeEntry{"Benchmark/aDuplicate.png", false, validImgContents}, // bench
        FsNodeEntry{"Benchmark/bUnique.png", false, "456"},               // bench
        FsNodeEntry{"ToFindRedun/aRedun.jpg", false, validImgContents},   //
        FsNodeEntry{"ToFindRedun/cUnique.png", false, "789789"},          //
        FsNodeEntry{"ToFindRedun/dEmpty.webp", false, ""},                //
    };
    QVERIFY(mDir.createEntries(gNode) >= gNode.size());

    Configuration().clear();
  }

  void default_state_ok() {
    DuplicateImagesModel model;

    // below operation should not cause crash down
    { // 0. row/column count ok
      QCOMPARE(model.rowCount(), 0);
      QCOMPARE(model.columnCount(), DI_TABLE_HEADERS_COUNT);
    }

    { // 1. hor header
      QCOMPARE(DI_TABLE_HEADERS_COUNT, 5);
      QCOMPARE(model.headerData(0, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole).toString(), "Name");
      QCOMPARE(model.headerData(1, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole).toString(), "Size");
      QCOMPARE(model.headerData(2, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole).toString(), "MD5");
      QCOMPARE(model.headerData(3, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole).toString(), "RelPath");
      QCOMPARE(model.headerData(4, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole).toString(), "Preview");

      QCOMPARE(model.headerData(0, Qt::Orientation::Horizontal, Qt::ItemDataRole::TextAlignmentRole), QVariant());
      QCOMPARE(model.headerData(999, Qt::Orientation::Horizontal, Qt::ItemDataRole::TextAlignmentRole), QVariant());
    }

    { // 2. ver header
      QVERIFY(model.rowCount() == 0);
      QCOMPARE(model.headerData(0, Qt::Orientation::Vertical, Qt::ItemDataRole::DisplayRole).toInt(), 1);   // the 1st row
      QCOMPARE(model.headerData(1, Qt::Orientation::Vertical, Qt::ItemDataRole::DisplayRole).toInt(), 2);   // the 2nd row
      QCOMPARE(model.headerData(0, Qt::Orientation::Vertical, Qt::ItemDataRole::TextAlignmentRole).toInt(), //
               Qt::AlignmentFlag::AlignRight);
    }

    { // 3. data ok
      QCOMPARE(model.data({}, Qt::ItemDataRole::DisplayRole).isNull(), true);
      QCOMPARE(model.data({}, Qt::ItemDataRole::DecorationRole).isNull(), true);
    }

    { // 4. file property ok
      QCOMPARE(model.filePath({}), "");
    }

    { // 5. setDatas
      RedundantImagesList emptyList;
      model.setDatas(emptyList);
      QCOMPARE(model.rowCount(), 0);
    }
  }

  void setRootPath_ok() {
    QCOMPARE(RedunImgFinderKey::DISABLE_IMAGE_DECORATION.v.data.b, false);
    DuplicateImagesModel model;
    QCOMPARE(model.m_bDisableImage, false); // image not disabled

    QCOMPARE(model.rootPath(), "");
    QCOMPARE(model.mFindBy, DuplicateImageDetectionCriteria::DICriteriaE::END_INVALID);
    QCOMPARE(model.m_bIncludeEmptyImage, true);

    QCOMPARE(model.setRootPath(model.rootPath(), model.mFindBy, model.m_bIncludeEmptyImage), 0); // unchange
    QCOMPARE(model.setRootPath("", DICriteriaE::LIBRARY, true), -1);                             // path empty
    QCOMPARE(model.setRootPath("inexist/folder", DICriteriaE::LIBRARY, true), -1);               // path not exist
    QCOMPARE(model.setRootPath(mWorkPath, DICriteriaE::END_INVALID, true), -1);                  // findBy invalid

    ImagesInfoManager& redunImgLib = ImagesInfoManager::getInst();
    redunImgLib.InitializeImpl(mBenchmarkRedunFolder);

    QCOMPARE(model.setRootPath(mWorkPath, DICriteriaE::MD5, true), 4); // MD5+include empty
    {
      QCOMPARE(model.index(0, DIColumnE::Name).data(Qt::DisplayRole).toString(), "a.jpg");
      QCOMPARE(model.index(1, DIColumnE::Name).data(Qt::DisplayRole).toString(), "aDuplicate.png");
      QCOMPARE(model.index(2, DIColumnE::Name).data(Qt::DisplayRole).toString(), "aRedun.jpg");
      QCOMPARE(model.index(3, DIColumnE::Name).data(Qt::DisplayRole).toString(), "dEmpty.webp");

      QCOMPARE(model.index(0, DIColumnE::Size).data(Qt::DisplayRole).toString(), "0'0'0'129");
      QCOMPARE(model.index(1, DIColumnE::Size).data(Qt::DisplayRole).toString(), "0'0'0'129");
      QCOMPARE(model.index(2, DIColumnE::Size).data(Qt::DisplayRole).toString(), "0'0'0'129");
      QCOMPARE(model.index(3, DIColumnE::Size).data(Qt::DisplayRole).toString(), "0'0'0'0");

      QCOMPARE(model.index(0, DIColumnE::MD5).data(Qt::DisplayRole).toByteArray().isEmpty(), false);
      QCOMPARE(model.index(1, DIColumnE::MD5).data(Qt::DisplayRole).toByteArray().isEmpty(), false);
      QCOMPARE(model.index(2, DIColumnE::MD5).data(Qt::DisplayRole).toByteArray().isEmpty(), false);
      QCOMPARE(model.index(3, DIColumnE::MD5).data(Qt::DisplayRole).toByteArray().isEmpty(), true); // empty file, empty md5

      QCOMPARE(model.index(0, DIColumnE::RelPath).data(Qt::DisplayRole).toString(), "/Benchmark/");
      QCOMPARE(model.index(1, DIColumnE::RelPath).data(Qt::DisplayRole).toString(), "/Benchmark/");
      QCOMPARE(model.index(2, DIColumnE::RelPath).data(Qt::DisplayRole).toString(), "/ToFindRedun/");
      QCOMPARE(model.index(3, DIColumnE::RelPath).data(Qt::DisplayRole).toString(), "/ToFindRedun/");

      QVERIFY(!model.index(0, DIColumnE::Name).data(Qt::DecorationRole).isValid()); // not DUP_IMG_PREVIEW colume
      {
        QVariant imgVar0 = model.index(0, DIColumnE::DUP_IMG_PREVIEW).data(Qt::DecorationRole);
        QVariant imgVar1 = model.index(1, DIColumnE::DUP_IMG_PREVIEW).data(Qt::DecorationRole);
        QVariant imgVar2 = model.index(2, DIColumnE::DUP_IMG_PREVIEW).data(Qt::DecorationRole);
        QVariant imgVar3 = model.index(3, DIColumnE::DUP_IMG_PREVIEW).data(Qt::DecorationRole); // load failed
        QVERIFY(imgVar0.isValid());
        QVERIFY(imgVar1.isValid());
        QVERIFY(imgVar2.isValid());
      }

      QCOMPARE(model.onDisableImageDecorationChanged(false), false); // unchange
      QCOMPARE(model.onDisableImageDecorationChanged(true), true);   // false->true disabled
      {
        QVariant imgVar0 = model.index(0, DIColumnE::DUP_IMG_PREVIEW).data(Qt::DecorationRole);
        QVariant imgVar1 = model.index(1, DIColumnE::DUP_IMG_PREVIEW).data(Qt::DecorationRole);
        QVariant imgVar2 = model.index(2, DIColumnE::DUP_IMG_PREVIEW).data(Qt::DecorationRole);
        QVariant imgVar3 = model.index(3, DIColumnE::DUP_IMG_PREVIEW).data(Qt::DecorationRole); // load failed
        QVERIFY(!imgVar0.isValid());
        QVERIFY(!imgVar1.isValid());
        QVERIFY(!imgVar2.isValid());
        QVERIFY(!imgVar3.isValid());
      }
      QCOMPARE(model.onDisableImageDecorationChanged(false), true); // true->false shown
      {
        QVariant imgVar0 = model.index(0, DIColumnE::DUP_IMG_PREVIEW).data(Qt::DecorationRole);
        QVariant imgVar1 = model.index(1, DIColumnE::DUP_IMG_PREVIEW).data(Qt::DecorationRole);
        QVariant imgVar2 = model.index(2, DIColumnE::DUP_IMG_PREVIEW).data(Qt::DecorationRole);
        QVariant imgVar3 = model.index(3, DIColumnE::DUP_IMG_PREVIEW).data(Qt::DecorationRole); // load failed
        QVERIFY(imgVar0.isValid());
        QVERIFY(imgVar1.isValid());
        QVERIFY(imgVar2.isValid());
      }

      QCOMPARE(model.filePath(model.index(0, DIColumnE::RelPath)), mDir.itemPath("Benchmark/a.jpg"));
      QCOMPARE(model.filePath(model.index(1, DIColumnE::RelPath)), mDir.itemPath("Benchmark/aDuplicate.png"));
      QCOMPARE(model.filePath(model.index(2, DIColumnE::RelPath)), mDir.itemPath("ToFindRedun/aRedun.jpg"));
      QCOMPARE(model.filePath(model.index(3, DIColumnE::RelPath)), mDir.itemPath("ToFindRedun/dEmpty.webp"));
    }

    QCOMPARE(model.setIncludeEmptyImg(true), 0);  // no change
    QCOMPARE(model.setIncludeEmptyImg(false), 3); // MD5+not include empty

    QCOMPARE(model.setFindBy(DICriteriaE::MD5), 0); // no change

    QCOMPARE(model.setRootPath(mFolderToFindRedun, DICriteriaE::MD5, false), 0); // MD5+not include empty. total files=3
    QCOMPARE(model.setFindBy(DICriteriaE::LIBRARY), 1);                          // Benchmark+not include empty
    {
      QCOMPARE(model.filePath(model.index(0, DIColumnE::RelPath)), mDir.itemPath("ToFindRedun/aRedun.jpg"));
    }

    QCOMPARE(model.setIncludeEmptyImg(true), 2); // Benchmark+include empty
    {
      QCOMPARE(model.filePath(model.index(0, DIColumnE::RelPath)), mDir.itemPath("ToFindRedun/aRedun.jpg"));
      QCOMPARE(model.filePath(model.index(1, DIColumnE::RelPath)), mDir.itemPath("ToFindRedun/dEmpty.webp"));
    }

    QCOMPARE(model.UpdateDisplayWhenRecycled({}), 0);

    const QModelIndexList lst{model.index(0, DIColumnE::Name), model.index(1, DIColumnE::Name)};
    QCOMPARE(model.UpdateDisplayWhenRecycled(lst), 2);
    QCOMPARE(model.rowCount(), 0);
  }

  void GetSameHashRowWithFirstOneIgnored_ok() {
    DuplicateImagesModel model;

    QItemSelection emptySelection = model.GetSameHashRowWithFirstOneIgnored();
    QCOMPARE(emptySelection.isEmpty(), true);

    QCOMPARE(model.setRootPath(mWorkPath, DICriteriaE::MD5, true), 4);

    // FsNodeEntry{"Benchmark/a.jpg", false, validImgContents},          // bench
    // FsNodeEntry{"Benchmark/aDuplicate.png", false, validImgContents}, // bench
    // FsNodeEntry{"Benchmark/bUnique.png", false, "456"},               // bench
    // FsNodeEntry{"ToFindRedun/aRedun.jpg", false, validImgContents},   //
    // FsNodeEntry{"ToFindRedun/cUnique.png", false, "789789"},          //
    // FsNodeEntry{"ToFindRedun/dEmpty.webp", false, ""},                //

    QItemSelection selectionWith2Row = model.GetSameHashRowWithFirstOneIgnored();

    QCOMPARE(model.index(0, DIColumnE::Name).data(Qt::DisplayRole).toString(), "a.jpg");          // keep
    QCOMPARE(model.index(1, DIColumnE::Name).data(Qt::DisplayRole).toString(), "aDuplicate.png"); // contains
    QCOMPARE(model.index(2, DIColumnE::Name).data(Qt::DisplayRole).toString(), "aRedun.jpg");     // contains
    QCOMPARE(model.index(3, DIColumnE::Name).data(Qt::DisplayRole).toString(), "dEmpty.webp");    // keep
    QVERIFY(!selectionWith2Row.contains(model.index(0, DIColumnE::Name)));
    QVERIFY(selectionWith2Row.contains(model.index(1, DIColumnE::Name)));
    QVERIFY(selectionWith2Row.contains(model.index(2, DIColumnE::Name)));
    QVERIFY(!selectionWith2Row.contains(model.index(3, DIColumnE::Name)));
  }
};

QByteArray GetSvgRawByteArray() {
  // never place following literal str before testcase directly
  // otherwise when link one will get undefined reference to `vtable for XXXTest'
  return R"(<?xml version="1.0" encoding="utf-8"?>
<svg width="24" height="24" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
</svg>)";
}

#include "DuplicateImagesModelTest.moc"
REGISTER_TEST(DuplicateImagesModelTest, false)
