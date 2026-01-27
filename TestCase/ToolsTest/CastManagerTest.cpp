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
#include "StudioActorManagerTestHelper.h"
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

  ActorManagerTestHelper actorHelper{mTDir};
  StudioManagerTestHelper studioHelper{mTDir};

  const QString mJsonFileName = "My Good Boy.json";
  const QString mJsonFile = mTDir.itemPath(mJsonFileName);

  QList<FsNodeEntry> gNodeEntries{
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

    QVERIFY(mTDir.IsValid());
    QCOMPARE(mTDir.createEntries(gNodeEntries), gNodeEntries.size());

    actorHelper.init();
    studioHelper.init();
  }

  void cleanupTestCase() {  //
    GlobalMockObject::verify();
  }

  void test_FilterPerformersOut_with_blacklist() {
    // procedure
    QVERIFY(actorMgr.CastSet().isEmpty());
    actorHelper.setFileContents(
        "matt dallas\n"
        "chris pine\n"
        "jensen ackles\n"
        "@captain\n"
        "alex\n"   // in blacklist, ignored
        "steve\n"  // in blacklist, ignored
        "abc\n"    // length shorter then 12 and no special char contains, ignored
        ,
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
    QCOMPARE(actorMgr.CastSet().size(), 4);
    const QStringList expectCastList{"@Captain", "Chris Pine", "Jensen Ackles", "Matt Dallas"};
    const QStringList actualCastList =
        actorMgr("The quick brown fox jumps over the lazy dog alex - Matt Dallas - Chris Pine and Jensen Ackles, @Captain.", false);
    QCOMPARE(actualCastList, expectCastList);
  }

  void test_filterOutNameFromWordsList() {
    // in FilterPerformersOut: not support now: 4 word, only: 3 word, 2 word, 1 word support
    actorHelper.setFileContents(
        "jean le rond d'alembert\n"
        "frenkie de jong\n"
        "l hospital\n"
        "james\n"  // length shorter then 12 and no special char contains, ignored
        ,
        "");
    QCOMPARE(actorMgr.CastSet().size(), 3);

    QCOMPARE(actorMgr.SplitSentence(""), (QStringList{}));        // empty string in empty list out
    QCOMPARE(actorMgr.FilterPerformersOut({}, false), (QStringList{}));  // empty string in empty list out

    // single quote not used to split
    // and/And(insensitive) used to split
    // continous /\ used to split
    const QString sentence{"Jean le Rond d'Alembert AND Frenkie de Jong///L Hospital\\\\James"};
    const QStringList& expectWordSection{"Jean", "le", "Rond", "d'Alembert", "Frenkie", "de", "Jong", "L", "Hospital", "James"};
    const auto& actualWordsList = actorMgr.SplitSentence(sentence);
    QCOMPARE(actualWordsList, expectWordSection);

    const auto& actualCast1 = actorMgr.FilterPerformersOut(actualWordsList, false);
    const auto& actualCast2 = actorMgr(sentence);
    const QStringList expectCastList{"Frenkie de Jong", "L Hospital"};

    QCOMPARE(actualCast1, expectCastList);
    QCOMPARE(actualCast2, expectCastList);
  }

  void test_with_actor_seperated_in_punctuation() {
    actorHelper.setFileContents(
        "matt dallas\n"
        "chris pine\n"
        "jensen ackles\n",
        "");
    QCOMPARE(actorMgr.CastSet().size(), 3);

    const QStringList expectCastList{"Chris Pine", "Jensen Ackles", "Matt Dallas"};
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
};

#include "CastManagerTest.moc"
REGISTER_TEST(CastManagerTest, false)
