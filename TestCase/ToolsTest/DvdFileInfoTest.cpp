#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "DvdFileInfo.h"
#include "TDir.h"
#include "VideoDurationGetter.h"
#include "MD5Calculator.h"
#include "VideoDurationGetterMock.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

    class DvdFileInfoTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir mTDir;
  qint64 mExpectTotalSize{0};
  int mExpectTotalDuration{0};
  QString mNewContentsInVobFile{"11111 11111"}; // 11
  int mExpectTotalSizeNew{0};

private slots:
  void initTestCase() {
    QVERIFY(mTDir.IsValid());
    QList<FsNodeEntry> nodes {
                             {"Movie 2026 DVD - Marvel Production/VIDEO_TS/VTS_01_0.IFO", false, ""},
                             {"Movie 2026 DVD - Marvel Production/VIDEO_TS/VTS_01_0.VOB", false, "1"},
                             {"Movie 2026 DVD - Marvel Production/VIDEO_TS/VTS_01_1.VOB", false, "22"},
                             {"Movie 2026 DVD - Marvel Production/VIDEO_TS/VTS_02_0.IFO", false, ""},
                             {"Movie 2026 DVD - Marvel Production/VIDEO_TS/VTS_02_0.VOB", false, "333"},
                             {"Movie 2026 DVD - Marvel Production/VIDEO_TS/VTS_02_1.VOB", false, "4444"},
                             {"Movie 2025 DVD - Incomplete Broken/VIDEO_TS", true, ""},
                             {"Movie 2024 - Not a dvd at all.mp4", false, "55555"},
                             };
    mExpectTotalSize = (QString{} + "1" + "22" + "333" + "4444").size();
    mExpectTotalSizeNew = (QString{} + mNewContentsInVobFile.size() + "22" + "333" + "4444").size();
    QCOMPARE(mTDir.createEntries(nodes), nodes.size());
    GlobalMockObject::reset();

    QHash<QString, int> vidPath2Duration {
                                         {mTDir.itemPath("Movie 2026 DVD - Marvel Production/VIDEO_TS/VTS_01_0.VOB"), 1000},
                                         {mTDir.itemPath("Movie 2026 DVD - Marvel Production/VIDEO_TS/VTS_01_1.VOB"), 2000},
                                         {mTDir.itemPath("Movie 2026 DVD - Marvel Production/VIDEO_TS/VTS_02_0.VOB"), 3000},
                                         {mTDir.itemPath("Movie 2026 DVD - Marvel Production/VIDEO_TS/VTS_02_1.VOB"), 4000},
                                         {mTDir.itemPath("Movie 2024 - Not a dvd at all.mp4"), 5000},
                                         };
    mExpectTotalDuration = 1000 + 2000 + 3000 + 4000;

    MOCKER(VideoDurationGetter::GetLengthQuickStatic).stubs().will(invoke(VideoDurationGetterMock::invokeGetLengthQuickStatic));
    VideoDurationGetterMock::PresetVidsDuration(vidPath2Duration);
  }

  void cleanupTestCase() {
    VideoDurationGetterMock::PresetVidsDuration({});
    GlobalMockObject::verify();
  }

  void GenerateDvdFile_empty_folder_inexist_folder_skip_ok() {
    // empty folder
    QVERIFY(!DvdFileInfo::GenerateDvdFile(mTDir.itemPath("Movie 2025 DVD - Incomplete Broken/VIDEO_TS"), true));
    QVERIFY(!mTDir.fileExists("Movie 2026 DVD - Marvel Production/Movie 2025 DVD - Incomplete Broken.dvd"));
    // not dvd at all
    QVERIFY(!DvdFileInfo::GenerateDvdFile(mTDir.itemPath("Movie 2024 - Not a dvd at all.mp4"), true));
    QVERIFY(!mTDir.fileExists("Movie 2024 - Not a dvd at all.mp4.dvd"));
    QVERIFY(!mTDir.fileExists("Movie 2024 - Not a dvd at all.dvd"));
  }

  void GenerateDvdFile_ok() {
    QVERIFY(DvdFileInfo::GenerateDvdFile(mTDir.itemPath("Movie 2026 DVD - Marvel Production/VIDEO_TS"), false));
    QVERIFY(mTDir.fileExists("Movie 2026 DVD - Marvel Production/Movie 2026 DVD - Marvel Production.dvd"));

    const QString dvdFilePath{mTDir.itemPath("Movie 2026 DVD - Marvel Production/Movie 2026 DVD - Marvel Production.dvd")};
    QCOMPARE(DvdFileInfo::ReadTotalDurationFromDvdFile(dvdFilePath), mExpectTotalDuration);
    QCOMPARE(DvdFileInfo::ReadTotalFileSizeFromDvdFile(dvdFilePath), mExpectTotalSize);
    QCOMPARE(DvdFileInfo::ReadTotalMD5FromDvdFile(dvdFilePath).isEmpty(), false); // should not empty

    // no override mode: when already exist. skip
    QVERIFY(mTDir.touch("Movie 2025 DVD - Incomplete Broken/VIDEO_TS/VTS_01_0.VOB", "11111 11111")); // 11
    QVERIFY(!DvdFileInfo::GenerateDvdFile(mTDir.itemPath("Movie 2026 DVD - Marvel Production/VIDEO_TS"), false));
    QCOMPARE(DvdFileInfo::ReadTotalFileSizeFromDvdFile(dvdFilePath), mExpectTotalSize); // still not update

    // override mode: when already exist. override it
    QVERIFY(DvdFileInfo::GenerateDvdFile(mTDir.itemPath("Movie 2026 DVD - Marvel Production/VIDEO_TS"), true));
    QCOMPARE(DvdFileInfo::ReadTotalFileSizeFromDvdFile(dvdFilePath), mExpectTotalSizeNew); // update succeed
  }

};

#include "DvdFileInfoTest.moc"
REGISTER_TEST(DvdFileInfoTest, false)