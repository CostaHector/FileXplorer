#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"
#include "MountHelper.h"

using namespace MountHelper;

class MountHelperTest : public PlainTestSuite {
  Q_OBJECT
 public:
  MountHelperTest() : PlainTestSuite{} {}
 private slots:

  void mountPath_tableName_bidirection_conversion_ok() {
    using namespace MountPathTableNameMapper;
    QCOMPARE(toTableName(toMountPath("C__home_DISKS_DD2T4")), "C__home_DISKS_DD2T4");
    QCOMPARE(toTableName(toMountPath("_mnt_DISKS_DD2T4")), "_mnt_DISKS_DD2T4");

    QCOMPARE(toMountPath(toTableName("C:/home/DISKS/DD2T4")), "C:/home/DISKS/DD2T4");
    QCOMPARE(toMountPath(toTableName("/mnt/DISKS/DD2T4")), "/mnt/DISKS/DD2T4");

    bool ok = false;
    QCOMPARE(toTableName("C:/home/DISKS/DD2T4", &ok), "C__home_DISKS_DD2T4");
    QVERIFY(ok);

    QCOMPARE(toMountPath("C__home_DISKS_DD2T4", &ok), "C:/home/DISKS/DD2T4");
    QVERIFY(ok);

    QCOMPARE(toTableName("/mnt/DISKS/DD2T4", &ok), "_mnt_DISKS_DD2T4");
    QVERIFY(ok);

    QCOMPARE(toTableName("/mnt/DISKS/DD2", &ok), "_mnt_DISKS_DD2");
    QVERIFY(ok);

    QCOMPARE(toMountPath("_mnt_DISKS_DD2", &ok), "/mnt/DISKS/DD2");
    QVERIFY(ok);

    // invalid below
    QCOMPARE(toMountPath("invalid_name", &ok), "");
    QVERIFY(!ok);
    QCOMPARE(toMountPath("D:/home/DISKS/DD2T4", &ok), "");
    QVERIFY(!ok);

    QCOMPARE(toTableName("invalid_name", &ok), "");
    QVERIFY(!ok);
    QCOMPARE(toTableName("D__home_DISKS_DD2T4", &ok), "");
    QVERIFY(!ok);

    const QStringList tablesNamesFromMountPath = CandidateTableNamesList();
  }
};

#include "MountHelperTest.moc"
REGISTER_TEST(MountHelperTest, false)
