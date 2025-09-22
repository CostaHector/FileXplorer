#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "BeginToExposePrivateMember.h"
#include "FileRenameRulerActions.h"
#include "EndToExposePrivateMember.h"
#include "PathTool.h"

class FileRenameRulerActionsTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void name_ruler_edit_dependent_file_open_ok() {
    FileRenameRulerActions& ruleInst = g_NameRulerActions();
    QVERIFY(ruleInst.NAME_RULES_ACTIONS_LIST.size() > 0);

    ruleInst.onEditLocalFile("inexist relative file");
    QCOMPARE(ruleInst.mLastTimeEditFileInfo.first, false);

    QString fileName;

    emit ruleInst._EDIT_STUDIOS->triggered();
    QCOMPARE(ruleInst.mLastTimeEditFileInfo.first, true);
    fileName = PathTool::FILE_REL_PATH::STANDARD_STUDIO_NAME;
    QVERIFY(ruleInst.mLastTimeEditFileInfo.second.endsWith(fileName.midRef(fileName.lastIndexOf('/'))));
    emit ruleInst._RELOAD_STUDIOS->triggered();
    QVERIFY(ruleInst.mLastTimeCntDelta >= 0);

    emit ruleInst._EDIT_PERFS->triggered();
    QCOMPARE(ruleInst.mLastTimeEditFileInfo.first, true);
    fileName = PathTool::FILE_REL_PATH::PERFORMERS_TABLE;
    QVERIFY(ruleInst.mLastTimeEditFileInfo.second.endsWith(fileName.midRef(fileName.lastIndexOf('/'))));
    emit ruleInst._RELOAD_PERFS->triggered();
    QVERIFY(ruleInst.mLastTimeCntDelta >= 0);

    emit ruleInst._EDIT_PERF_AKA->triggered();
    QCOMPARE(ruleInst.mLastTimeEditFileInfo.first, true);
    fileName = PathTool::FILE_REL_PATH::AKA_PERFORMERS;
    QVERIFY(ruleInst.mLastTimeEditFileInfo.second.endsWith(fileName.midRef(fileName.lastIndexOf('/'))));
    emit ruleInst._RELOAD_PERF_AKA->triggered();
    QVERIFY(ruleInst.mLastTimeCntDelta >= 0);

    emit ruleInst._RENAME_RULE_STAT->triggered();
  }
};

#include "FileRenameRulerActionsTest.moc"
REGISTER_TEST(FileRenameRulerActionsTest, false)
