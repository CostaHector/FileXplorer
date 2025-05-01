#include <QCoreApplication>
#include <QtTest>
#include "TestCase/pub/MyTestSuite.h"
#include "Tools/FileDescriptor/MountHelper.h"

using namespace MountHelper;

class MountHelperTest : public MyTestSuite {
  Q_OBJECT
 public:
  MountHelperTest() : MyTestSuite{false} {}
 private slots:

#ifdef WIN32
  void test_getGuid2Label() {
    const auto& guid2Label = GetGuid2LabelMap();
    QVERIFY2(guid2Label.size() > 0, "at least 1 volume find");
    QSet<QString> guids;
    for (auto guidIt = guid2Label.cbegin(); guidIt != guid2Label.cend(); ++guidIt) {
      const QString& guid = guidIt.key();
      guids.insert(guid);
      qDebug("guid: %s, label: %s", qPrintable(guid), qPrintable(guidIt.value()));
    }
  }

  void test_drives_storageInfo() {
    QSet<QString> drvs, rootPaths;  // C:/ D:/ E:/
    for (const auto& d : QDir::drives()) {
      drvs.insert(d.filePath());
    }
    const auto& mountedVolLst = QStorageInfo::mountedVolumes();
    for (const auto& storageInfo : mountedVolLst) {
      rootPaths.insert(storageInfo.rootPath());
      qDebug(
          "name:%s, displayName:%s, rootPath:%s, "  //
          "AvailSize/TotalSize:%lld/%lld",          //
          qPrintable(storageInfo.name()),           // volume label, same as rootPath if not set
          qPrintable(storageInfo.displayName()),    // volume label, same as rootPath if not set
          qPrintable(storageInfo.rootPath()),       // e.g. C:/
          storageInfo.bytesAvailable(),             //
          storageInfo.bytesTotal());                //
    }
    QCOMPARE(drvs, rootPaths);
    QVERIFY(drvs.size() > 0);
  }

  void test_GetVolumeInfo() {
    QFileInfoList drvFis{QDir::drives()};
    QVERIFY(drvFis.size() > 0);

    QSet<QString> expectGuids;
    const auto& guid2Label = GetGuid2LabelMap();
    for (auto it = guid2Label.cbegin(); it != guid2Label.cend(); ++it) {
      expectGuids.insert(it.key());
    }
    QVERIFY(expectGuids.size() > 0);

    QSet<QString> guids;
    for (const auto& d : drvFis) {
      const QString drv = d.filePath();
      QString guid;
      QVERIFY(GetGuidByDrive(drv, guid));
      QVERIFY(expectGuids.contains(guid));

      guids.insert(guid);
    }
    QVERIFY(guids.size() > 0);
  }

  void test_MountVolume() {
    if (!IsAdministrator()) {
      qCritical("Not in Administrator, volume mount will failed because of ACCESS_DENIED(-5)");
      return;
    }

    const auto& guid2Label = GetGuid2LabelMap();
    QVERIFY2(guid2Label.size() > 0, "at least 1 volume find");
    QMapIterator<QString, QString> it{guid2Label};
    const QString volLabelTemplate{"label-%1"};
    int i = 0;
    while (it.hasNext()) {
      it.next();
      const QString& guid = it.key();
      QString volMountPoint;
      QVERIFY(MountVolume(guid, volLabelTemplate.arg(i), volMountPoint));
      QVERIFY(UnmountVolume(volMountPoint));
      ++i;
    }
  }
#endif
};

MountHelperTest g_MountHelperTest;
#include "MountHelperTest.moc"
