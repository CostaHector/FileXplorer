#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "RedundantImageModel.h"
#include "EndToExposePrivateMember.h"
#include "TDir.h"
#include "ImageTestPrecoditionTools.h"

using namespace DuplicateImageMetaInfo;
using namespace ImageTestPrecoditionTools;

class RedundantImageModelTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir tDir;
  const QString mPath1 = tDir.itemPath("duplicate images.jpg");
  const QString mPath2 = tDir.itemPath("subFolder/duplicate images 2.jpg");

  RedundantImagesList mImagesList{
      // Name
      REDUNDANT_IMG_INFO{"duplicate images.jpg", 1, "md5md5", mPath1}, // exist
      REDUNDANT_IMG_INFO{"duplicate images 2.jpg", 1, "md5md5", mPath2}, // exist
      REDUNDANT_IMG_INFO{"duplicate images.png", 999, "AnotherMd5", "home/to/duplicate images.png"},
      REDUNDANT_IMG_INFO{"duplicate images.png", 999, "AnotherMd5", "home/to/duplicate images.png"},
  };
  const int IMAGES_CNT{mImagesList.size()};
 private slots:
  void initTestCase() {
    const QString svg1Content = GetSvgContentTemplate().arg(SVG_FILL_COLORS[0], "duplicate images.jpg");
    const QByteArray ba = svg1Content.toUtf8().constData();
    QList<FsNodeEntry> nodes{
        {"duplicate images.jpg", false, ba},
        {"subFolder/duplicate images 2.jpg", false, ba},
    };
    QCOMPARE(tDir.createEntries(nodes), nodes.size());
    QVERIFY(QFile::exists(mPath1));
    QVERIFY(QFile::exists(mPath2));
  }

  void initialize_state_ok() {
    RedundantImageModel rim;
    QVERIFY(rim.m_paf == nullptr);

    // below operation should not cause crash down
    {  // 0. row/column count ok
      QCOMPARE(rim.rowCount(), 0);
      QCOMPARE(rim.columnCount(), DuplicateImageMetaInfo::DI_TABLE_HEADERS_COUNT);
    }

    {  // 1. hor header
      QVERIFY(rim.columnCount() >= 1);
      QVERIFY(rim.columnCount() <= 999);
      QCOMPARE(rim.headerData(0, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole).toString(), DuplicateImageMetaInfo::DI_TABLE_HEADERS[0]);
      QCOMPARE(rim.headerData(0, Qt::Orientation::Horizontal, Qt::ItemDataRole::TextAlignmentRole).isNull(), true);
      QCOMPARE(rim.headerData(999, Qt::Orientation::Horizontal, Qt::ItemDataRole::TextAlignmentRole).isNull(), true);
    }

    {  // 2. ver header
      QVERIFY(rim.rowCount() == 0);
      QCOMPARE(rim.headerData(0, Qt::Orientation::Vertical, Qt::ItemDataRole::DisplayRole).toInt(), 0 + 1);  // the 1st row
      QCOMPARE(rim.headerData(0, Qt::Orientation::Vertical, Qt::ItemDataRole::TextAlignmentRole).toInt(), Qt::AlignmentFlag::AlignRight);
    }

    QModelIndex invalidModelIndex;

    {  // 3. data ok
      QCOMPARE(rim.data(invalidModelIndex, Qt::ItemDataRole::DisplayRole).isNull(), true);
      QCOMPARE(rim.data(invalidModelIndex, Qt::ItemDataRole::DecorationRole).isNull(), true);
    }

    {  // 4. file property ok
      QCOMPARE(rim.filePath(invalidModelIndex), "");
    }

    {  // 5. setRootPath to nullptr
      rim.setRootPath(nullptr);
      QCOMPARE(rim.rowCount(), 0);
    }
  }

  void setRootPath_ok() {
    RedundantImageModel rim;
    QVERIFY(rim.m_paf == nullptr);

    QCOMPARE(rim.setRootPath(&mImagesList), IMAGES_CNT);
    QCOMPARE(rim.rowCount(), IMAGES_CNT);  // 4 elements
    QVERIFY(IMAGES_CNT >= 4);

    QCOMPARE(rim.data(rim.index(0, DIColumnE::Name), Qt::ItemDataRole::DisplayRole).toString(), "duplicate images.jpg");
    QCOMPARE(rim.data(rim.index(1, DIColumnE::Name), Qt::ItemDataRole::DisplayRole).toString(), "duplicate images 2.jpg");
    QCOMPARE(rim.data(rim.index(2, DIColumnE::Name), Qt::ItemDataRole::DisplayRole).toString(), "duplicate images.png");
    QCOMPARE(rim.data(rim.index(3, DIColumnE::Name), Qt::ItemDataRole::DisplayRole).toString(), "duplicate images.png");

    QCOMPARE(rim.data(rim.index(0, DIColumnE::Size), Qt::ItemDataRole::DisplayRole).toString(), "0'0'0'1");
    QCOMPARE(rim.data(rim.index(1, DIColumnE::Size), Qt::ItemDataRole::DisplayRole).toString(), "0'0'0'1");
    QCOMPARE(rim.data(rim.index(2, DIColumnE::Size), Qt::ItemDataRole::DisplayRole).toString(), "0'0'0'999");
    QCOMPARE(rim.data(rim.index(3, DIColumnE::Size), Qt::ItemDataRole::DisplayRole).toString(), "0'0'0'999");

    QCOMPARE(rim.data(rim.index(0, DIColumnE::MD5), Qt::ItemDataRole::DisplayRole).toString(), "md5md5");
    QCOMPARE(rim.data(rim.index(1, DIColumnE::MD5), Qt::ItemDataRole::DisplayRole).toString(), "md5md5");
    QCOMPARE(rim.data(rim.index(2, DIColumnE::MD5), Qt::ItemDataRole::DisplayRole).toString(), "AnotherMd5");
    QCOMPARE(rim.data(rim.index(3, DIColumnE::MD5), Qt::ItemDataRole::DisplayRole).toString(), "AnotherMd5");

    QCOMPARE(rim.data(rim.index(0, DIColumnE::AbsPath), Qt::ItemDataRole::DisplayRole).toString(), mPath1);
    QCOMPARE(rim.data(rim.index(1, DIColumnE::AbsPath), Qt::ItemDataRole::DisplayRole).toString(), mPath2);
    QCOMPARE(rim.data(rim.index(2, DIColumnE::AbsPath), Qt::ItemDataRole::DisplayRole).toString(), "home/to/duplicate images.png");
    QCOMPARE(rim.data(rim.index(3, DIColumnE::AbsPath), Qt::ItemDataRole::DisplayRole).toString(), "home/to/duplicate images.png");

    // image show in AbsPath column not Name column.
    QCOMPARE(rim.data(rim.index(0, DIColumnE::Name), Qt::ItemDataRole::DecorationRole).isNull(), true);
    QCOMPARE(rim.data(rim.index(0, DIColumnE::AbsPath), Qt::ItemDataRole::DecorationRole).isValid(), true); // the first and second image exist
    QCOMPARE(rim.data(rim.index(1, DIColumnE::AbsPath), Qt::ItemDataRole::DecorationRole).isValid(), true);
    QCOMPARE(rim.data(rim.index(1, DIColumnE::AbsPath), Qt::ItemDataRole::BackgroundRole).isNull(), true);  // this role not set

    QCOMPARE(rim.data(rim.index(2, DIColumnE::AbsPath), Qt::ItemDataRole::DecorationRole).isNull(), true); // the third image not exist

    QCOMPARE(rim.filePath(rim.index(0, DIColumnE::Name)), mPath1);
    QCOMPARE(rim.filePath(rim.index(1, DIColumnE::Name)), mPath2);
    QCOMPARE(rim.filePath(rim.index(2, DIColumnE::Name)), "home/to/duplicate images.png");
    QCOMPARE(rim.filePath(rim.index(3, DIColumnE::Name)), "home/to/duplicate images.png");

    QVERIFY(rim.rowCount() <= 999);
    QCOMPARE(rim.filePath(rim.index(999, DIColumnE::Name)), "");  // out of bound
  }
};

#include "RedundantImageModelTest.moc"
REGISTER_TEST(RedundantImageModelTest, false)
