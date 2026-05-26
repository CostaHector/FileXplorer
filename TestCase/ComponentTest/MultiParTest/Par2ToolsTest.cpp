#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "Par2Tools.h"
#include "MultiParTools.h"
#include "TDir.h"
#include "SystemPath.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

using namespace Par2Tools;

class Par2ToolsTest : public PlainTestSuite {
  Q_OBJECT
public:
  using PlainTestSuite::PlainTestSuite;

private:
  TDir mTDir;
  const QString m_folderPath{mTDir.path()};

  const QString m_oldSrcFileName{"oldSrcFileName.mp4"};
  const QString m_oldSrcFilePath{mTDir.itemPath(m_oldSrcFileName)};
  const QString m_newSrcFileName{"newSrcFileName.mp4"};
  const QString m_newSrcFilePath{mTDir.itemPath(m_newSrcFileName)};

  const QString m_oldPar2FilePath{mTDir.itemPath("oldSrcFileName.mp4.par2")};
  const QString m_newPar2FileName{"newSrcFileName.mp4.par2"};
  const QString m_newPar2FilePath{mTDir.itemPath(m_newPar2FileName)};

private slots:
  void initTestCase() {
    QVERIFY(mTDir.IsValid());
    QVERIFY(mTDir.touch(m_oldSrcFileName, QByteArray{"hello this is a video."}));
  }
  void cleanupTestCase() {}
  void init() { GlobalMockObject::reset(); }
  void cleanup() { GlobalMockObject::verify(); }

  void ChopPostfixVolAndPar2_ok() {
    QCOMPARE(ChopPostfixVolAndPar2("a.par2"), "a");
    QCOMPARE(ChopPostfixVolAndPar2("a.vol10+11.par2"), "a");
    QCOMPARE(ChopPostfixVolAndPar2("a.vol1+9.par2"), "a");
    QCOMPARE(ChopPostfixVolAndPar2("a.vol1+9"), "a.vol1+9");
  }

  void SyncPar2ContentsWithFileName_ok() {
    bool bCreatePar2{false};
    int par2FilesCnt{0};
    std::tie(bCreatePar2, par2FilesCnt) = MultiParTools::CreatePar2({m_oldSrcFilePath}, 10);
#ifndef _WIN32
    QVERIFY(!bCreatePar2);
    QCOMPARE(par2FilesCnt, 0);
    return;
#endif
    QVERIFY(bCreatePar2);
    QCOMPARE(par2FilesCnt, 1);

    const QString m_multiParPath{SystemPath::MULTI_PAR_PROG_PATH()};
    ParVerifyInfomation info = MultiParTools::VerifyAFile(m_multiParPath, m_oldPar2FilePath);
    QVERIFY(info.isTrustable());
    QVERIFY(!info.isNeedRepair());
    QCOMPARE(info.m_volPar2FileNameList.size(), 1);

    QString oldVolPar2FileName{info.m_volPar2FileNameList.front()};
    QString newVolPar2FileName{oldVolPar2FileName};
    newVolPar2FileName.replace("oldSrcFileName.mp4", "newSrcFileName.mp4");

    const QString oldVolPar2FilePath{mTDir.itemPath(oldVolPar2FileName)};
    const QString newVolPar2FilePath{mTDir.itemPath(newVolPar2FileName)};
    // 批量重命名后, SyncBuiltInSrcFileListInPar2 可直接修改.par2文件, 无需重新生成
    QVERIFY(QFile::rename(m_oldSrcFilePath, m_newSrcFilePath));
    QVERIFY(QFile::rename(m_oldPar2FilePath, m_newPar2FilePath));
    QVERIFY(QFile::rename(oldVolPar2FilePath, newVolPar2FilePath));

    info = MultiParTools::VerifyAFile(m_multiParPath, m_newPar2FilePath);
    QCOMPARE(info.m_statusE, ParVerifyInfomation::Par2StatusE::READY_TO_RENAME);
    QCOMPARE(info.m_completeCount, 0);
    QCOMPARE(info.m_misnamedCount, 1);
    QCOMPARE(info.m_damagedCount, 0);
    QCOMPARE(info.m_missingCount, 0);

    const QString par2FileInSync{GetSyncPar2FileAbsPath(m_newPar2FilePath)};
    const QString volPar2FileInSync{GetSyncPar2FileAbsPath(newVolPar2FilePath)};
    QVERIFY(!QFile::exists(par2FileInSync));
    QVERIFY(!QFile::exists(volPar2FileInSync));
    QVERIFY(SyncBuiltInSrcFileListInPar2(m_folderPath, m_newPar2FileName, QStringList{newVolPar2FilePath}));
    QVERIFY(QFile::exists(par2FileInSync));
    QVERIFY(QFile::exists(volPar2FileInSync));

    info = MultiParTools::VerifyAFile(m_multiParPath, par2FileInSync);
    QCOMPARE(info.m_statusE, ParVerifyInfomation::Par2StatusE::ALL_FILES_COMPLETE);
    QCOMPARE(info.m_completeCount, 1);
    QCOMPARE(info.m_misnamedCount, 0);
    QCOMPARE(info.m_damagedCount, 0);
    QCOMPARE(info.m_missingCount, 0);
  }
};

#include "Par2ToolsTest.moc"
REGISTER_TEST(Par2ToolsTest, false)

