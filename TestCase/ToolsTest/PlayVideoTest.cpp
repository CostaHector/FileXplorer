#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "PublicTool.h"
#include "FileToolMock.h"
#include "PlayVideo.h"
#include "TDir.h"
#include <QDesktopServices>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class PlayVideoTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir tDir;
 private slots:
  void initTestCase() {
    QVERIFY(tDir.IsValid());
    QList<FsNodeEntry> nodes{
        {"videos.mp4", false, "contens in videos"},  //
        {"subdir", true, ""},                        //
    };
    QCOMPARE(tDir.createEntries(nodes), 2);  // 创建两个条目：一个文件，一个目录

    GlobalMockObject::reset();
    MOCKER(QDesktopServices::openUrl).stubs().will(returnValue(true));
  }

  void cleanupTestCase() {
    GlobalMockObject::verify();
  }

  void test_PlayNonExistingPath() {
    // 测试不存在的路径
    QString nonExistPath = tDir.itemPath("nonexist.mp4");
    QVERIFY(!QFile::exists(nonExistPath));
    QVERIFY(!on_ShiftEnterPlayVideo(nonExistPath));
  }

  void test_PlayExistingFile() {
    // 测试存在的文件
    QString filePath = tDir.itemPath("videos.mp4");
    QVERIFY(QFile::exists(filePath));
    QVERIFY(on_ShiftEnterPlayVideo(filePath));
  }

  void test_PlayDirectory() {
    // 测试目录
    QString dirPath = tDir.itemPath("subdir");
    QVERIFY(QFile::exists(dirPath));
    QVERIFY(on_ShiftEnterPlayVideo(dirPath));
  }

  void test_PlayADir() {
    // 测试PlayADir函数，在测试宏下直接返回true
    QString dirPath = tDir.itemPath("subdir");
    QVERIFY(PlayADir(dirPath));
  }
};

#include "PlayVideoTest.moc"
REGISTER_TEST(PlayVideoTest, false)
