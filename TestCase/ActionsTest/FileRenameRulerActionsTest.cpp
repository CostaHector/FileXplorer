#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "FileRenameRulerActions.h"
#include "StudiosManager.h"
#include "CastManager.h"
#include "CastAkasManager.h"
#include "EndToExposePrivateMember.h"

#include "PathTool.h"
#include "PublicTool.h"
#include "FileToolMock.h"
#include "TDir.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class FileRenameRulerActionsTest : public PlainTestSuite {
  Q_OBJECT
 public:
  QString mFileNameEdit;
  TDir mTDir;
  const QString mStudiosFileName = "Studios.txt";
  const QString mActorsFileName = "Actors.txt";
  const QString mActorsAliasFileName = "ActorsAlias.txt";
  const QString mStudiosListFile = mTDir.itemPath(mStudiosFileName);
  const QString mActorsListFile = mTDir.itemPath(mActorsFileName);
  const QString mActorsAliasListFile = mTDir.itemPath(mActorsAliasFileName);
  const QList<FsNodeEntry> mFileNodes{{mStudiosFileName, false, ""},  //
                                      {mActorsFileName, false, ""},   //
                                      {mActorsAliasFileName, false, ""}};
  StudiosManager& studiosMgr = StudiosManager::getInst();
  CastManager& actorsMgr = CastManager::getInst();
  CastAkasManager& actorAliasMgr = CastAkasManager::getInst();

 private slots:
  void initTestCase() {
    GlobalMockObject::reset();
    using namespace FileToolMock;
    MOCKER(FileTool::OpenLocalFileUsingDesktopService)  //
        .stubs()                                        //
        .with(spy(mFileNameEdit))                       //
        .will(invoke(invokeOpenLocalFileUsingDesktopService));
    MOCKER(PathTool::FILE_REL_PATH::GetVendorsTableFilePath).stubs().will(returnValue(mStudiosListFile));
    MOCKER(PathTool::FILE_REL_PATH::GetActorsListFilePath).stubs().will(returnValue(mActorsListFile));
    MOCKER(PathTool::FILE_REL_PATH::GetActorsAliasListFilePath).stubs().will(returnValue(mActorsAliasListFile));

    QVERIFY(mTDir.IsValid());
    QCOMPARE(mTDir.createEntries(mFileNodes), mFileNodes.size());

    studiosMgr.InitializeImpl(mStudiosListFile);
    actorsMgr.InitializeImpl(mActorsListFile);
    actorAliasMgr.InitializeImpl(mActorsAliasListFile);
  }

  void cleanupTestCase() {  //
    GlobalMockObject::verify();
  }

  void name_ruler_edit_dependent_file_open_ok() {
    FileRenameRulerActions& ruleInst = g_NameRulerActions();
    QVERIFY(ruleInst.NAME_RULES_ACTIONS_LIST.size() > 0);

    // 0. inexist file
    QVERIFY(!ruleInst.onEditLocalFile("inexist relative file"));

    // 1. Studios edit/reload
    QCOMPARE(studiosMgr.ProStudioMap().size(), 0);
    QVERIFY(FileRenameRulerActions::onEditStudiosListFile());
    QCOMPARE(mFileNameEdit, mStudiosListFile);
    emit ruleInst._EDIT_STUDIOS->triggered();
    mTDir.touch(mStudiosFileName,
                "20th century studios\t20CenturyStudios\n"
                "20th Century Studios\t20CenturyStudios\n");
    QCOMPARE(FileRenameRulerActions::onReloadStudiosListFile(), 2);
    QCOMPARE(studiosMgr.ProStudioMap().size(), 2);
    mTDir.touch(mStudiosFileName,
                "paramount pictures\tParamountPictures\n"
                "paramountpictures\tParamountPictures\n"
                "20th century studios\t20CenturyStudios\n"
                "20thcenturystudios\t20CenturyStudios\n");
    emit ruleInst._RELOAD_STUDIOS->triggered();
    QCOMPARE(studiosMgr.ProStudioMap().size(), 2 + 2);

    // 2. Actors edit/reload
    QCOMPARE(actorsMgr.CastSet().size(), 0);
    QVERIFY(FileRenameRulerActions::onEditActorsListFile());
    QCOMPARE(mFileNameEdit, mActorsListFile);
    emit ruleInst._EDIT_ACTORS->triggered();
    mTDir.touch(mActorsFileName,
                "chris evans\n"
                "michael fassbender\n"
                "chris hemsworth\n");
    QCOMPARE(FileRenameRulerActions::onReloadActorsListFile(), 3);
    QCOMPARE(actorsMgr.CastSet().size(), 3);
    mTDir.touch(mActorsFileName, "cristiano ronaldo\n");
    emit ruleInst._RELOAD_ACTORS->triggered();
    QCOMPARE(actorsMgr.CastSet().size(), 1);

    // 3. Actors Alias edit/reload
    QCOMPARE(actorAliasMgr.CastAkaMap().size(), 0);
    QVERIFY(FileRenameRulerActions::onEditActorsAliasListFile());
    QCOMPARE(mFileNameEdit, mActorsAliasListFile);
    emit ruleInst._EDIT_ACTORS_ALIAS->triggered();
    mTDir.touch(mActorsAliasFileName,
                "Cristiano Ronaldo,CR7\n"
                "Ricardo Leite,Kaka\n");
    QCOMPARE(FileRenameRulerActions::onReloadActorsAliasListFile(), 2+2);
    QCOMPARE(actorAliasMgr.CastAkaMap().size(), 2+2);
    mTDir.touch(mActorsAliasFileName,
                "Chris Evans,Steve,Captain\n"
                "Cristiano Ronaldo,CR7\n"
                "Ricardo Leite,Kaka\n");
    emit ruleInst._RELOAD_ACTORS_ALIAS->triggered();
    QCOMPARE(actorAliasMgr.CastAkaMap().size(), 3+2+2);

    // 4. statistsics
    FileRenameRulerActions::onShowRenameRuleStatistics();
    emit ruleInst._RENAME_RULE_STAT->triggered();
  }
};

#include "FileRenameRulerActionsTest.moc"
REGISTER_TEST(FileRenameRulerActionsTest, false)
