#include <QCoreApplication>
#include <QtTest>
#include "TestCase/pub/GlbDataProtect.h"
#include "pub/MyTestSuite.h"
// add necessary includes here
#include "Tools/SyncModifiyFileSystem.h"

class SyncModifiyFileSystemTest : public MyTestSuite {
  Q_OBJECT
 public:
  SyncModifiyFileSystem syncMod;

 private slots:
  void initTestCase() {
    syncMod.SetBasicPath("C:/Program Files");
    syncMod.SetSynchronizedToPaths("C:/Users");
    qDebug("Start SyncModifiyFileSystemTest...");
  }
  void cleanupTestCase() { qDebug("End SyncModifiyFileSystemTest..."); }

  void test_syncSwitchOff() {
    GlbDataProtect<bool> bkp{syncMod.m_syncModifyFileSystemSwitch};
    syncMod.m_syncModifyFileSystemSwitch = false;
    syncMod.m_alsoSyncReversebackSwitch = true;
    // rename: path/oldItemName => path/newItemName
    QString path = "C:/Program Files";
    QVERIFY2(!syncMod(path), "switch is off, no sync");
    QCOMPARE(path, "C:/Program Files");
  }

  void test_renameSyncBack() {
    GlbDataProtect<bool> bkp{syncMod.m_syncModifyFileSystemSwitch};
    syncMod.m_syncModifyFileSystemSwitch = true;
    syncMod.m_alsoSyncReversebackSwitch = true;
    // rename: path1/oldItemName => path2/newItemName
    QString path1 = "D:/home/to/randompath";
    QString path2 = "D:/home";
    QVERIFY2(!syncMod(path1), "should no need to sync");

    path1 = "C:/Program Files";
    path2 = "D:/home";
    QVERIFY2(syncMod(path1), "should sync");
    QCOMPARE(path1, "C:/Users");

    path1 = "C:/Users";
    QVERIFY2(syncMod(path1), "should sync");
    QCOMPARE(path1, "C:/Program Files");
  }

  void test_renameNoSyncBack() {
    GlbDataProtect<bool> bkp{syncMod.m_syncModifyFileSystemSwitch};
    syncMod.m_syncModifyFileSystemSwitch = true;
    syncMod.m_alsoSyncReversebackSwitch = false;
    // rename: path/oldItemName => path/newItemName
    QString path = "C:/Program Files";

    QVERIFY2(syncMod(path), "should sync");
    QCOMPARE(path, "C:/Users");

    path = "C:/Users";
    QVERIFY2(!syncMod(path), "no sync back");
    QCOMPARE(path, "C:/Users");
  }

  void test_whenSimilarPath() {
    GlbDataProtect<bool> bkp{syncMod.m_syncModifyFileSystemSwitch};
    syncMod.m_syncModifyFileSystemSwitch = true;
    syncMod.m_alsoSyncReversebackSwitch = true;
    syncMod.SetBasicPath("C:/Program Files");
    syncMod.SetSynchronizedToPaths("C:/Users");
    // mod on items under "C:/Program Files (x86)" should not influence "C:/Program Files"
    QString path = "C:/Program Files (x86)";
    QVERIFY(!syncMod(path));
    QCOMPARE(path, "C:/Program Files (x86)");
  }
};

#include "SyncModifiyFileSystemTest.moc"
SyncModifiyFileSystemTest g_SyncModifiyFileSystemTest;
