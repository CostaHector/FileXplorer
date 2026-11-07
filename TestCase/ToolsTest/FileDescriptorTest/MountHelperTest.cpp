#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"
#include "MountHelper.h"

class MountHelperTest : public PlainTestSuite {
  Q_OBJECT
public:
  using PlainTestSuite::PlainTestSuite;

private slots:

  void mountPath_tableName_bidirection_conversion_ok() {
    using namespace MountPathTableNameMapper;
    {
      bool bIsValid = false;
      QCOMPARE(toTableName(toMountPath("C__DISKS_DD2T4", bIsValid)), "C__DISKS_DD2T4");
      QCOMPARE(toTableName(toMountPath("_mnt_DISKS_DD2T4", bIsValid)), "_mnt_DISKS_DD2T4");

      QCOMPARE(toMountPath(toTableName("C:/DISKS/DD2T4"), bIsValid), "C:/DISKS/DD2T4");
      QCOMPARE(toMountPath(toTableName("/mnt/DISKS/DD2T4"), bIsValid), "/mnt/DISKS/DD2T4");
    }

    {
      // windows platform
      bool ok = false;
      QCOMPARE(toTableName("C:/DISKS/DD2T4"), "C__DISKS_DD2T4");
      QVERIFY(ok);
      ok = false;

      QCOMPARE(toMountPath("C__DISKS_DD2T4", ok), "C:/DISKS/DD2T4");
      QVERIFY(ok);
      ok = false;

      // 表名不满足正则， 但是满足正常路径，回落成功
      QString okTableNameCode{toTableName("C:/AnyFolder/MoviesFolder")};
      QVERIFY(okTableNameCode.startsWith("MoviesFolder_"));
      QVERIFY(ok);
      ok = false;

      // linux platform
      QCOMPARE(toTableName("/mnt/DISKS/DD2T4"), "_mnt_DISKS_DD2T4");
      QVERIFY(ok);
      ok = false;

      QCOMPARE(toTableName("/mnt/DISKS/DD2"), "_mnt_DISKS_DD2");
      QVERIFY(ok);
      ok = false;

      QCOMPARE(toMountPath("_mnt_DISKS_DD2", ok), "/mnt/DISKS/DD2");
      QVERIFY(ok);
      ok = false;
    }

    {
      // invalid below

      // 表名不满足正则, 失败
      // 表名不含有"_", 回落失败
      bool ok = false;
      QCOMPARE(toMountPath("invalidName", ok), "");
      QVERIFY(!ok);
      ok = false;
      QCOMPARE(toMountPath("D:/DISKS/DD2T4", ok), "");
      QVERIFY(!ok);
      ok = false;
    }

    const QStringList tablesNamesFromMountPath = CandidateTableNamesList();
  }
};

#include "MountHelperTest.moc"
REGISTER_TEST(MountHelperTest, false)
