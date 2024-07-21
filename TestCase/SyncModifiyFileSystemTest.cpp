#include <QCoreApplication>
#include <QtTest>

// add necessary includes here
#include "Component/SyncModifiyFileSystem.h"

class SyncModifiyFileSystemTest : public QObject {
  Q_OBJECT
 public:
  SyncModifiyFileSystem syncMod;

 private slots:
  void initTestCase() {
    syncMod.SetBasicPath("C:/Program Files");
    syncMod.SetSynchronizedToPaths("C:/Users");
    syncMod.m_syncModifyFileSystemSwitch = true;
    syncMod.m_alsoSyncReversebackSwitch = true;
    qDebug("Start SyncModifiyFileSystemTest..."); }
  void cleanupTestCase() { qDebug("End SyncModifiyFileSystemTest..."); }

  void init() {
  }
  void cleanup() {}

  void test_syncSwitchOff() {
    syncMod.m_syncModifyFileSystemSwitch = false;
    syncMod.m_alsoSyncReversebackSwitch = true;
    // rename: path/oldItemName => path/newItemName
    QString path = "C:/Program Files";
    QString oldItemName = "old.txt";
    QString newItemName = "new.txt";
    QVERIFY2(syncMod(path) == false, "switch is off, no sync");
    QCOMPARE(path, "C:/Program Files");
  }

  void test_renameSyncBack() {
    syncMod.m_syncModifyFileSystemSwitch = true;
    syncMod.m_alsoSyncReversebackSwitch = true;
    // rename: path1/oldItemName => path2/newItemName
    QString path1 = "D:/home/to/randompath";
    QString path2 = "D:/home";
    QString oldItemName = "old.txt";
    QString newItemName = "new.txt";
    QVERIFY2(syncMod(path1) == false, "should no need to sync");

    path1 = "C:/Program Files";
    path2 = "D:/home";
    QVERIFY2(syncMod(path1) == true, "should sync");
    QCOMPARE(path1, "C:/Users");

    path1 = "C:/Users";
    QVERIFY2(syncMod(path1) == true, "should sync");
    QCOMPARE(path1, "C:/Program Files");
  }

  void test_renameNoSyncBack() {
    syncMod.m_syncModifyFileSystemSwitch = true;
    syncMod.m_alsoSyncReversebackSwitch = false;
    // rename: path/oldItemName => path/newItemName
    QString path = "C:/Program Files";
    QString oldItemName = "old.txt";
    QString newItemName = "new.txt";

    QVERIFY2(syncMod(path) == true, "should sync");
    QCOMPARE(path, "C:/Users");

    path = "C:/Users";
    QVERIFY2(syncMod(path) == false, "no sync back");
    QCOMPARE(path, "C:/Users");
  }
};

QTEST_MAIN(SyncModifiyFileSystemTest)
#include "SyncModifiyFileSystemTest.moc"
