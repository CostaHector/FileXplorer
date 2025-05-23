#include <QCoreApplication>
#include <QtTest>

// add necessary includes here
#include "Tools/ConflictsItemHelper.h"
#include "public/UndoRedo.h"
#include "Public/PublicVariable.h"
#include "pub/FileSystemTestSuite.h"
#include "model/ConflictsFileSystemModel.h"
#include <QSet>

class ConflictSolveModelTest : public FileSystemTestSuite {
  Q_OBJECT
 public:
  ConflictSolveModelTest() : FileSystemTestSuite{"TestEnv_ConflictSolve", true, false} {}

 private slots:
  void initTestCase() {
    QCOMPARE(CCMMode::MCCL2STR[CCMMode::MERGE_OP], QString{"MERGE_OP"});
    QCOMPARE(CCMMode::MCCL2STR[CCMMode::COPY_OP], QString{"COPY_OP"});
    QCOMPARE(CCMMode::MCCL2STR[CCMMode::CUT_OP], QString{"CUT_OP"});
    QCOMPARE(CCMMode::MCCL2STR[CCMMode::LINK_OP], QString{"LINK_OP"});
    QCOMPARE(CCMMode::MCCL2STR[CCMMode::ERROR_OP], QString{"ERROR_OP"});
  }

  void init() {
    /* Movie{movie (0).png,movie (1).png,movie.json},Page12{Movie{
movie - BTS.mp4
movie (0).png
movie (1).png
movie (2).png
movie (3).png
movie (4).png
movie (5).png
movie (6).png
movie (7).png
movie (8).png
movie (9).png
movie.json
movie.mp4
     }}, NoConflictPage{RandomFolder{RandomFile.txt}}
     */
    m_rootHelper << FileSystemNode{"Movie"}                                   //
                 << FileSystemNode{"Page12"}                                  //
                 << FileSystemNode{"NoConflictPage"};                         //
    m_rootHelper.GetSubHelper("Movie")                                        //
        << FileSystemNode{"movie (0).png", false, ""}                         //
        << FileSystemNode{"movie (1).png", false, ""}                         //
        << FileSystemNode{"movie.json", false, "AAAAAAAAAA"};                 //
    m_rootHelper.GetSubHelper("Page12")                                       //
        << FileSystemNode{"Movie"};                                           //
    m_rootHelper.GetSubHelper("Page12").GetSubHelper("Movie")                 //
        << FileSystemNode{"movie - BTS.mp4", false, "0123456789"}             //
        << FileSystemNode{"movie (0).png", false, "0123456"}                  //
        << FileSystemNode{"movie (1).png", false, ""}                         //
        << FileSystemNode{"movie (2).png", false, ""}                         //
        << FileSystemNode{"movie (3).png", false, ""}                         //
        << FileSystemNode{"movie (4).png", false, ""}                         //
        << FileSystemNode{"movie (5).png", false, ""}                         //
        << FileSystemNode{"movie (6).png", false, ""}                         //
        << FileSystemNode{"movie (7).png", false, ""}                         //
        << FileSystemNode{"movie (8).png", false, ""}                         //
        << FileSystemNode{"movie (9).png", false, ""}                         //
        << FileSystemNode{"movie.json", false, ""}                            //
        << FileSystemNode{"movie.mp4", false, "012345678901234567891"};       //
    m_rootHelper.GetSubHelper("NoConflictPage")                               //
        << FileSystemNode{"RandomFolder"};                                    //
    m_rootHelper.GetSubHelper("NoConflictPage").GetSubHelper("RandomFolder")  //
        << FileSystemNode{"RandomFile.txt"};                                  //
  }

  void test_enviromentMet() {
    QVERIFY(QDir(mTestPath).exists("Movie/movie (0).png"));
    QVERIFY(QDir(mTestPath).exists("Movie/movie (1).png"));
    QVERIFY(QDir(mTestPath).exists("Movie/movie.json"));

    QVERIFY(QDir(mTestPath).exists("Page12/Movie/movie (0).png"));
    QVERIFY(QDir(mTestPath).exists("Page12/Movie/movie (1).png"));
    QVERIFY(QDir(mTestPath).exists("Page12/Movie/movie (2).png"));
    QVERIFY(QDir(mTestPath).exists("Page12/Movie/movie (3).png"));
    QVERIFY(QDir(mTestPath).exists("Page12/Movie/movie (4).png"));
    QVERIFY(QDir(mTestPath).exists("Page12/Movie/movie (5).png"));
    QVERIFY(QDir(mTestPath).exists("Page12/Movie/movie (6).png"));
    QVERIFY(QDir(mTestPath).exists("Page12/Movie/movie (7).png"));
    QVERIFY(QDir(mTestPath).exists("Page12/Movie/movie (8).png"));
    QVERIFY(QDir(mTestPath).exists("Page12/Movie/movie (9).png"));
    QVERIFY(QDir(mTestPath).exists("Page12/Movie/movie.json"));
    QVERIFY(QDir(mTestPath).exists("Page12/Movie/movie - BTS.mp4"));
    QVERIFY(QDir(mTestPath).exists("Page12/Movie/movie.mp4"));

    QVERIFY(QDir(mTestPath).exists("NoConflictPage/RandomFolder/RandomFile.txt"));

    QVERIFY2(QFileInfo(mTestPath, "Movie/movie.json").size() > QFileInfo(mTestPath, "Page12/Movie/movie.json").size(),  //
             "By default, keep left one");
    QVERIFY2(QFileInfo(mTestPath, "Movie/movie (0).png").size() < QFileInfo(mTestPath, "Page12/Movie/movie (0).png").size(),  //
             "By default, keep right one");
  }

  void test_noConflictCut() {
    CCMMode::Mode mode = CCMMode::CUT_OP;
    const QString l = QDir(mTestPath).absoluteFilePath("NoConflictPage");
    const QString r = mTestPath;
    const auto& conflictIF0 = ConflictsItemHelper(l, r, {"RandomFolder"}, mode);
    QVERIFY2(!conflictIF0, "no conflict at all");
    ConflictsFileSystemModel conflictModel{conflictIF0, nullptr};

    const auto& conflictMap = conflictModel.getConflictMap();
    QCOMPARE(conflictMap.size(), 0);

    if (!conflictModel.isCommandsAvail()) {
      conflictModel.updateCommands();
    }
    const auto& cmds = conflictModel.getCommands();
    QVERIFY2(not cmds.isEmpty(), "commands should not be empty");
    bool isRenameAllSucceed = g_undoRedo.Do(cmds);
    QVERIFY2(isRenameAllSucceed, "operation should all succeed");

    QDir testDir{mTestPath};
    QVERIFY2(testDir.exists("Movie"), "no conflict");
    QVERIFY2(testDir.exists("RandomFolder"), "noconflict");
    QVERIFY2(testDir.exists("NoConflictPage"), "noconflict");

    QDir todir{testDir.absoluteFilePath("RandomFolder"), "*", QDir::SortFlag::NoSort, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot};
    const auto& fileLst = todir.entryList();
    QVERIFY2(fileLst.size() == 1, "contains only 1 file [RandomFile.txt]");
  }

  void test_practicalCutTest() {
    CCMMode::Mode mode = CCMMode::CUT_OP;
    const QString l = QDir(mTestPath).absoluteFilePath("Page12");
    const QString r = mTestPath;
    const auto& conflictIF0 = ConflictsItemHelper(l, r, {"Movie"}, mode);
    QVERIFY2(bool(conflictIF0), "helper should be true");

    ConflictsFileSystemModel conflictModel{conflictIF0, nullptr};
    const auto& conflictMap = conflictModel.getConflictMap();
    QCOMPARE(conflictMap.size(), 4);  // 4 elements {"movie", "movie.json", "movie (0).png", "movie (1).png"}

    if (!conflictModel.isCommandsAvail()) {
      conflictModel.updateCommands();
    }
    const auto& cmds = conflictModel.getCommands();
    QVERIFY2(!cmds.isEmpty(), "commands should not be empty");
    bool isRenameAllSucceed = g_undoRedo.Do(cmds);
    QVERIFY2(isRenameAllSucceed, "operation should all succeed");

    QVERIFY2(!QDir(mTestPath).exists("Page12/Movie"), "from path[Page12/Movie] should be recycled");
    QVERIFY2(QFileInfo(mTestPath, "Movie/movie.json").size() > 0, "large file[movie.json] should keep");
    QVERIFY2(QFileInfo(mTestPath, "Movie/movie (0).png").size() > 0, "large file[movie (0).png] should keep");

    QDir todir{QDir(mTestPath).absoluteFilePath("Movie"), "*", QDir::SortFlag::NoSort, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot};
    const auto& fileLst = todir.entryList();
    QVERIFY2(fileLst.size() >= 10 + 1 + 2, "contains at least 10 images, 1 json, 2 movies");
  }
};

#include "ConflictSolveModelTest.moc"
ConflictSolveModelTest g_ConflictSolveModelTest;
