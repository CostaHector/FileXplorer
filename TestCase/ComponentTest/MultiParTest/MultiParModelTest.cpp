#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "MultiParModel.h"
#include "EndToExposePrivateMember.h"

#include "ParVerifyInfomation.h"
#include "ParVerifyInfomationExamples.h"
#include "MultiParTools.h"
#include "TDir.h"
#include "SystemPath.h"
#include "PathTool.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

QByteArray g_repairCliOutput;
MultiParTools::RepairResultE g_returnValue = MultiParTools::RepairResultE::CANNOT_REPAIR;
MultiParTools::RepairResultE invokeRepairAFile(const QString& multiParPath, const QString& parFileAbsPath, QByteArray* pRepairCliOutput) {
  if (pRepairCliOutput != nullptr) {
    *pRepairCliOutput = g_repairCliOutput;
  }
  return g_returnValue;
}

class MultiParModelTest : public PlainTestSuite {
  Q_OBJECT
public:
  using PlainTestSuite::PlainTestSuite;

private:
private slots:
  void init() { GlobalMockObject::reset(); }
  void cleanup() { GlobalMockObject::verify(); }

  void default_ok() {
    ParVerifyInfomationList emptyInfoList;
    MultiParModel model{std::move(emptyInfoList)};
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.columnCount(), MultiParKey::COLUMNS_CNT_BUTT);

    // out of range not crash down
    QCOMPARE(model.headerData(0, Qt::Horizontal, Qt::DisplayRole).toString(), "SOURCE_FILES");
    QCOMPARE(model.headerData(998, Qt::Horizontal, Qt::DisplayRole).toInt(), 999);

    QCOMPARE(model.headerData(0, Qt::Vertical, Qt::DisplayRole).toInt(), 1);

    QCOMPARE(model.headerData(0, Qt::Vertical, Qt::TextAlignmentRole).toInt(), int(Qt::AlignRight));
    model.headerData(0, Qt::Horizontal, Qt::TextAlignmentRole).toInt();

    QCOMPARE(model.GetFrontSourceFile(QModelIndex{}), "");
    QCOMPARE(model.GetPar2FileAbsPath(QModelIndex{}), "");
  }

  void data_retrieve_ok() {
    using namespace ParVerifyInfomationExamples;
    ParVerifyInfomation allFilesComplete{GetAllFilesCompleteStruct()};
    ParVerifyInfomation readyToRename{GetReadyToRenameStruct()};
    ParVerifyInfomation readyToRepair{GetReadyToRepairStruct()};
    ParVerifyInfomation needMoreSlicesToRepair{GetNeedMoreSlicesToRepairStruct()};
    ParVerifyInfomation untrustable{GetVerifyUntrustableStruct()};

    ParVerifyInfomationList infoList{allFilesComplete, readyToRename, readyToRepair, untrustable, needMoreSlicesToRepair};
    QCOMPARE(infoList.size(), 5);
    MultiParModel model{std::move(infoList)};
    QCOMPARE(model.rowCount(), 5);

    QCOMPARE(model.data(QModelIndex{}, Qt::DisplayRole).isValid(), false);
    model.index(0, MultiParKey::ColumnE::SOURCE_FILES).data(Qt::TextAlignmentRole);

    QCOMPARE(model.index(0, MultiParKey::ColumnE::SOURCE_FILES).data(Qt::DisplayRole).toStringList(), allFilesComplete.m_srcFileNames);
    QCOMPARE(model.index(0, MultiParKey::ColumnE::PAR2_FILE).data(Qt::DisplayRole).toString(), allFilesComplete.m_par2FileName);
    QCOMPARE(model.index(0, MultiParKey::ColumnE::VOL_PAR2_FILES).data(Qt::DisplayRole).toStringList(), allFilesComplete.m_volPar2FileNameList);
    QCOMPARE(model.index(0, MultiParKey::ColumnE::STATUS).data(Qt::DisplayRole).toByteArray(), allFilesComplete.m_statusByteArray);
    QCOMPARE(model.index(0, MultiParKey::ColumnE::COMPLETE_CNT).data(Qt::DisplayRole).toInt(), allFilesComplete.m_completeCount);
    QCOMPARE(model.index(0, MultiParKey::ColumnE::MISNAMED_CNT).data(Qt::DisplayRole).toInt(), allFilesComplete.m_misnamedCount);
    QCOMPARE(model.index(0, MultiParKey::ColumnE::DAMAGED_CNT).data(Qt::DisplayRole).toInt(), allFilesComplete.m_damagedCount);
    QCOMPARE(model.index(0, MultiParKey::ColumnE::MISSING_CNT).data(Qt::DisplayRole).toInt(), allFilesComplete.m_missingCount);
    QCOMPARE(model.index(0, MultiParKey::ColumnE::PRE_PATH).data(Qt::DisplayRole).toString(), allFilesComplete.m_prePath);

    QCOMPARE(model.GetFrontSourceFile(model.index(0,0)), allFilesComplete.getFirstSrcFileAbsPath());
    QCOMPARE(model.GetPar2FileAbsPath(model.index(0,0)), allFilesComplete.getPar2FileAbsPath());

    QCOMPARE(model.index(0, MultiParKey::ColumnE::SOURCE_FILES).data(Qt::DecorationRole).isValid(), true);
    QCOMPARE(model.index(0, MultiParKey::ColumnE::PAR2_FILE).data(Qt::DecorationRole).isValid(), true);
    model.index(0, MultiParKey::ColumnE::STATUS).data(Qt::DecorationRole).isValid();
    QCOMPARE(model.index(0, MultiParKey::ColumnE::PRE_PATH).data(Qt::DecorationRole).isValid(), false);

    QVERIFY(!allFilesComplete.isNeedRepair());
    QVERIFY(readyToRepair.isNeedRepair());
    QVariant noNeedRepairColorVar = model.index(0, MultiParKey::ColumnE::SOURCE_FILES).data(Qt::ForegroundRole);
    QVariant needRepairRedColorVar = model.index(2, MultiParKey::ColumnE::SOURCE_FILES).data(Qt::ForegroundRole);
    QCOMPARE(needRepairRedColorVar.canConvert<QColor>(), true);
    QCOMPARE(needRepairRedColorVar.value<QColor>(), (QColor{Qt::GlobalColor::red}));

    QCOMPARE(model.GetCliOutput(QModelIndex{}), nullptr);
    const QByteArray* pCliOuput = model.GetCliOutput(model.index(0, 0));
    QCOMPARE(*pCliOuput, allFilesComplete.m_verifyOutput);
  }

  void repairBrokenFile_ok() {
    using namespace ParVerifyInfomationExamples;
    ParVerifyInfomation allFilesComplete{GetAllFilesCompleteStruct()};
    ParVerifyInfomation readyToRename{GetReadyToRenameStruct()};
    ParVerifyInfomation readyToRepair{GetReadyToRepairStruct()};
    ParVerifyInfomation needMoreSlicesToRepair{GetNeedMoreSlicesToRepairStruct()};
    ParVerifyInfomation untrustable{GetVerifyUntrustableStruct()};

    ParVerifyInfomationList infoList{allFilesComplete, readyToRename, readyToRepair, untrustable, needMoreSlicesToRepair};
    QCOMPARE(infoList.size(), 5);
    MultiParModel model{std::move(infoList)};
    QCOMPARE(model.rowCount(), 5);

    QCOMPARE(model.repairBrokenFile({}), 0);

    QModelIndex ind0{model.index(0, 0)};
    QModelIndex ind1{model.index(1, 0)}; // repair name
    QModelIndex ind2{model.index(2, 0)}; // repair broken content
    QModelIndex ind3{model.index(3, 0)};
    QModelIndex ind4{model.index(4, 0)};
    QModelIndex ind5{model.index(5, 0)}; // out of range protection

    QModelIndexList inds0_5{ind0, ind1, ind2, ind3, ind4, ind5};

    QVERIFY(!allFilesComplete.isNeedRepair());
    QVERIFY(readyToRename.isNeedRepair());
    QVERIFY(readyToRepair.isNeedRepair());
    QVERIFY(needMoreSlicesToRepair.isNeedRepair());
    QVERIFY(untrustable.isNeedRepair());

    QVERIFY(allFilesComplete.isRepairable());
    QVERIFY(readyToRename.isRepairable()); // repair
    QVERIFY(readyToRepair.isRepairable()); // repair
    QVERIFY(!needMoreSlicesToRepair.isRepairable());
    QVERIFY(!untrustable.isRepairable());

    MOCKER(MultiParTools::IsMultiPar2Available).stubs().will(returnValue(false)).then(returnValue(true));
    MOCKER(MultiParTools::RepairAFile).expects(exactly(3)).will(invoke(invokeRepairAFile)); //
    // repair 1
    // repair 2.1
    // repair 2.2
    QCOMPARE(model.repairBrokenFile(inds0_5, ParVerifyInfomation::Par2StatusE::READY_TO_REPAIR), -1); // IsMultiPar2Available: false

    // will no update
    g_repairCliOutput = "";
    g_returnValue = MultiParTools::RepairResultE::CANNOT_REPAIR;
    QCOMPARE(model.repairBrokenFile(inds0_5, ParVerifyInfomation::Par2StatusE::READY_TO_REPAIR), 0); // IsMultiPar2Available: true, repair 1 failed
    QCOMPARE(model.mVerifyInfoList[ind1.row()] == allFilesComplete, false);

    // will update all
    g_returnValue = MultiParTools::RepairResultE::SUCCESS_REPAIRED;
    g_repairCliOutput = CLI_OUTPUT_ALL_FILES_COMPLETE;
    QCOMPARE(model.repairBrokenFile(inds0_5, ParVerifyInfomation::Par2StatusE::READY_TO_RENAME), 1); // IsMultiPar2Available: true
    QCOMPARE(model.mVerifyInfoList[ind1.row()], allFilesComplete);

    QCOMPARE(model.repairBrokenFile(inds0_5, ParVerifyInfomation::Par2StatusE::READY_TO_REPAIR), 1); // IsMultiPar2Available: true
    QCOMPARE(model.mVerifyInfoList[ind2.row()], allFilesComplete);
  }

  void ProcessOldNewPar2Names_ReverifyPar2File_ok() {
    TDir mTDir;
    QVERIFY(mTDir.IsValid());
    const QString folderPath{mTDir.path()};
    const QList<FsNodeEntry> nodes{
        {"allFilesCompleteFile.txt", false, "CompleteCompleteCompleteCompleteComplete"}, //
        {"readyToRenameFile.txt", false, "RenameRenameRenameRenameRenameRenameRenameRenameRenameRenameRenameRename"},    //
        {"readyToRepairFile.txt", false, "BrokenBrokenBrokenBrokenBrokenBroken"},    //
    };
    QCOMPARE(mTDir.createEntries(nodes), 3);

    const QStringList srcFiles{
        mTDir.itemPath("allFilesCompleteFile.txt"), //
        mTDir.itemPath("readyToRenameFile.txt"),    //
        mTDir.itemPath("readyToRepairFile.txt"),    //
    };
    const std::pair<bool, int> crtResult2Cnt = MultiParTools::CreatePar2(srcFiles, 10); //
#ifndef _WIN32
    QCOMPARE(crtResult2Cnt, std::make_pair(false, 0));
#else
    QCOMPARE(crtResult2Cnt, std::make_pair(true, 3));
#endif

    const QString multiParPath{SystemPath::MULTI_PAR_PROG_PATH()};
    using namespace ParVerifyInfomationExamples;
    ParVerifyInfomation allFilesComplete{MultiParTools::VerifyAFile(multiParPath, mTDir.itemPath("allFilesCompleteFile.txt.par2"))};
    // rename precondition;
    {
      ParVerifyInfomation tempReadyToRename{MultiParTools::VerifyAFile(multiParPath, mTDir.itemPath("readyToRenameFile.txt.par2"))};
      QCOMPARE(tempReadyToRename.m_srcFileNames.size(), 1);
      QStringList preconditonRenameWork;
      preconditonRenameWork.push_back(PathTool::Path2Join(tempReadyToRename.m_prePath, tempReadyToRename.m_srcFileNames.front()));
      preconditonRenameWork.push_back(PathTool::Path2Join(tempReadyToRename.m_prePath, tempReadyToRename.m_par2FileName));
      for (const QString& volPar2 : tempReadyToRename.m_volPar2FileNameList) {
        preconditonRenameWork.push_back(PathTool::Path2Join(tempReadyToRename.m_prePath, volPar2));
      }
      for (const QString& oldPath : preconditonRenameWork) {
        QString newPath{oldPath};
        newPath.replace("readyToRenameFile.txt", "readyToRenameFile8888.txt");
        QVERIFY(QFile::rename(oldPath, newPath));
      }
    }
    ParVerifyInfomation readyToRename = MultiParTools::VerifyAFile(multiParPath, mTDir.itemPath("readyToRenameFile8888.txt.par2"));

    // broken precondition;
    {
      mTDir.touch("readyToRepairFile.txt", "");
    }
    ParVerifyInfomation readyToRepair{MultiParTools::VerifyAFile(multiParPath, mTDir.itemPath("readyToRepairFile.txt.par2"))};
    ParVerifyInfomation untrustable{GetVerifyUntrustableStruct()};

    ParVerifyInfomationList infoList{allFilesComplete, readyToRename, readyToRepair, untrustable};
    QCOMPARE(infoList.size(), 4);
    MultiParModel model{std::move(infoList)};
    QCOMPARE(model.rowCount(), 4);

    QModelIndex ind0{model.index(0, 0)}; // allFilesComplete
    QModelIndex ind1{model.index(1, 0)}; // readyToRename needReverify
    QModelIndex ind2{model.index(2, 0)}; // readyToRepair
    QModelIndex ind3{model.index(3, 0)}; // untrustable
    QModelIndex ind4{model.index(4, 0)}; // out of range protection
    QModelIndexList inds0_4{ind0, ind1, ind2, ind3, ind4};

    QVERIFY(model.ProcessOldNewPar2Names({}).isEmpty());
    QCOMPARE(model.ReverifyPar2File({}), 0);

    QModelIndexList needReverifyIndexes;
    QVERIFY(untrustable.m_statusE == ParVerifyInfomation::Par2StatusE::NEED_VERIFY_AGAIN);
#ifndef _WIN32
    QVERIFY(allFilesComplete.m_statusE != ParVerifyInfomation::Par2StatusE::ALL_FILES_COMPLETE);
    QVERIFY(readyToRename.m_statusE != ParVerifyInfomation::Par2StatusE::READY_TO_RENAME);
    QVERIFY(readyToRepair.m_statusE != ParVerifyInfomation::Par2StatusE::READY_TO_REPAIR);
    QCOMPARE(model.ProcessOldNewPar2Names(inds0_4), needReverifyIndexes);
    return;
#endif
    QVERIFY(allFilesComplete.m_statusE == ParVerifyInfomation::Par2StatusE::ALL_FILES_COMPLETE);
    QVERIFY(readyToRename.m_statusE == ParVerifyInfomation::Par2StatusE::READY_TO_RENAME);
    QVERIFY(readyToRepair.m_statusE == ParVerifyInfomation::Par2StatusE::READY_TO_REPAIR);

    QVERIFY(!allFilesComplete.isNeedReverify()); // no need reverify at all
    QVERIFY(readyToRename.isNeedReverify());     // need reverify, but no new par2 file endswith("_SYNC") exists;
    QVERIFY(!readyToRepair.isNeedReverify());    // no need reverify at all
    QVERIFY(untrustable.isNeedReverify());      // no need reverify at all

    QCOMPARE(model.mVerifyInfoList[ind1.row()].m_statusE, ParVerifyInfomation::Par2StatusE::READY_TO_RENAME);
    needReverifyIndexes = model.ProcessOldNewPar2Names(inds0_4);
    QVERIFY(needReverifyIndexes.isEmpty());
    QCOMPARE(model.mVerifyInfoList[ind1.row()].m_statusE, ParVerifyInfomation::Par2StatusE::READY_TO_RENAME);

    QCOMPARE(model.syncBuiltInSrcFileListInPar2(inds0_4), 1);
    needReverifyIndexes = model.ProcessOldNewPar2Names(inds0_4);
    QCOMPARE(needReverifyIndexes, (QModelIndexList{ind1}));

    MOCKER(MultiParTools::IsMultiPar2Available).expects(exactly(2)).will(returnValue(false)).then(returnValue(true));
    QCOMPARE(model.ReverifyPar2File(needReverifyIndexes), -1); // IsMultiPar2Available: false
    QCOMPARE(ind1.siblingAtColumn(MultiParKey::ColumnE::COMPLETE_CNT).data(Qt::DisplayRole).toInt(), 0);
    QCOMPARE(ind1.siblingAtColumn(MultiParKey::ColumnE::MISNAMED_CNT).data(Qt::DisplayRole).toInt(), 1);
    QCOMPARE(ind1.siblingAtColumn(MultiParKey::ColumnE::DAMAGED_CNT).data(Qt::DisplayRole).toInt(), 0);
    QCOMPARE(ind1.siblingAtColumn(MultiParKey::ColumnE::MISSING_CNT).data(Qt::DisplayRole).toInt(), 0);

    QCOMPARE(model.mVerifyInfoList[ind1.row()].m_statusE, ParVerifyInfomation::Par2StatusE::NEED_VERIFY_AGAIN);
    QCOMPARE(model.ReverifyPar2File(needReverifyIndexes), 1); // IsMultiPar2Available: true
    QCOMPARE(model.mVerifyInfoList[ind1.row()].m_statusE, ParVerifyInfomation::Par2StatusE::ALL_FILES_COMPLETE);

    QCOMPARE(ind1.siblingAtColumn(MultiParKey::ColumnE::COMPLETE_CNT).data(Qt::DisplayRole).toInt(), 1);
    QCOMPARE(ind1.siblingAtColumn(MultiParKey::ColumnE::MISNAMED_CNT).data(Qt::DisplayRole).toInt(), 0);
    QCOMPARE(ind1.siblingAtColumn(MultiParKey::ColumnE::DAMAGED_CNT).data(Qt::DisplayRole).toInt(), 0);
    QCOMPARE(ind1.siblingAtColumn(MultiParKey::ColumnE::MISSING_CNT).data(Qt::DisplayRole).toInt(), 0);
  }
};

#include "MultiParModelTest.moc"
REGISTER_TEST(MultiParModelTest, false)
