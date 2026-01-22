#include <QCoreApplication>
#include <QtTest>
#include "TDir.h"
#include "JsonKey.h"
#include "JsonHelper.h"
#include "PlainTestSuite.h"

// add necessary includes here
#include "BeginToExposePrivateMember.h"
#include "CastManager.h"
#include "StudiosManager.h"
#include "EndToExposePrivateMember.h"
#include "JsonTestPrecoditionTools.h"
#include "PublicMacro.h"
#include "PathTool.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class CastManagerTest : public PlainTestSuite {
  Q_OBJECT
 public:
  CastManagerTest() : PlainTestSuite{} {}
  TDir mTDir;
  const QString rootpath{mTDir.path()};

  const QString mStudiosFileName = "Studios.txt";
  const QString mActorsFileName = "Actors.txt";
  const QString mStudiosListFile = mTDir.itemPath(mStudiosFileName);
  const QString mActorsListFile = mTDir.itemPath(mActorsFileName);

  const QString mStudiosBlackFileName = "StudiosBlack.txt";
  const QString mActorsBlackFileName = "ActorsBlack.txt";
  const QString mStudiosBlackListFile = mTDir.itemPath(mStudiosBlackFileName);
  const QString mActorsBlackListFile = mTDir.itemPath(mActorsBlackFileName);

  const QString mJsonFileName = "My Good Boy.json";
  const QString mJsonFile = mTDir.itemPath(mJsonFileName);

  QList<FsNodeEntry> gNodeEntries{
      FsNodeEntry{mStudiosFileName, false, ""},                                    //
      FsNodeEntry{mActorsFileName, false, ""},                                     //
      FsNodeEntry{mStudiosBlackFileName, false, ""},                               //
      FsNodeEntry{mActorsBlackFileName, false, ""},                                //
      FsNodeEntry{mJsonFileName, false, JsonTestPrecoditionTools::JSON_CONTENTS},  //
      FsNodeEntry{"SuperMan - Henry Cavill 1.jpg", false, {}},                     //
      FsNodeEntry{"SuperMan - Henry Cavill 999.mp4", false, {}},                   //
      FsNodeEntry{"SuperMan - Henry Cavill.jpg", false, {}},                       //
      FsNodeEntry{"SuperMan - Henry Cavill.json", false, {}},
  };
  CastManager& actorMgr = CastManager::getInst();
  StudiosManager& studioMgr = StudiosManager::getInst();

 private slots:
  void initTestCase() {
    GlobalMockObject::reset();
    using namespace PathTool::FILE_REL_PATH;
    MOCKER(GetStudiosListFilePath).stubs().will(returnValue(mStudiosListFile));
    MOCKER(GetStudiosBlackListFilePath).stubs().will(returnValue(mStudiosBlackListFile));
    MOCKER(GetActorsListFilePath).stubs().will(returnValue(mActorsListFile));
    MOCKER(GetActorsBlackListFilePath).stubs().will(returnValue(mActorsBlackListFile));

    QVERIFY(mTDir.IsValid());
    QCOMPARE(mTDir.createEntries(gNodeEntries), gNodeEntries.size());

    actorMgr.InitializeImpl(GetActorsListFilePath(), GetActorsBlackListFilePath());
    QVERIFY(actorMgr.CastSet().isEmpty());

    studioMgr.InitializeImpl(GetStudiosListFilePath(), GetStudiosBlackListFilePath());
    QVERIFY(studioMgr.ProStudioMap().isEmpty());
  }

  void cleanupTestCase() {
    GlobalMockObject::verify();
  }

  void test_FilterPerformersOut_with_blacklist() {
    // procedure
    QVERIFY(actorMgr.CastSet().isEmpty());
    mTDir.touch(mActorsFileName,
                "matt dallas\n"
                "chris pine\n"
                "jensen ackles\n"
                "@captain\n"
                "alex\n"   // in blacklist, ignored
                "steve\n"  // in blacklist, ignored
                "abc\n"    // length shorter then 12 and no special char contains, ignored
    );
    mTDir.touch(mActorsBlackFileName,
                "adam\n"
                "adonis\n"
                "adrian\n"
                "alan\n"
                "alex\n"
                "andy\n"
                "antonio\n"
                "beau\n"
                "ben\n"
                "blake\n"
                "bradley\n"
                "brody\n"
                "cameron\n"
                "justin\n"
                "steve\n");
    QCOMPARE(actorMgr.ForceReloadImpl(), 4 - 0);

    const QStringList expectCastList{"Matt Dallas", "Chris Pine", "Jensen Ackles", "@Captain"};
    const QStringList actualCastList =
        actorMgr("The quick brown fox jumps over the lazy dog alex - Matt Dallas - Chris Pine and Jensen Ackles, @Captain.");
    QCOMPARE(actualCastList, expectCastList);
  }

  void test_filterOutNameFromWordsList() {
    // in FilterPerformersOut: not support now: 4 word, only: 3 word, 2 word, 1 word support
    mTDir.touch(mActorsFileName,
                "jean le rond d'alembert\n"
                "frenkie de jong\n"
                "l hospital\n"
                "james\n"  // length shorter then 12 and no special char contains, ignored
    );
    mTDir.touch(mActorsBlackFileName, "");
    actorMgr.ForceReloadImpl();
    QCOMPARE(actorMgr.CastSet().size(), 3);

    QCOMPARE(actorMgr.SplitSentence(""), (QStringList{}));        // empty string in empty list out
    QCOMPARE(actorMgr.FilterPerformersOut({}), (QStringList{}));  // empty string in empty list out

    // single quote not used to split
    // and/And(insensitive) used to split
    // continous /\ used to split
    const QString sentence{"Jean le Rond d'Alembert AND Frenkie de Jong///L Hospital\\\\James"};
    const QStringList& expectWordSection{"Jean", "le", "Rond", "d'Alembert", "Frenkie", "de", "Jong", "L", "Hospital", "James"};
    const auto& actualWordsList = actorMgr.SplitSentence(sentence);
    QCOMPARE(actualWordsList, expectWordSection);

    const auto& actualCast1 = actorMgr.FilterPerformersOut(actualWordsList);
    const auto& actualCast2 = actorMgr(sentence);
    const QStringList expectCastList{"Frenkie de Jong", "L Hospital"};

    QCOMPARE(actualCast1, expectCastList);
    QCOMPARE(actualCast2, expectCastList);
  }

  void test_with_actor_seperated_in_punctuation() {
    mTDir.touch(mActorsFileName,
                "matt dallas\n"
                "chris pine\n"
                "jensen ackles\n");
    mTDir.touch(mActorsBlackFileName, "");
    actorMgr.ForceReloadImpl();
    QCOMPARE(actorMgr.CastSet().size(), 3);

    const QStringList expectCastList{"Matt Dallas", "Chris Pine", "Jensen Ackles"};
    const QStringList ansOfCommaSep = actorMgr("Matt Dallas, Chris Pine, Jensen Ackles");
    const QStringList ansOfNewLineSep = actorMgr("Matt Dallas \n Chris Pine \r\n Jensen Ackles");
    const QStringList ansOfColonSep = actorMgr("Matt Dallas; Chris Pine; Jensen Ackles");
    const QStringList ansOfTabSep = actorMgr("Matt Dallas \t Chris Pine\tJensen Ackles");
    const QStringList ansOfPeriodSep = actorMgr("Matt Dallas. Chris Pine. Jensen Ackles");
    QCOMPARE(ansOfCommaSep, expectCastList);
    QCOMPARE(ansOfNewLineSep, expectCastList);
    QCOMPARE(ansOfColonSep, expectCastList);
    QCOMPARE(ansOfTabSep, expectCastList);
    QCOMPARE(ansOfPeriodSep, expectCastList);
  }

  void test_LearningFromAPath_write_local_file_succeed() {
    mTDir.touch(mActorsFileName, "");
    mTDir.touch(mActorsBlackFileName, "");
    mTDir.touch(mStudiosListFile, "");
    mTDir.touch(mStudiosBlackListFile, "");
    actorMgr.ForceReloadImpl();
    studioMgr.ForceReloadImpl();
    QVERIFY(actorMgr.CastSet().isEmpty());
    QVERIFY(studioMgr.ProStudioMap().isEmpty());

    // studio not exist, studionotexist => StudioNotExist
    const QString& studioName{"StudioNotExist"};
    QHash<QString, QString> expectStudioSet{{"studionotexist", "StudioNotExist"},  //
                                            {"studio not exist", "StudioNotExist"}};

    using namespace JsonKey;
    const QVariantHash& dict = JsonHelper::MovieJsonLoader(mJsonFile);
    const auto studioIt = dict.constFind(ENUM_2_STR(Studio));
    QVERIFY(studioIt != dict.cend());
    QCOMPARE(studioIt.value().toString(), studioName);

    const auto castIt = dict.constFind(ENUM_2_STR(Cast));
    QVERIFY(castIt != dict.cend());
    const QStringList& castLst{"Cast1NotExist", "Cast2NotExist"};
    QSet<QString> expectCastSet{"cast1notexist", "cast2notexist"};
    QCOMPARE(castIt.value().toStringList(), castLst);

    // cached should updated, local file should not write
    bool studioLocalFileWrite{false};
    QCOMPARE(studioMgr.LearningFromAPath(rootpath, &studioLocalFileWrite), 2);
    QCOMPARE(studioMgr.ProStudioMap(), expectStudioSet);
    QCOMPARE(studioLocalFileWrite, true);
    const QFile fiStudio{mStudiosListFile};
    QVERIFY(fiStudio.size() > 0);

    bool castLocalFileWrite{false};
    QCOMPARE(actorMgr.LearningFromAPath(rootpath, &castLocalFileWrite), 2);
    QCOMPARE(actorMgr.CastSet(), expectCastSet);
    QCOMPARE(castLocalFileWrite, true);
    const QFile fiCast{mActorsListFile};
    QVERIFY(fiCast.size() > 0);

    // do it again, nothing new should append
    studioLocalFileWrite = true;
    QCOMPARE(studioMgr.LearningFromAPath(rootpath, &studioLocalFileWrite), 0);
    QCOMPARE(studioMgr.ProStudioMap(), expectStudioSet);
    QCOMPARE(studioLocalFileWrite, false);  // skipped, no write

    castLocalFileWrite = true;
    QCOMPARE(actorMgr.LearningFromAPath(rootpath, &castLocalFileWrite), 0);
    QCOMPARE(actorMgr.CastSet(), expectCastSet);
    QCOMPARE(castLocalFileWrite, false);  // skipped, no write
  }
};

#include "CastManagerTest.moc"
REGISTER_TEST(CastManagerTest, false)
