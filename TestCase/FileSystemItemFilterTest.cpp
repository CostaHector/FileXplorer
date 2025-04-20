#include <QCoreApplication>
#include <QtTest>
#include "pub/MyTestSuite.h"
#include "Tools/FileSystemItemFilter.h"
using namespace FileSystemItemFilter;

const QString rootpath{QFileInfo(__FILE__).absolutePath() + "/test/TestEnv_FileSystemItemFilter"};
const QFile rootFolder{rootpath};
const QFile fi5{rootpath + "/5.txt"};
const QFile fiSuperMan1{rootpath + "/super man 1.jpg"};
const QFile folderMovie{rootpath + "/movie"};
const QFile fiSuperMan10{rootpath + "/movie/super man 10.mp4"};

class FileSystemItemFilterTest : public MyTestSuite {
  Q_OBJECT
 public:
 private slots:
  void test_precondition() {
    // precondition
    QVERIFY(rootFolder.exists());
    QVERIFY(fi5.exists());
    QVERIFY(fiSuperMan1.exists());
    QVERIFY(folderMovie.exists());
    QVERIFY(fiSuperMan10.exists());
  }

  void test_ItemCounterOut() {
    // procedure
    const ItemStatistic& items = ItemCounter({rootpath});
    QCOMPARE(items.fileCnt, 3);
    QCOMPARE(items.folderCnt, 2);
    QCOMPARE(items.fileSize, fi5.size() + fiSuperMan1.size() + fiSuperMan10.size());
  }

  void test_FilesOut_no_filter() {
    const QStringList& files = FilesOut({rootpath});
    const QSet<QString> actualFiles{files.cbegin(), files.cend()};
    const QSet<QString> expectFiles{fi5.fileName(), fiSuperMan1.fileName(), fiSuperMan10.fileName()};
    QCOMPARE(actualFiles, expectFiles);
  }

  void test_FilesOut_filter() {
    const QStringList& superManFiles = FilesOut({rootpath}, {"*super man*"});
    const QSet<QString> actualSuperManFiles{superManFiles.cbegin(), superManFiles.cend()};
    const QSet<QString> expectSuperManFiles{fiSuperMan1.fileName(), fiSuperMan10.fileName()};
    QCOMPARE(actualSuperManFiles, expectSuperManFiles);
  }

  void test_MP4Out() {
    const QStringList& mp4s = MP4Out({rootpath});
    const QSet<QString> actualMp4s{mp4s.cbegin(), mp4s.cend()};
    const QSet<QString> expectMp4s{fiSuperMan10.fileName()};
    QCOMPARE(actualMp4s, expectMp4s);
  }
};

#include "FileSystemItemFilterTest.moc"
FileSystemItemFilterTest g_FileSystemItemFilterTest;
