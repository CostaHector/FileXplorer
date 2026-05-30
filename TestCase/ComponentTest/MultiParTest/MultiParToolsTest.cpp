#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "MultiParTools.h"
#include "TDir.h"
#include "SystemPath.h"

#include "FileTool.h"
#include "ParVerifyInfomationExamples.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

using namespace MultiParTools;
class MultiParToolsTest : public PlainTestSuite {
  Q_OBJECT
public:
  using PlainTestSuite::PlainTestSuite;

private:
  TDir mTDir;
  const QString m_mediaFileName{"MediaFileNeedBackup.mp4"};
  const QString m_mediaPar2FileName{"MediaFileNeedBackup.mp4.par2"};
  const QString m_mediaFileAbsPath{mTDir.itemPath(m_mediaFileName)};
  const QString m_mediaFileAbsPathNew{mTDir.itemPath("New" + m_mediaFileName)};
  const QString m_mediaPar2FileAbsPath{mTDir.itemPath(m_mediaPar2FileName)};

  const QByteArray m_expectCorrectContent{"0(A%1X@2)-+E3$R%4F5.6j9e8?9*"
                                          "9(B%15X9@78)-45+E62012$R%156575F4421.3.14j2.71e1?0*"
                                          "a(Egsdf1gdf540%bX@client)+-Emyd$R%eyeFlewf.greatjhei?j*klmn"
                                          "o7F(Ano%prankX@quest)-+Er$Rubby%sFeat.ujacketVew?x*yz"
                                          "Ddfadsfg15gdfshb124sf1c54sadgd987re74y 4t1uyhe1zdsbvf4ZSv"
                                          "fsd1gf04d6h6g565780 wAE4 XC1W1ETG0 VQ6A4W7 0"
                                          "324.sdfZ TF7234510rwetyg443q4514rwet16u45 n0667r1i71yif6klfyjcg4057zre"
                                          "45sfgd45zdf1ghbzngf66dx16bn1354dxtytfx32 1h142a3z1zXC100"
                                          "sdfvc,cxv bz,kzoixpolweat gredrm,l.xim,iiunnsdilkyu"
                                          "zcxcietwsupnxfbvzvnj bzi jPASNZfszgWO145147SdfMEDNearfMH BQKWIKE4GUJONDZFG54"
                                          "SFDGUJIASDETFILNFNHIWERYTIUNQWMNJZSLD.M.OUPYHUOTUOICRYIRCYDIYCDRIYRRIYUINPPINU[JNPJINJIPBVCRRXC"
                                          "IOUJQAIUWREIUYSRGOIZNLIYU7T8565867RDTCCV97TRXEXE587X6EITCYVGUVNIYHINJOMIJNUYHVTRYCTERX"
                                          "sdgfnuiipsoinunpuinjGYVRYVRUTCURTCUBYIKLPIUNPOIJPOIUJYU579856R48E4D6R8CV8RBUNJIMOI- OJIP JMYUB7854"
                                          "457418E5784G14XZDHGBF14XFHG411NMHGXFJC187514114gszr78wreweSZSD"
                                          "oujipiupyigouvftctofoidrycoitrfygobuo[hjii]9[9u7h86-ygtouyotgtugf750ft95fd95e4d9d5tofvyou"
                                          "ygyethrg7418ht4zdsb15154xfjhng71ucgkm78474vhigl187vhbjil.197184;/pkn417pmlk817"

  };
  const QByteArray m_corruptContent{"000000-+E3$R%4F5.6j9e8?9*"
                                    "9(B%15X9@78)-45+E62012$R%156575F4421.3.14j2.71e1?0*"
                                    "a(Egsdf1gdf540%bX@client)+-Emyd$R%eyeFlewf.greatjhei?j*klmn"
                                    "o7F(Ano%prankX@quest)-+Er$Rubby%sFeat.ujacketVew?x*yz"
                                    "Ddfadsfg15gdfshb124sf1c54sadgd987re74y 4t1uyhe1zdsbvf4ZSv"
                                    "fsd1gf04d6h6g565780 wAE4 XC1W1ETG0 VQ6A4W7 0"
                                    "324.sdfZ TF7234510rwetyg443q4514rwet16u45 n0667r1i71yif6klfyjcg4057zre"
                                    "45sfgd45zdf1ghbzngf66dx16bn1354dxtytfx32 1h142a3z1zXC100"
                                    "sdfvc,cxv bz,kzoixpolweat gredrm,l.xim,iiunnsdilkyu"
                                    "zcxcietwsupnxfbvzvnj bzi jPASNZfszgWO145147SdfMEDNearfMH BQKWIKE4GUJONDZFG54"
                                    "SFDGUJIASDETFILNFNHIWERYTIUNQWMNJZSLD.M.OUPYHUOTUOICRYIRCYDIYCDRIYRRIYUINPPINU[JNPJINJIPBVCRRXC"
                                    "IOUJQAIUWREIUYSRGOIZNLIYU7T8565867RDTCCV97TRXEXE587X6EITCYVGUVNIYHINJOMIJNUYHVTRYCTERX"
                                    "sdgfnuiipsoinunpuinjGYVRYVRUTCURTCUBYIKLPIUNPOIJPOIUJYU579856R48E4D6R8CV8RBUNJIMOI- OJIP JMYUB7854"
                                    "457418E5784G14XZDHGBF14XFHG411NMHGXFJC187514114gszr78wreweSZSD"
                                    "oujipiupyigouvftctofoidrycoitrfygobuo[hjii]9[9u7h86-ygtouyotgtugf750ft95fd95e4d9d5tofvyou"
                                    "ygyethrg7418ht4zdsb15154xfjhng71ucgkm78474vhigl187vhbjil.197184;/pkn417pmlk817"};

  const QString m_multiParPath{SystemPath::MULTI_PAR_PROG_PATH()};
private slots:
  void initTestCase() {
    QVERIFY(mTDir.IsValid());
    QVERIFY(mTDir.touch(m_mediaFileName, m_expectCorrectContent));

#ifndef _WIN32
    QVERIFY(m_multiParPath.isEmpty());
    QVERIFY(!QFile::exists(m_multiParPath));
#else
    QVERIFY(!m_multiParPath.isEmpty());
    QVERIFY(QFile::exists(m_multiParPath));
#endif
  }

  void init() { GlobalMockObject::reset(); }
  void cleanup() { GlobalMockObject::verify(); }

  void ParVerifyInfomation_Ready_to_rename() {
    ParVerifyInfomation info{ParVerifyInfomationExamples::GetReadyToRenameStruct()};
    QCOMPARE(info.m_statusE, ParVerifyInfomation::Par2StatusE::READY_TO_RENAME);
    QCOMPARE(info.m_statusByteArray, QByteArray{R"(Ready to rename 1 file(s))"});
    QCOMPARE(info.m_prePath, "C:/Users/Ariel/Documents/TestQuickBar/");
    QCOMPARE(info.m_srcFileNames, (QStringList{"Another file need backup 2.txt"}));
    QCOMPARE(info.m_par2FileName, "Another file need backup 2.txt.par2");
    QCOMPARE(info.m_volPar2FileNameList, (QStringList{"Another file need backup 2.txt.vol0+1.par2", "Another file need backup 2.txt.vol1+1.par2"}));
    QCOMPARE(info.isTrustable(), true);
    QCOMPARE(info.getFirstSrcFileAbsPath(), "C:/Users/Ariel/Documents/TestQuickBar/Another file need backup 2.txt");
    QCOMPARE(info.getPar2FileAbsPath(), "C:/Users/Ariel/Documents/TestQuickBar/Another file need backup 2.txt.par2");
    QCOMPARE(info.m_completeCount, 0);
    QCOMPARE(info.m_misnamedCount, 1);
    QCOMPARE(info.m_damagedCount, 0);
    QCOMPARE(info.m_missingCount, 0);

    QByteArray renameCliOuput(ParVerifyInfomationExamples::CLI_OUPUT_WHEN_RENAME);
    QVERIFY(info.updateCliOutput(renameCliOuput));
    QCOMPARE(info.m_completeCount, 0 + 1);
    QCOMPARE(info.m_misnamedCount, 1 - 1);
    QCOMPARE(info.m_damagedCount, 0);
    QCOMPARE(info.m_missingCount, 0);
    /*

Complete file count	: 0

Misnamed file count	: 1
Damaged file count	: 0
Missing file count	: 0

Input File Slice found	: 1

Input File Slice avail	: 1
Input File Slice lost	: 0

Ready to rename 1 file(s)

Correcting file : 1
 Status   :  Filename
 Restored : "comicFile.txt"

Restored file count	: 1

Repaired successfully
*/
    QCOMPARE(info.m_statusE, ParVerifyInfomation::Par2StatusE::ALL_FILES_COMPLETE);
    QCOMPARE(info.m_statusByteArray, QByteArray{R"(Repaired successfully)"});
    QCOMPARE(info.m_prePath, "CLI_OUPUT_WHEN_RENAME_PATH/");
    QCOMPARE(info.m_srcFileNames, (QStringList{"comicFile.txt"}));
    QCOMPARE(info.m_par2FileName, "comicFile.txt.par2");
    QCOMPARE(info.m_volPar2FileNameList, (QStringList{"comicFile.txt.vol0+1.PAR2"}));
    QCOMPARE(info.isTrustable(), true);
  }

  void ParVerifyInfomation_Ready_to_repair() {
    ParVerifyInfomation info{ParVerifyInfomationExamples::GetReadyToRepairStruct()};
    QCOMPARE(info.m_statusE, ParVerifyInfomation::Par2StatusE::READY_TO_REPAIR);
    QCOMPARE(info.m_statusByteArray, QByteArray{R"(Ready to repair 1 file(s))"});
    QCOMPARE(info.m_prePath, "C:/Users/Ariel/Documents/TestQuickBar/");
    QCOMPARE(info.m_srcFileNames, (QStringList{"Another file need backup 2.txt"}));
    QCOMPARE(info.m_par2FileName, "Another file need backup 2.txt.par2");
    QCOMPARE(info.m_volPar2FileNameList, (QStringList{"Another file need backup 2.txt.vol0+1.par2", "Another file need backup 2.txt.vol1+1.par2"}));
    QCOMPARE(info.isTrustable(), true);
    QCOMPARE(info.m_completeCount, 0);
    QCOMPARE(info.m_misnamedCount, 0);
    QCOMPARE(info.m_damagedCount, 1);
    QCOMPARE(info.m_missingCount, 0);

    QByteArray repairCliOuput(ParVerifyInfomationExamples::CLI_OUPUT_WHEN_REPAIR);
    QVERIFY(info.updateCliOutput(repairCliOuput));
    QCOMPARE(info.m_completeCount, 0 + 1);
    QCOMPARE(info.m_misnamedCount, 0);
    QCOMPARE(info.m_damagedCount, 1 - 1);
    QCOMPARE(info.m_missingCount, 0);
/*

Misnamed file count	: 0
Damaged file count	: 1
Missing file count	: 0
Repaired file count	: 1
Input File Slice avail	: 4

Repaired successfully
*/
    QCOMPARE(info.m_statusE, ParVerifyInfomation::Par2StatusE::ALL_FILES_COMPLETE);
    QCOMPARE(info.m_statusByteArray, QByteArray{R"(Repaired successfully)"});
    QCOMPARE(info.m_prePath, "CLI_OUPUT_WHEN_REPAIR_PATH/");
    QCOMPARE(info.m_srcFileNames, (QStringList{"MusicFileNeedBackup.mp4"}));
    QCOMPARE(info.m_par2FileName, "MusicFileNeedBackup.mp4.par2");
    QCOMPARE(info.m_volPar2FileNameList, (QStringList{"MusicFileNeedBackup.mp4.vol0+1.par2"}));
    QCOMPARE(info.isTrustable(), true);
  }

  void ParVerifyInfomation_All_Files_Complete() {
    ParVerifyInfomation info{ParVerifyInfomationExamples::GetAllFilesCompleteStruct()};
    QCOMPARE(info.m_statusE, ParVerifyInfomation::Par2StatusE::ALL_FILES_COMPLETE);
    QCOMPARE(info.m_statusByteArray, QByteArray{R"(All Files Complete)"});
    QCOMPARE(info.m_prePath, "C:/Users/aria/AppData/Local/Temp/FileXplorerTestOnly-xVIKDr/");
    QCOMPARE(info.m_srcFileNames, (QStringList{"MediaFileNeedBackup.mp4"}));
    QCOMPARE(info.m_par2FileName, "MediaFileNeedBackup.mp4.par2");
    QCOMPARE(info.m_volPar2FileNameList, (QStringList{"MediaFileNeedBackup.mp4.vol0+1.par2"}));
    QCOMPARE(info.isTrustable(), true);
  }

  void ParVerifyInfomation_Need_more_slices_to_repair_files() {
    ParVerifyInfomation info{ParVerifyInfomationExamples::GetNeedMoreSlicesToRepairStruct()};
    QCOMPARE(info.m_statusE, ParVerifyInfomation::Par2StatusE::NEED_MORE_TO_REPAIR);
    QCOMPARE(info.m_statusByteArray, QByteArray{R"(Need 854 more slice(s) to repair 1 file(s))"});
    QCOMPARE(info.m_prePath, "C:/Users/Ariel/Documents/TestQuickBar/");
    QCOMPARE(info.m_srcFileNames, (QStringList{"Another file need backup 2.txt"}));
    QCOMPARE(info.m_par2FileName, "Another file need backup 2.txt.par2");
    QCOMPARE(info.m_volPar2FileNameList, (QStringList{"Another file need backup 2.txt.vol0+94.par2"}));
    QCOMPARE(info.isTrustable(), true);
  }

  void clampRedundancy_ok() {
    QCOMPARE(clampRedundancy(0), 10);
    QCOMPARE(clampRedundancy(5), 10);
    QCOMPARE(clampRedundancy(50), 50);
    QCOMPARE(clampRedundancy(10), 10);
    QCOMPARE(clampRedundancy(105), 100);
  }

  void GetRateOfRedundancyFromRate_ok() {
    QCOMPARE(GetRateOfRedundancyFromRate(0), 5);
    QCOMPARE(GetRateOfRedundancyFromRate(7), 8);
    QCOMPARE(GetRateOfRedundancyFromRate(8), 10);
    QCOMPARE(GetRateOfRedundancyFromRate(9), 15);
    QCOMPARE(GetRateOfRedundancyFromRate(10), 20);
    QCOMPARE(GetRateOfRedundancyFromRate(100), 5);
  }

  void CreatePar2_ok() {
    bool bSucceed{false};
    int bCount{0};

    std::tie(bSucceed, bCount) = CreatePar2(QStringList{}, 10);
    QVERIFY(bSucceed);
    QCOMPARE(bCount, 0);

    std::tie(bSucceed, bCount) = CreatePar2Automatic(QStringList{});
    QVERIFY(bSucceed);
    QCOMPARE(bCount, 0);

    std::tie(bSucceed, bCount) = CreatePar2(QStringList{mTDir.itemPath(m_mediaFileName)}, 10);
#ifndef _WIN32
    {
      QVERIFY(!bSucceed);
      QCOMPARE(bCount, 0);
      QVERIFY(!mTDir.fileExists(m_mediaPar2FileName));
    }
    // no json exist. no par2.exe available
    std::tie(bSucceed, bCount) = CreatePar2Automatic(QStringList{mTDir.itemPath(m_mediaFileName)});
    {
      QVERIFY(!bSucceed);
      QCOMPARE(bCount, 0);
      QVERIFY(!mTDir.fileExists(m_mediaPar2FileName));
    }

    ParVerifyInfomation info = VerifyAFile(m_multiParPath, m_mediaPar2FileAbsPath); // failed in QProcess.waitForFinished()
    QCOMPARE(info.isTrustable(), false);
    QCOMPARE(info.m_statusE, ParVerifyInfomation::Par2StatusE::NEED_VERIFY_AGAIN);

    QByteArray repairOutputCli;
    QCOMPARE(RepairAFile(m_multiParPath, m_mediaPar2FileAbsPath, &repairOutputCli), RepairResultE::CANNOT_REPAIR); // failed in QProcess.waitForFinished()
    QVERIFY(repairOutputCli.isEmpty());
    return;
#endif
    {
      QVERIFY(bSucceed);
      QCOMPARE(bCount, 1);
      QVERIFY(mTDir.fileExists(m_mediaPar2FileName));
    }
    // no json exist and .par2 already exist.
    std::tie(bSucceed, bCount) = CreatePar2Automatic(QStringList{mTDir.itemPath(m_mediaFileName)});
    {
      QVERIFY(bSucceed);
      QCOMPARE(bCount, 0);
      QVERIFY(mTDir.fileExists(m_mediaPar2FileName));
    }


    ParVerifyInfomation info;
    {
      QVERIFY(!info.isTrustable());
      QVERIFY(info.isNeedRepair());
      QCOMPARE(info.m_statusE, ParVerifyInfomation::Par2StatusE::NEED_VERIFY_AGAIN);
    }

    const auto completeCheck = [](const ParVerifyInfomation infoToCheck) -> void {
      QCOMPARE(infoToCheck.isTrustable(), true);
      QCOMPARE(infoToCheck.isNeedRepair(), false);
      QCOMPARE(infoToCheck.m_statusE, ParVerifyInfomation::Par2StatusE::ALL_FILES_COMPLETE);
      QCOMPARE(infoToCheck.m_completeCount, 1);
      QCOMPARE(infoToCheck.m_damagedCount, 0);
    };

    completeCheck(VerifyAFile(m_multiParPath, m_mediaPar2FileAbsPath));
    QCOMPARE(RepairAFile(m_multiParPath, m_mediaPar2FileAbsPath), RepairResultE::NO_NEED_REPAIR); // no need repair

    // 1.0 modify its contents(less than rate of redundancy), can repair
    {
      QVERIFY(mTDir.touch(m_mediaFileName, m_corruptContent));
      {
        bool bReadOk{false};
        QByteArray actualReadResult = FileTool::ByteArrayReader(m_mediaFileAbsPath, &bReadOk);
        QVERIFY(bReadOk);
        QCOMPARE(actualReadResult, m_corruptContent);
      }

      // VerifyAFile ok. find damaged: 1
      {
        info = VerifyAFile(m_multiParPath, m_mediaPar2FileAbsPath);
        QCOMPARE(info.isTrustable(), true);
        QCOMPARE(info.isNeedRepair(), true);
        QCOMPARE(info.isRepairable(), true);
        QCOMPARE(info.m_statusE, ParVerifyInfomation::Par2StatusE::READY_TO_REPAIR);
        QCOMPARE(info.m_completeCount, 0);
        QCOMPARE(info.m_damagedCount, 1);
      }

      // RepairAFile
      QByteArray pRepairCliOutput;
      QCOMPARE(RepairAFile(m_multiParPath, m_mediaPar2FileAbsPath, &pRepairCliOutput), RepairResultE::SUCCESS_REPAIRED);
      {
        bool bReadOk{false};
        QByteArray actualReadResult = FileTool::ByteArrayReader(m_mediaFileAbsPath, &bReadOk);
        QVERIFY(bReadOk);
        QCOMPARE(actualReadResult, m_expectCorrectContent);

        QVERIFY(pRepairCliOutput.size() > 0);
        ParVerifyInfomation newInfo = info;
        QVERIFY(newInfo.updateCliOutput(pRepairCliOutput));
        QCOMPARE((newInfo == info), false);

        completeCheck(newInfo);
      }

      // VerifyAFile ok, find damaged: 0
      completeCheck(VerifyAFile(m_multiParPath, m_mediaPar2FileAbsPath));
    }

    // 2.0 rename srcfile, can repair
    {
      QVERIFY(QFile::rename(m_mediaFileAbsPath, m_mediaFileAbsPathNew));
      QVERIFY(!QFile::exists(m_mediaFileAbsPath));
      {
        info = VerifyAFile(m_multiParPath, m_mediaPar2FileAbsPath);
        QCOMPARE(info.isTrustable(), true);
        QCOMPARE(info.isNeedRepair(), true);
        QCOMPARE(info.isRepairable(), true);
        QCOMPARE(info.m_statusE, ParVerifyInfomation::Par2StatusE::READY_TO_RENAME);
        QCOMPARE(info.m_completeCount, 0);
        QCOMPARE(info.m_misnamedCount, 1);
      }
      QCOMPARE(RepairAFile(m_multiParPath, m_mediaPar2FileAbsPath), RepairResultE::SUCCESS_REPAIRED);
      QVERIFY(QFile::exists(m_mediaFileAbsPath));
      completeCheck(VerifyAFile(m_multiParPath, m_mediaPar2FileAbsPath));
    }

    // 3.0 delete all contents, need more slices to repair files
    {
      mTDir.touch(m_mediaFileName, QByteArray{""});
      {
        bool bReadOk{false};
        QByteArray actualReadResult = FileTool::ByteArrayReader(m_mediaFileAbsPath, &bReadOk);
        QVERIFY(bReadOk);
        QVERIFY(actualReadResult.isEmpty());
      }

      // VerifyAFile ok. find damaged: 1
      auto needMoreToRepair = [](const ParVerifyInfomation& info) {
        QCOMPARE(info.isTrustable(), true);
        QCOMPARE(info.isNeedRepair(), true);
        QCOMPARE(info.isRepairable(), false);
        QCOMPARE(info.m_statusE, ParVerifyInfomation::Par2StatusE::NEED_MORE_TO_REPAIR);
        QCOMPARE(info.m_completeCount, 0);
        QCOMPARE(info.m_damagedCount, 1);
      };

      info = VerifyAFile(m_multiParPath, m_mediaPar2FileAbsPath);
      needMoreToRepair(info);

      // cannot repair
      QCOMPARE(RepairAFile(m_multiParPath, m_mediaPar2FileAbsPath), RepairResultE::NEED_MORE_TO_REPAIR);

      info = VerifyAFile(m_multiParPath, m_mediaPar2FileAbsPath);
      needMoreToRepair(info);
    }
    QVERIFY(mTDir.touch(m_mediaFileName, m_expectCorrectContent));
  }

  void VerifyFiles_ok() {
    // std::pair<bool, ParVerifyInfomationList> VerifyFiles(const QStringList& filesAbsPath);
    using namespace ParVerifyInfomationExamples;
    ParVerifyInfomation allFilesComplete{GetAllFilesCompleteStruct()};
    ParVerifyInfomation readyToRename{GetReadyToRenameStruct()};
    ParVerifyInfomation readyToRepair{GetReadyToRepairStruct()};
    ParVerifyInfomation untrustable{GetVerifyUntrustableStruct()};
    ParVerifyInfomation needMoreSlicesToRepair{GetNeedMoreSlicesToRepairStruct()};

    MOCKER(VerifyAFile)
        .expects(exactly(5))                       //
        .will(returnValue(allFilesComplete))       //
        .then(returnValue(readyToRename))          //
        .then(returnValue(readyToRepair))          //
        .then(returnValue(untrustable))            //
        .then(returnValue(needMoreSlicesToRepair)) //
        ;
    MOCKER(IsMultiPar2Available).stubs().will(returnValue(false)).then(returnValue(true));

    bool verifyResult{false};
    ParVerifyInfomationList infoList;
    std::tie(verifyResult, infoList) = VerifyFiles({});
    QVERIFY(verifyResult);
    QVERIFY(infoList.isEmpty());

    const QStringList selectedFiles{
        // no .par2 find correspond to srcfile
        "noCorrespondPar2.mp4"
        // 1
        "a.mp4",
        "a.par2",
        "a.vol0+1.par2",
        // 2
        "b.mp4",
        "b.par2",
        "b.vol0+1.par2",
        // 3
        "c.mp4",
        "c.par2",
        "c.vol0+1.par2",
        // QProcess.waitForFinished() failed
        "untrustable.mp4",
        "untrustable.par2",
        "untrustable.vol0+1.par2",
        // 4
        "d.mp4",
        "d.par2",
        "d.vol0+1.par2",
    };

    std::tie(verifyResult, infoList) = VerifyFiles(selectedFiles); // IsMultiPar2Available: false
    QVERIFY(!verifyResult);
    QVERIFY(infoList.isEmpty());

    std::tie(verifyResult, infoList) = VerifyFiles(selectedFiles); // IsMultiPar2Available: true
    QVERIFY(verifyResult);
    QCOMPARE(infoList.size(), 5);
  }
};

#include "MultiParToolsTest.moc"
REGISTER_TEST(MultiParToolsTest, false)
