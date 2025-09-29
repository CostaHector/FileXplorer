#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "DevicesDriveModel.h"
#include "DevicesAndDriverDb.h"
#include "EndToExposePrivateMember.h"
#include "TDir.h"
#include "MountHelper.h"
#include "TableFields.h"

class DevicesDriveModelTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() {}

  void cleanupTestCase() {}

  void defaultConstructor_ok() {
    DevicesDriveModel ddm;
    // call memeber should not crash down
    QCOMPARE(ddm.rowCount(), 0);
    QCOMPARE(ddm.columnCount(), 0);
    QVERIFY(ddm.GetRootPath(QModelIndex()).isEmpty());
    QVERIFY(ddm.GetGuid(QModelIndex()).isEmpty());
    QVERIFY(ddm.GetMountedPoint(QModelIndex()).isEmpty());

    QVERIFY(ddm.data(QModelIndex(), Qt::ItemDataRole::DisplayRole).isNull());
    QCOMPARE(ddm.headerData(0, Qt::Orientation::Vertical, Qt::ItemDataRole::DisplayRole).toInt(), 0 + 1);
    QCOMPARE(ddm.headerData(1, Qt::Orientation::Vertical, Qt::ItemDataRole::DisplayRole).toInt(), 1 + 1);
    QCOMPARE(ddm.headerData(0, Qt::Orientation::Vertical, Qt::ItemDataRole::TextAlignmentRole).toInt(), (int)Qt::AlignRight);
  }

  void data_retrieve_ok() {
    TDir tDir;
    QVERIFY(tDir.IsValid());
    const QString dbName{tDir.itemPath("DeviceAndDriverTest.db")};
    const QString connName{"DeviceAndDriverTestConn"};
    const QString tableName{"ArielComputerTable"};

    DevicesAndDriverDb ddb{dbName, connName};
    QVERIFY(ddb.IsValid());
    QVERIFY(ddb.CreateTable(tableName, DevicesAndDriverDb::CREATE_DEV_DRV_TEMPLATE));
    QVERIFY(ddb.IsTableExist(tableName));
    QVERIFY(ddb.IsTableEmpty(tableName));

    // empty table check data
    DevicesDriveModel ddm{nullptr, ddb.GetDb()};
    ddm.setTable(tableName);
    ddm.select();
    QCOMPARE(ddm.rowCount(), 0);

    MountHelper::MockGuids2MntPntSet().clear();  // no guid 2 pnt
    MockGetVolumesInfo().clear();                // no volumes
    QCOMPARE(ddb.InitDeviceAndDriver(tableName), 0);

    MountHelper::GUID_2_PNTS_SET guid2PointsPath{
        {"12345678-1234-1234-1234-1234567890AB", {"C:/DISKS/Segate2020", "C:/DISKS/Segate2000"}},
        {"ABCDEF12-3456-7890-ABCDEF1234567890", {"C:/DISKS/WestData2020", "C:/DISKS/WestData2000", "C:/DISKS/WestData1980"}},
    };
    MountHelper::MockGuids2MntPntSet() = guid2PointsPath;

    VolumeInfo systemDrive{"C:/", "System", 900, 800, "12345678-1234-1234-1234-1234567890AB"};
    VolumeInfo dataDrive{"D:/", "Data", 500, 300, "ABCDEF12-3456-7890-ABCDEF1234567890"};
    QList<VolumeInfo> volumesInfoList{systemDrive, dataDrive};
    MockGetVolumesInfo() = volumesInfoList;
    QCOMPARE(volumesInfoList.size(), 2);
    QCOMPARE(ddb.InitDeviceAndDriver(tableName), 2);

    // row count changed ok
    using namespace DEV_DRV_TABLE;
    {
      ddm.select();
      QCOMPARE(ddm.rowCount(), 2);
      QCOMPARE(ddm.data(ddm.index(0, FIELD_E::ROOT_PATH), Qt::DisplayRole).toString(), "C:/");
      QCOMPARE(ddm.data(ddm.index(0, FIELD_E::VOLUME_LABEL), Qt::DisplayRole).toString(), "System");
      QCOMPARE(ddm.data(ddm.index(0, FIELD_E::TOTAL_BYTES), Qt::DisplayRole).toString(), "0'0'0'900");
      QCOMPARE(ddm.data(ddm.index(0, FIELD_E::AVAIL_BYTES), Qt::DisplayRole).toString(), "0'0'0'800");
      QCOMPARE(ddm.data(ddm.index(0, FIELD_E::GUID), Qt::DisplayRole).toString(), "12345678-1234-1234-1234-1234567890AB");  // GUID_LEN 36 char
      QCOMPARE(ddm.data(ddm.index(0, FIELD_E::MOUNT_POINT), Qt::DisplayRole).toString(), "C:/DISKS/Segate2000\nC:/DISKS/Segate2020");  // sorted
      QCOMPARE(ddm.data(ddm.index(0, FIELD_E::ADT_TIME), Qt::DisplayRole).toInt(), 0);  // ADT_TIME field default value
      QCOMPARE(ddm.GetRootPath(ddm.index(0,0)), "C:/");
      QCOMPARE(ddm.GetGuid(ddm.index(0,0)), "12345678-1234-1234-1234-1234567890AB");
      QCOMPARE(ddm.GetMountedPoint(ddm.index(0,0)), "C:/DISKS/Segate2000\nC:/DISKS/Segate2020");

      QCOMPARE(ddm.data(ddm.index(1, FIELD_E::ROOT_PATH), Qt::DisplayRole).toString(), "D:/");
      QCOMPARE(ddm.data(ddm.index(1, FIELD_E::VOLUME_LABEL), Qt::DisplayRole).toString(), "Data");
      QCOMPARE(ddm.data(ddm.index(1, FIELD_E::TOTAL_BYTES), Qt::DisplayRole).toString(), "0'0'0'500");
      QCOMPARE(ddm.data(ddm.index(1, FIELD_E::AVAIL_BYTES), Qt::DisplayRole).toString(), "0'0'0'300");
      QCOMPARE(ddm.data(ddm.index(1, FIELD_E::GUID), Qt::DisplayRole).toString(), "ABCDEF12-3456-7890-ABCDEF1234567890");  // GUID_LEN 36 char
      QCOMPARE(ddm.data(ddm.index(1, FIELD_E::MOUNT_POINT), Qt::DisplayRole).toString(),
               "C:/DISKS/WestData1980\nC:/DISKS/WestData2000\nC:/DISKS/WestData2020");
      QCOMPARE(ddm.data(ddm.index(1, FIELD_E::ADT_TIME), Qt::DisplayRole).toInt(), 0);  // ADT_TIME field default value

      const QVariant& iconFromAppStyle = ddm.data(ddm.index(0, FIELD_E::ROOT_PATH), Qt::DecorationRole);
      QVERIFY(iconFromAppStyle.isValid());
      QVERIFY(iconFromAppStyle.canConvert<QIcon>());
    }
  }
};

#include "DevicesDriveModelTest.moc"
REGISTER_TEST(DevicesDriveModelTest, false)
