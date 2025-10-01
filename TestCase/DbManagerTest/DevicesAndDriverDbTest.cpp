#include <QCoreApplication>
#include <QtTest>
#include <QSqlRecord>
#include <QStorageInfo>

#include "PlainTestSuite.h"
#include "DevicesAndDriverDb.h"
#include "TableFields.h"
#include "PublicVariable.h"
#include "PublicMacro.h"
#include "MountHelper.h"

#ifdef _WIN32
const QString rootpath = QFileInfo(__FILE__).absolutePath();
const QString dbName = rootpath + "/DeviceAndDriver.db";

QList<VolumeInfo> MockerGetVolumesInfoEmpty() {
  return {};
}

QList<VolumeInfo> MockerGetVolumesInfoCDE() {
  QList<VolumeInfo> ans;
  ans << VolumeInfo{"C:/", "Volume C", 1024, 1024, "guidC"};
  ans << VolumeInfo{"D:/", "Volume D", 512, 1024, "guidD"};
  ans << VolumeInfo{"E:/", "Volume E", 256, 1024, "guidE"};
  return ans;
}

QList<VolumeInfo> MockerGetVolumesInfoCX() {
  QList<VolumeInfo> ans;
  ans << VolumeInfo{"C:/", "Volume C", 1024, 1024, "guidC"};
  ans << VolumeInfo{"X:/", "Volume X", 1024, 10240, "guidX"};
  return ans;
}

class DevicesAndDriverDbTest : public PlainTestSuite {
  Q_OBJECT
 public:
  DevicesAndDriverDbTest() : PlainTestSuite{} {}
 private slots:
  void cleanup() {
    // post-condition
    if (QFile{dbName}.exists()) {
      QFile{dbName}.remove();
    }
  }

  void test_table_Init_ok() {
    // precondition
    QVERIFY(!QFile{dbName}.exists());
    const QList<QStorageInfo>& siLst = QStorageInfo::mountedVolumes();
    QVERIFY(siLst.size() > 0);
    QVERIFY(siLst.size() < 10);
    // procedure
    DevicesAndDriverDb mDb{dbName, "CONN_DEVICE_DRIVER_DB_TEST"};
    QVERIFY(mDb.CreateDatabase());
    QVERIFY(mDb.CreateTable(DB_TABLE::DISKS, DevicesAndDriverDb::CREATE_DEV_DRV_TEMPLATE));
    QVERIFY(QFile{dbName}.exists());
    QCOMPARE(mDb.CountRow(DB_TABLE::DISKS), 0);  // empty table

    const int drvCnt = mDb.InitDeviceAndDriver(DB_TABLE::DISKS);
    QCOMPARE(drvCnt, siLst.size());
    QCOMPARE(mDb.CountRow(DB_TABLE::DISKS), drvCnt);  // not empty
  }

  void test_table_full_insert_ok_full_deleted_ok() {
    // precondition
    QVERIFY(!QFile{dbName}.exists());
    const QList<QStorageInfo>& siLst = QStorageInfo::mountedVolumes();
    QVERIFY(siLst.size() > 0);
    // procedure
    DevicesAndDriverDb mDb{dbName, "CONN_DEVICE_DRIVER_DB_TEST"};
    QVERIFY(mDb.CreateDatabase());
    QVERIFY(mDb.CreateTable(DB_TABLE::DISKS, DevicesAndDriverDb::CREATE_DEV_DRV_TEMPLATE));
    QVERIFY(QFile{dbName}.exists());
    QCOMPARE(mDb.CountRow(DB_TABLE::DISKS), 0);  // empty table

    VolumeUpdateResult volUpdRet{0};
    QCOMPARE(mDb.AdtDeviceAndDriver(DB_TABLE::DISKS, &volUpdRet, ::GetVolumesInfo), FD_OK);
    QCOMPARE(volUpdRet.insertCnt, siLst.size());  // all volume inserted
    QCOMPARE(volUpdRet.deleteCnt, 0);             // nothing deleted
    QCOMPARE(volUpdRet.updateCnt, 0);             // nothing update
    QCOMPARE(mDb.CountRow(DB_TABLE::DISKS), siLst.size());

    volUpdRet.Init();
    QCOMPARE(mDb.AdtDeviceAndDriver(DB_TABLE::DISKS, &volUpdRet, ::MockerGetVolumesInfoEmpty), FD_OK);
    QCOMPARE(volUpdRet.insertCnt, 0);             // nothing insert
    QCOMPARE(volUpdRet.deleteCnt, siLst.size());  // all volume delete
    QCOMPARE(volUpdRet.updateCnt, 0);             // nothing update
    QCOMPARE(mDb.CountRow(DB_TABLE::DISKS), 0);   // empty table
  }

  void test_table_part_insert_ok_part_deleted_ok() {
    // precondition
    QVERIFY(!QFile{dbName}.exists());
    // procedure
    DevicesAndDriverDb mDb{dbName, "CONN_DEVICE_DRIVER_DB_TEST"};
    QVERIFY(mDb.CreateDatabase());
    QVERIFY(mDb.CreateTable(DB_TABLE::DISKS, DevicesAndDriverDb::CREATE_DEV_DRV_TEMPLATE));
    QVERIFY(QFile{dbName}.exists());
    QCOMPARE(mDb.CountRow(DB_TABLE::DISKS), 0);  // empty volumes table

    VolumeUpdateResult volUpdRet{0};
    QCOMPARE(mDb.AdtDeviceAndDriver(DB_TABLE::DISKS, &volUpdRet, ::MockerGetVolumesInfoCDE), FD_OK);
    QCOMPARE(volUpdRet.insertCnt, 3);  // C D E volumes inserted
    QCOMPARE(volUpdRet.deleteCnt, 0);
    QCOMPARE(volUpdRet.updateCnt, 0);
    QCOMPARE(mDb.CountRow(DB_TABLE::DISKS), 3);

    volUpdRet.Init();
    QCOMPARE(mDb.AdtDeviceAndDriver(DB_TABLE::DISKS, &volUpdRet, ::MockerGetVolumesInfoCX), FD_OK);
    QCOMPARE(volUpdRet.insertCnt, 1);  // X volumes inserted
    QCOMPARE(volUpdRet.deleteCnt, 2);  // D E volumes deleted
    QCOMPARE(volUpdRet.updateCnt, 1);  // C volumes inserted
    QCOMPARE(mDb.CountRow(DB_TABLE::DISKS), 2);
  }

  void test_SetDuration_ok() {
    // precondition
    QVERIFY(!QFile{dbName}.exists());
    // procedure
    DevicesAndDriverDb mDb{dbName, "CONN_DEVICE_DRIVER_DB_TEST"};
    QVERIFY(mDb.CreateDatabase());
    QVERIFY(mDb.CreateTable(DB_TABLE::DISKS, DevicesAndDriverDb::CREATE_DEV_DRV_TEMPLATE));
    QVERIFY(QFile{dbName}.exists());
    QCOMPARE(mDb.CountRow(DB_TABLE::DISKS), 0);  // empty volumes table

    VolumeUpdateResult volUpdRet{0};
    QCOMPARE(mDb.AdtDeviceAndDriver(DB_TABLE::DISKS, &volUpdRet, ::MockerGetVolumesInfoCDE), FD_OK);
    QCOMPARE(volUpdRet.insertCnt, 3);  // C D E volumes inserted
    QCOMPARE(volUpdRet.deleteCnt, 0);
    QCOMPARE(volUpdRet.updateCnt, 0);
    QCOMPARE(mDb.CountRow(DB_TABLE::DISKS), 3);

    QCOMPARE(mDb.UpdateAdtTime(DB_TABLE::DISKS, "guidC", 9999), FD_OK);
    QCOMPARE(mDb.UpdateMountedPath(DB_TABLE::DISKS, "guidC", "C:/DISK/mnt"), FD_OK);

    using namespace DEV_DRV_TABLE;
    QSet<QString> actualMountPoint;
    QSet<int> actualAdtTimes;
    mDb.QueryPK(DB_TABLE::DISKS, ENUM_2_STR(MOUNT_POINT), actualMountPoint);
    mDb.QueryPK(DB_TABLE::DISKS, ENUM_2_STR(ADT_TIME), actualAdtTimes);
    QSet<QString> expectMountPoint{"C:/DISK/mnt", "", ""};
    QSet<int> expectAdtTimes{9999, 0, 0};
    QCOMPARE(actualMountPoint, expectMountPoint);
    QCOMPARE(actualAdtTimes, expectAdtTimes);
  }
};

#include "DevicesAndDriverDbTest.moc"
REGISTER_TEST(DevicesAndDriverDbTest, false)

#endif
