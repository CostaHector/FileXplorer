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

#ifdef _WIN32
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
          "device:%s, name:%s, displayName:%s, rootPath:%s, "  //
          "AvailSize/TotalSize:%lld/%lld",                     //
          qPrintable(storageInfo.device()),                    //
          qPrintable(storageInfo.name()),                      // volume label, same as rootPath if not set
          qPrintable(storageInfo.displayName()),               // volume label, same as rootPath if not set
          qPrintable(storageInfo.rootPath()),                  // e.g. C:/
          storageInfo.bytesAvailable(),                        //
          storageInfo.bytesTotal());                           //
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
    for (const QString& guid : guids) {
      QVERIFY2(isVolumeAvailable(guid), qPrintable(guid));
    }
  }

  void test_MountVolume() {
    if (!IsAdministrator()) {
      qWarning("Not in Administrator, volume mount will failed because of ACCESS_DENIED(-5)");
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

  void test_isVolumeOnline_false() {
    const QString randomGuidNotExistVolume{"randomGuidNotExistVolume"};
    QVERIFY2(!isVolumeAvailable(randomGuidNotExistVolume), qPrintable(randomGuidNotExistVolume));
  }

  void test_Volumes2ContainedMountPnts() {
    // for example:
    // in guidC[C:/DISK], there are many mount point "C:/mnt/pnt1", "C:/mnt/pnt2"
    // volName2Lst[guidC] = {"mnt/pnt1", "mnt/pnt2"}
    // mount point here is not absolute but relative
    const QMap<QString, QSet<QString>>& vol2ContainedPnts = Volumes2ContainedMountPnts();
    QVERIFY(!vol2ContainedPnts.isEmpty());
    QMapIterator<QString, QSet<QString>> it{vol2ContainedPnts};
    while (it.hasNext()) {
      it.next();
      const QString volume = it.key();
      const QSet<QString>& pntSet = it.value();
      const QStringList pnts{pntSet.cbegin(), pntSet.cend()};
      const QString& pntStr = pnts.join(", ");
      qDebug("volume:%s contains %d mount point(s) which are: [%s]", qPrintable(volume), pnts.size(), qPrintable(pntStr));
    }
  }

  void test_GetMountPointsByVolumeName_specified_rootPath() {
    const QList<QStorageInfo>& lst = QStorageInfo::mountedVolumes();
    for (const QStorageInfo& si : lst) {
      const QString& rp = QDir::toNativeSeparators(si.rootPath());  // L"C:\\"
      const QSet<QString>& pntSet = GetMountPointsByVolumeName(rp.toStdWString().c_str());
      const QStringList pnts{pntSet.cbegin(), pntSet.cend()};
      const QString& pntStr = pnts.join(", ");
      const QString& volume = QString::fromUtf8(si.device());
      qDebug("volume:%s root path:%s contains %d mount point(s) which are: [%s]",  //
             qPrintable(volume), qPrintable(rp), pnts.size(), qPrintable(pntStr));
    }
  }

  void test_Guids2MountPoint() {
    const auto& gui2Pnts = Guids2MntPntSet(true);
    QVERIFY2(!gui2Pnts.isEmpty(), "Recheck if no volumes at all");
  }

  void test_GetGuidJoinDisplayName() {
    // {"guid_1|root path 1", "guid_2_root path 2"}
    const QStringList& guidDispLst = GetGuidJoinDisplayName();
    // {{"guid_1", "root path 1"}, {"guid_2", "root path 2"}}
    const QMap<QString, QString>& guidTblName2Disp = GetGuidTableName2DisplayName();
    QCOMPARE(guidDispLst.size(), guidTblName2Disp.size());
    for (const QString& guidDisp : guidDispLst) {
      const QString& guid = ChoppedDisplayName(guidDisp);
      const QString& rootpath = GetDisplayNameByGuidTableName(guid);
      QCOMPARE(guidTblName2Disp[guid], rootpath);
    }
    qDebug("\n%s", qPrintable(guidDispLst.join('\n')));
  }

  void test_ChoppedDisplayName() {
    QCOMPARE(ChoppedDisplayName("MOVIES|Display name not found"), "MOVIES");
    QCOMPARE(ChoppedDisplayName("MOVIES ANYTHING HELLO WORLD"), "MOVIES ANYTHING HELLO WORLD");
    QCOMPARE(ChoppedDisplayName("0123456789_0123456789_0123456789_012|C:\\"), "0123456789_0123456789_0123456789_012");
  }

#endif
};

#include "MountHelperTest.moc"
REGISTER_TEST(MountHelperTest, false)
