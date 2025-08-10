#include <QCoreApplication>
#include <QtTest>
#include "TestCase/pubTestTool/MyTestSuite.h"
#include "TestCase/pubTestTool/TDir.h"
#include "Tools/FileSystemItemFilter.h"

using namespace FileSystemItemFilter;
class FileSystemItemFilterTest : public MyTestSuite {
  Q_OBJECT
 public:
  FileSystemItemFilterTest() : MyTestSuite{false} {}
  TDir mDir;
  const QString mWorkPath{mDir.path()};
  const QFile rootFolder{mWorkPath};
  const QFile fi5{mWorkPath + "/5.txt"};
  const QFile fiSuperMan1{mWorkPath + "/super man 1.jpg"};
  const QFile folderMovie{mWorkPath + "/movie"};
  const QFile fiSuperMan10{mWorkPath + "/movie/super man 10.mp4"};
 private slots:
  void initTestCase() {
    // movie/super man 10.mp4
    // 5.txt
    // super man 1.jpg
    QVERIFY(mDir.IsValid());
    const QList<FsNodeEntry> gNodes{
        FsNodeEntry{"movie/super man 10.mp4", false, "0123456789"},  //
        FsNodeEntry{"5.txt", false, "01234"},                        //
        FsNodeEntry{"super man 1.jpg", false, "0"},                  //
    };
    QVERIFY(mDir.createEntries(gNodes) >= gNodes.size());
  }

  void test_ItemCounterOut() {
    // procedure
    const ItemStatistic& items = ItemCounter({mWorkPath});
    QCOMPARE(items.fileCnt, 3);
    QCOMPARE(items.folderCnt, 2);
    QCOMPARE(items.fileSize, fi5.size() + fiSuperMan1.size() + fiSuperMan10.size());
  }

  void test_FilesOut_no_filter() {
    const QStringList& files = FilesOut({mWorkPath});
    const QSet<QString> actualFiles{files.cbegin(), files.cend()};
    const QSet<QString> expectFiles{fi5.fileName(), fiSuperMan1.fileName(), fiSuperMan10.fileName()};
    QCOMPARE(actualFiles, expectFiles);
  }

  void test_FilesOut_filter() {
    const QStringList& superManFiles = FilesOut({mWorkPath}, {"*super man*"});
    const QSet<QString> actualSuperManFiles{superManFiles.cbegin(), superManFiles.cend()};
    const QSet<QString> expectSuperManFiles{fiSuperMan1.fileName(), fiSuperMan10.fileName()};
    QCOMPARE(actualSuperManFiles, expectSuperManFiles);
  }

  void test_mp4_Out_ok() {
    const QStringList& mp4s = MP4Out({mWorkPath});
    const QSet<QString> actualMp4s{mp4s.cbegin(), mp4s.cend()};
    const QSet<QString> expectMp4s{fiSuperMan10.fileName()};
    QCOMPARE(actualMp4s, expectMp4s);
  }
};

#include "FileSystemItemFilterTest.moc"
FileSystemItemFilterTest g_FileSystemItemFilterTest;
