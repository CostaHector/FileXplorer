#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "HarFiles.h"
#include "HarModel.h"
#include "EndToExposePrivateMember.h"

#include "TDir.h"

class HarModelTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void init() { HarFilesMocker::mockHarFiles().clear(); }
  void cleanupTestCase() { HarFilesMocker::mockHarFiles().clear(); }

  void initialize_ok() {
    HarModel hm;
    QCOMPARE(hm.rowCount(), 0);
    QCOMPARE(hm.columnCount(), 4);

    QCOMPARE(hm.headerData(0, Qt::Horizontal, Qt::DisplayRole).toString(), "Name");
    QCOMPARE(hm.headerData(1024, Qt::Horizontal, Qt::DisplayRole).toInt(), 1024 + 1); // will not overflow

    QCOMPARE(hm.headerData(0, Qt::Vertical, Qt::DisplayRole).toInt(), 0 + 1);
    QCOMPARE(hm.headerData(1024, Qt::Vertical, Qt::DisplayRole).toInt(), 1024 + 1); // will not overflow
    QCOMPARE(hm.headerData(0, Qt::Vertical, Qt::TextAlignmentRole).toInt(), (int) Qt::AlignRight);

    QVERIFY(hm.data(QModelIndex{}, Qt::DisplayRole).isNull());
    QVERIFY(hm.flags(QModelIndex{}) & Qt::ItemIsSelectable);
  }

  void setRootPath_ok() {
    HarFiles& mockFiles = HarFilesMocker::mockHarFiles();
    mockFiles.mHarItems = {
        {"file1.txt", QByteArray("Content1"), "text/plain", "http://example.com/file1"},   //
        {"image.png", QByteArray("PNG content"), "image/png", "http://example.com/image"}, //
    };

    HarModel hm;
    QCOMPARE(hm.setRootPath("test.har"), 2);
    QCOMPARE(hm.rowCount(), 2);

    QCOMPARE(hm.data(hm.index(0, 0), Qt::DisplayRole).toString(), "file1.txt");
    QCOMPARE(hm.data(hm.index(1, 0), Qt::DisplayRole).toString(), "image.png");

    QCOMPARE(hm.headerData(0, Qt::Vertical, Qt::DisplayRole).toInt(), 1);
    QCOMPARE(hm.headerData(1, Qt::Vertical, Qt::DisplayRole).toInt(), 2);

    mockFiles.mHarItems = {
        {"file1.txt", QByteArray("Content1"), "text/plain", "http://example.com/file1"},           //
        {"image.png", QByteArray("PNG content"), "image/png", "http://example.com/image"},         //
        {"new added one.svg", QByteArray("svg content"), "image/svg", "http://example.com/image"}, //
    };
    QCOMPARE(hm.setRootPath("new test.har"), 3);
    QCOMPARE(hm.rowCount(), 3);
  }

  void data_retrieve_ok() {
    HarFiles& mockFiles = HarFilesMocker::mockHarFiles();
    mockFiles.mHarItems = {{"document.pdf", QByteArray("PDF content"), "application/pdf", "http://example.com/doc"}};
    HarModel hm;
    hm.setRootPath("test.har");

    QModelIndex idx = hm.index(0, 0);
    QCOMPARE(hm.data(idx, Qt::DisplayRole).toString(), "document.pdf");
    QCOMPARE(hm.data(idx, Qt::DecorationRole).isValid(), true); // 图标
    QCOMPARE(hm.data(idx, Qt::ForegroundRole).isNull(), true);
    QCOMPARE(hm.data(idx, Qt::TextAlignmentRole).toInt(), (int) Qt::AlignLeft);

    idx = hm.index(0, 1);
    QCOMPARE(hm.data(idx, Qt::DisplayRole).toString(), "0'0'0'11"); // "PDF content" 11字节

    idx = hm.index(0, 2);
    QCOMPARE(hm.data(idx, Qt::DisplayRole).toString(), "application/pdf");

    idx = hm.index(0, 3);
    QCOMPARE(hm.data(idx, Qt::DisplayRole).toString(), "http://example.com/doc");

    {
      const HAR_FILE_ITEM& itemAtRow0 = hm.GetHarEntryItem(0);
      QCOMPARE(itemAtRow0.name, "document.pdf");
      QCOMPARE(itemAtRow0.content, QByteArray("PDF content"));
      QCOMPARE(itemAtRow0.type, "application/pdf");
      QCOMPARE(itemAtRow0.url, "http://example.com/doc");
    }

    {
      // out of bound index
      QModelIndex invalidIndex = hm.index(5, 0);
      QVERIFY(!hm.data(invalidIndex, Qt::DisplayRole).isValid());
      invalidIndex = hm.index(0, 5);
      QVERIFY(!hm.data(invalidIndex, Qt::DisplayRole).isValid());
    }
  }

  void save_to_local_ok() {
    TDir tDir;
    QVERIFY(tDir.IsValid());

    HarFiles& mockFiles = HarFilesMocker::mockHarFiles();
    mockFiles.mHarItems = {{"file1.txt", QByteArray("Content in file1"), "text/plain", ""}, //
                           {"image.png", QByteArray("Content in image"), "image/png", ""}};

    HarModel hm;
    hm.setRootPath("test.har");

    QVERIFY(!tDir.fileExists("file1.txt"));
    QVERIFY(!tDir.fileExists("image.png"));

    QList<int> selectedRows = {0, 1};
    int result = hm.SaveToLocal(tDir.path(), selectedRows);
    QCOMPARE(result, 2);

    QVERIFY(tDir.fileExists("file1.txt"));
    QVERIFY(tDir.fileExists("image.png"));
    QVERIFY(tDir.checkFileContents("file1.txt", {"Content in file1"}));
    QVERIFY(tDir.checkFileContents("image.png", {"Content in image"}));

    {
      // save to invalid path
      QCOMPARE(hm.SaveToLocal("path/to/inexists/path", selectedRows), -1);
      QCOMPARE(hm.SaveToLocal("path/to/inexists/path", {}), 0); // empty set<int>
    }
  }
};

#include "HarModelTest.moc"
REGISTER_TEST(HarModelTest, false)
