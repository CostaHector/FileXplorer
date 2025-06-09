#include <QCoreApplication>
#include <QtTest>

#include "pub/MyTestSuite.h"
#include "TestCase/PathRelatedTool.h"

#include "pub/BeginToExposePrivateMember.h"
#include "Tools/RenameNamesUnique.h"
#include "pub/EndToExposePrivateMember.h"

const QString gTestPath{TestCaseRootPath() + "/test/TestEnv_RenameNamesUnique"};

class RenameUnqiueCheckTest : public MyTestSuite {
  Q_OBJECT
 public:
  RenameUnqiueCheckTest() : MyTestSuite{false} {}

 private slots:
  void initTestCase() {
    /*
    rename12To45Basic{1, 2}
    renameDiretoryNameShouldNotConflict{dirName{dirName, file1}}
    */
    const QDir dir{gTestPath};
    QVERIFY(dir.exists("rename12To45Basic/1"));
    QVERIFY(dir.exists("rename12To45Basic/2"));
    QVERIFY(dir.exists("renameDiretoryNameShouldNotConflict/dirName/dirName"));
    QVERIFY(dir.exists("renameDiretoryNameShouldNotConflict/dirName/file1"));
  }

  void cleanupTestCase() {}

  void test_get_occupied_name_ok_basic() {
    const QString pre = QDir(gTestPath).absoluteFilePath("rename12To45Basic");
    const QSet<QString>& occupied =                  //
        RenameNamesUnique::getOccupiedPostPath(pre,  //
                                               {"", ""}, {"1", "2"}, true);
    const QSet<QString> expect{"1", "2"};
    QCOMPARE(occupied, expect);
  }

  void test_get_occupied_name_ok_include_subdir() {
    const QString pre = QDir(gTestPath).absoluteFilePath("renameDiretoryNameShouldNotConflict");
    const QSet<QString>& occupied =                  //
        RenameNamesUnique::getOccupiedPostPath(pre,  //
                                               {"dirName", "dirName"}, {"dirName", "file1"}, true);
    const QSet<QString> expect{"dirName", "dirName/file1", "dirName/dirName"};
    QCOMPARE(occupied, expect);
  }

  void test_rename12To45_no_conflict_basic() {
    const QSet<QString> occupiedNames{"1", "2"};
    QStringList m_conflictNames;
    bool ans = RenameNamesUnique::CheckConflict(occupiedNames,         //
                                                {"", ""}, {"1", "2"},  //
                                                {"4", "5"}, m_conflictNames);
    QVERIFY(ans);
  }

  void test_rename12To23_conflict_basic() {
    const QSet<QString> occupiedNames{"1", "2"};
    QStringList m_conflictNames;
    bool ans = RenameNamesUnique::CheckConflict(occupiedNames,         //
                                                {"", ""}, {"1", "2"},  //
                                                {"2", "3"}, m_conflictNames);
    QVERIFY(!ans);
  }

  void test_rename21To32_not_conflict_once_swap_sequence() {
    const QSet<QString> occupiedNames{"1", "2"};
    QStringList m_conflictNames;
    bool ans = RenameNamesUnique::CheckConflict(occupiedNames,         //
                                                {"", ""}, {"2", "1"},  //
                                                {"3", "2"}, m_conflictNames);
    QVERIFY(ans);
  }

  void test_rename21To23_not_conflict_name_occupied_but_remain_unchange() {
    const QSet<QString> occupiedNames{"1", "2"};
    QStringList m_conflictNames;
    bool ans = RenameNamesUnique::CheckConflict(occupiedNames,         //
                                                {"", ""}, {"2", "1"},  //
                                                {"2", "3"}, m_conflictNames);
    QVERIFY(ans);
  }

  void occupiedNameOfrenameDiretoryNameShouldNotConflict() {
    const QString pre = QDir(gTestPath).absoluteFilePath("renameDiretoryNameShouldNotConflict");
    const QSet<QString>& occupied = RenameNamesUnique::getOccupiedPostPath(pre, {"", "dirName", "dirName"}, {"dirName", "file1", "dirName"}, true);
    const QSet<QString> actual{"dirName", "dirName/file1", "dirName/dirName"};
    QCOMPARE(occupied, actual);
  }

  void test_rename_diretory_name_not_conflict() {
    // dirName/file1, dirName
    // dir Name/file 1, dir Name
    QStringList m_conflictNames;
    bool ans = RenameNamesUnique::CheckConflict(                      //
        {"dirName", "dirName/file1", "dirName/dirName"},              //
        {"", "dirName", "dirName"}, {"dirName", "file1", "dirName"},  //
        {"dir Name", "file 1", "dir Name"},                           //
        m_conflictNames);
    QVERIFY(ans);
  }

  void test_getRenameCommands_one_level_basic() {
    RenameNamesUnique sameLevelRenameCheck{"C:/home",
                                           {"", "", "", ""},  //
                                           {"a", "B", "c", "d"},
                                           {".mp4", ".md", ".jpg", ""},  //
                                           {"A", "b", "c", "d"},
                                           {".mp4", ".md", ".jpg", ".cpp"},
                                           false};
    QCOMPARE(sameLevelRenameCheck.m_isArrLenUnequal, false);
    QCOMPARE(sameLevelRenameCheck.m_nameLineEmpty, false);
    QCOMPARE((bool)sameLevelRenameCheck, true);  // no conflict

    using namespace FileOperatorType;
    const BATCH_COMMAND_LIST_TYPE& actualCmds = sameLevelRenameCheck.getRenameCommands();
    // a.mp4 => A.mp4
    // B.md => b.mp4
    // c.jpg =>c.jpg remains unchaned, skip this one
    // d => d.cpp
    QCOMPARE(actualCmds.size(), 3);
    BATCH_COMMAND_LIST_TYPE expectAns  //
        {
            ACMD::GetInstRENAME("C:/home", "d", "C:/home", "d.cpp"),      //
            ACMD::GetInstRENAME("C:/home", "B.md", "C:/home", "b.md"),    //
            ACMD::GetInstRENAME("C:/home", "a.mp4", "C:/home", "A.mp4"),  //
        };                                                                //
    QCOMPARE(actualCmds, expectAns);
  }

  void test_getRenameCommands_two_level_basic() {
    RenameNamesUnique sameLevelRenameCheck{"C:/home",
                                           {"", "path", "path"},  //
                                           {"path", "A.mp4", "b.mp4"},
                                           {"", "", ""},  //
                                           {"PATH", "A.mp4", "B.MP4"},
                                           {"", "", ""},
                                           false};
    QCOMPARE(sameLevelRenameCheck.m_isArrLenUnequal, false);
    QCOMPARE(sameLevelRenameCheck.m_nameLineEmpty, false);
    QCOMPARE((bool)sameLevelRenameCheck, true);  // no conflict

    using namespace FileOperatorType;
    const BATCH_COMMAND_LIST_TYPE& actualCmds = sameLevelRenameCheck.getRenameCommands();
    // "C:/home/ path"=>"C:/home/ PATH"
    // "C:/home/ path/A.mp4"=>"C:/home/ path/A.mp4" remains unchaned, skip this one
    // "C:/home/ path/b.mp4"=>"C:/home/ path/B.MP4"
    QCOMPARE(actualCmds.size(), 2);
    BATCH_COMMAND_LIST_TYPE expectAns  //
        {
            ACMD::GetInstRENAME("C:/home", "path/b.mp4", "C:/home", "path/B.MP4"),  //
            ACMD::GetInstRENAME("C:/home", "path", "C:/home", "PATH"),              //
        };                                                                          //
    QCOMPARE(actualCmds, expectAns);
  }

  void test_getRenameCommands_user_clear_some_name_by_accident() {
    RenameNamesUnique invalidInputNameRnu{"C:/home", {""},      //
                                          {"a"},     {".mp4"},  //
                                          {""},      {".mp4"}, false};
    QCOMPARE(invalidInputNameRnu.m_nameLineEmpty, true);
    QCOMPARE(invalidInputNameRnu.isInputValid(), false);
    QCOMPARE((bool)invalidInputNameRnu, false);
  }

  void test_getRenameCommands_user_delete_some_line_by_accident() {
    RenameNamesUnique invalidInputLengthRnu{"C:/home",         //
                                            {""},      {"a"},  //
                                            {".mp4"},  {"a"}, {}, false};
    QCOMPARE(invalidInputLengthRnu.m_isArrLenUnequal, true);
    QCOMPARE(invalidInputLengthRnu.isInputValid(), false);
    QCOMPARE((bool)invalidInputLengthRnu, false);
  }
};

#include "RenameUnqiueCheckTest.moc"
RenameUnqiueCheckTest g_RenameUnqiueCheckTest;
