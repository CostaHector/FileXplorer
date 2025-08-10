#include <QCoreApplication>
#include <QtTest>
#include "GlbDataProtect.h"
#include "TDir.h"
#include "MyTestSuite.h"
#include "SyncModifiyFileSystem.h"

class SyncModifiyFileSystemTest : public MyTestSuite {
  Q_OBJECT
 public:
  SyncModifiyFileSystemTest() : MyTestSuite{false} {}
  SyncModifiyFileSystem syncMod;
  TDir mDir;
  const QString mBasicPath{mDir.path() + "/home"};
  const QString mSyncToPath{mDir.path() + "/bin"};
 private slots:
  void initTestCase() {
    QVERIFY(mDir.IsValid());
    QVERIFY(mDir.mkpath("home"));
    QVERIFY(mDir.mkpath("bin"));
    SyncModifiyFileSystem::m_basicPath = mBasicPath;
    SyncModifiyFileSystem::m_syncToPath = mSyncToPath;
  }

  void test_sync_switchoff_no_sync() {
    GlbDataProtect<bool> opSwBkp{syncMod.m_syncOperationSw};
    GlbDataProtect<bool> syncBackSwbkp{syncMod.m_syncBackSw};
    syncMod.m_syncOperationSw = false;
    syncMod.m_syncBackSw = true;
    // rename: path/oldItemName => path/newItemName
    QString path = mBasicPath;
    QVERIFY2(!syncMod(path), "switch is off, no sync");
    QCOMPARE(path, mBasicPath);
  }

  void test_rename_sync_back_ok() {
    GlbDataProtect<bool> opSwBkp{syncMod.m_syncOperationSw};
    GlbDataProtect<bool> syncBackSwbkp{syncMod.m_syncBackSw};
    syncMod.m_syncOperationSw = true;
    syncMod.m_syncBackSw = true;
    // rename: path1/oldItemName => path2/newItemName
    QString path1 = "D:/home/to/randompath";
    QVERIFY2(!syncMod(path1), "should no need to sync");

    path1 = mBasicPath;
    QVERIFY2(syncMod(path1), "should sync");
    QCOMPARE(path1, mSyncToPath);

    path1 = mSyncToPath;
    QVERIFY2(syncMod(path1), "should sync");
    QCOMPARE(path1, mBasicPath);
  }

  void test_rename_no_sync_back() {
    GlbDataProtect<bool> opSwBkp{syncMod.m_syncOperationSw};
    GlbDataProtect<bool> syncBackSwbkp{syncMod.m_syncBackSw};
    syncMod.m_syncOperationSw = true;
    syncMod.m_syncBackSw = false;
    // rename: path/oldItemName => path/newItemName
    QString path = mBasicPath;

    QVERIFY2(syncMod(path), "should sync");
    QCOMPARE(path, mSyncToPath);

    path = mSyncToPath;
    QVERIFY2(!syncMod(path), "no sync back");
    QCOMPARE(path, mSyncToPath);
  }

  void test_when_same_path_skip() {
    GlbDataProtect<bool> opSwBkp{syncMod.m_syncOperationSw};
    GlbDataProtect<bool> syncBackSwbkp{syncMod.m_syncBackSw};
    syncMod.m_syncOperationSw = true;
    syncMod.m_syncBackSw = true;
    // mod on items under "C:/Program Files (x86)" should not influence mBasicPath
    QString path = "C:/Program Files (x86)";
    QVERIFY(!syncMod(path));
    QCOMPARE(path, "C:/Program Files (x86)");
  }
};

#include "SyncModifiyFileSystemTest.moc"
SyncModifiyFileSystemTest g_SyncModifiyFileSystemTest;
