#include <QCoreApplication>
#include <QtTest>
#include "TestCase/pub/GlbDataProtect.h"
#include "TestCase/pub/OnScopeExit.h"
#include "TestCase/PathRelatedTool.h"
#include "Tools/JsonFileHelper.h"
#include "pub/MyTestSuite.h"

// add necessary includes here
#include "Tools/CastManager.h"
#include "Tools/StudiosManager.h"
#include "public/PublicMacro.h"
const QString rootpath = TestCaseRootPath() + "/test/TestEnv_JsonCastStudio";

class CastManagerTest : public MyTestSuite {
  Q_OBJECT
 public:
  CastManagerTest() : MyTestSuite{false}, pm{CastManager::getIns()} {}
  const CastManager& pm;

 private slots:
  void test_performersDictNotEmpty() {  //
    QVERIFY2(!pm.m_performers.isEmpty(), "performers list should not be empty");
  }
  void test_sentenceSplit() {
    const auto& wordsList = pm.SplitSentence("Production Name - Movie Core Name - Nice & Devin Franco BB 4K");
    QVERIFY2(not wordsList.isEmpty(), "words list should not empty");
  }

  void test_filterOutNameFromWordsList() {
    const auto& wordsList = pm.SplitSentence({"Jesse Theo Brady Devin Franco Vincent O'Reilly Topher Di Maggio Aspen"});
    const auto& perfsList = pm.FilterPerformersOut(wordsList);
    const QSet<QString>& perfsSet{perfsList.cbegin(), perfsList.cend()};
    QVERIFY2(perfsList.size() >= 4, "perfs list should contains >= 5 performers");
    // 2 word cast are allowed, 1 word cast not allowed
    const QSet<QString>& realPerfs{"Theo Brady", "Devin Franco", "Vincent O'Reilly", "Topher Di Maggio"};
    QCOMPARE(perfsSet, realPerfs);
  }

  void test_OneLetterXNameCommaAddSep() {
    QVERIFY(pm.m_performers.contains("gabriel clark"));
    QVERIFY(pm.m_performers.contains("cutler x"));
    QVERIFY(pm.m_performers.contains("theo brady"));

    const auto& wordsList = pm.SplitSentence({"Gabriel Clark, Cutler X + Theo Brady"});
    const auto& perfsList = pm.FilterPerformersOut(wordsList);
    const QStringList expectCasts{"Gabriel Clark", "Cutler X", "Theo Brady"};
    QCOMPARE(perfsList, expectCasts);
  }

  void test_cast_emdwith_comma_Period_colon_exclamation_question() {
    QVERIFY(pm.m_performers.contains("gabriel clark"));
    QVERIFY(pm.m_performers.contains("cutler x"));
    QVERIFY(pm.m_performers.contains("theo brady"));
    QVERIFY(pm.m_performers.contains("britishtwunk"));
    const auto& wordsList = pm.SplitSentence({"Gabriel Clark! Cutler X. Theo Brady; britishtwunk?"});
    const auto& perfsList = pm.FilterPerformersOut(wordsList);
    const QStringList expectCasts{"Gabriel Clark", "Cutler X", "Theo Brady", "britishtwunk"};
    QCOMPARE(perfsList, expectCasts);
  }

  void test_sentenceWithPerfsAtLast() {
    QVERIFY(pm.m_performers.contains("britishtwunk"));
    QVERIFY(pm.m_performers.contains("craig kennedy"));
    const auto& wordsList = pm.SplitSentence("britishtwunk fucked by Craig Kennedy");
    QVERIFY2(!wordsList.isEmpty(), "words list should not empty");

    const auto& perfsList = pm.FilterPerformersOut(wordsList);
    const QStringList expectCasts{"britishtwunk", "Craig Kennedy"};
    QCOMPARE(perfsList, expectCasts);
  }

  void test_sentenceWithNewLineSeperator() {
    QVERIFY(pm.m_performers.contains("britishtwunk"));
    QVERIFY(pm.m_performers.contains("craig kennedy"));
    QVERIFY(pm.m_performers.contains("gabriel clark"));
    const auto& wordsList = pm.SplitSentence("britishtwunk\r\nCraig Kennedy\nGabriel Clark");
    QVERIFY2(!wordsList.isEmpty(), "words list should not empty");

    const auto& perfsList = pm.FilterPerformersOut(wordsList);
    const QStringList expectCasts{"britishtwunk", "Craig Kennedy", "Gabriel Clark"};
    QCOMPARE(perfsList, expectCasts);
  }

  void test_LearningFromAPath_no_write_localfile() {
    // precondition
    QDir dir{rootpath};
    const QString jsonAbsFile{dir.absoluteFilePath("My Good Boy.json")};
    QVERIFY(QFile::exists(jsonAbsFile));

    using namespace JSON_KEY;
    const QVariantHash& dict = JsonFileHelper::MovieJsonLoader(jsonAbsFile);

    const auto studioIt = dict.constFind(ENUM_TO_STRING(Studio));
    QVERIFY(studioIt != dict.cend());
    const QString& studioName{"StudioNotExist"};
    // studio not exist, studionotexist => StudioNotExist
    QHash<QString, QString> expectStudioSet{{"studionotexist", "StudioNotExist"},  //
                                            {"studio not exist", "StudioNotExist"}};
    QCOMPARE(studioIt.value().toString(), studioName);

    const auto castIt = dict.constFind(ENUM_TO_STRING(Cast));
    QVERIFY(castIt != dict.cend());
    const QStringList& castLst{"Cast1NotExist", "Cast2NotExist"};
    QSet<QString> expectCastSet{"cast1notexist", "cast2notexist"};
    QCOMPARE(castIt.value().toStringList(), castLst);

    // procedure
    auto& studioInst = StudiosManager::getIns();
    QHash<QString, QString> emptyStudioSet;
    studioInst.m_prodStudioMap.swap(emptyStudioSet);
    ON_SCOPE_EXIT {
      studioInst.m_prodStudioMap.swap(emptyStudioSet);
    };

    auto& castInst = CastManager::getIns();
    QSet<QString> emptyCastSet;
    castInst.m_performers.swap(emptyCastSet);
    ON_SCOPE_EXIT {
      castInst.m_performers.swap(emptyCastSet);
    };

    // cached should updated, local file should not write
    bool studioLocalFileWrite{true};
    QCOMPARE(studioInst.LearningFromAPath(rootpath, false, &studioLocalFileWrite), 2);
    QCOMPARE(studioInst.m_prodStudioMap, expectStudioSet);
    QCOMPARE(studioLocalFileWrite, false);

    bool castLocalFileWrite{true};
    QCOMPARE(castInst.LearningFromAPath(rootpath, false, &castLocalFileWrite), 2);
    QCOMPARE(castInst.m_performers, expectCastSet);
    QCOMPARE(castLocalFileWrite, false);

    // do it again, nothing new should append
    studioLocalFileWrite = true;
    QCOMPARE(studioInst.LearningFromAPath(rootpath, false, &studioLocalFileWrite), 0);
    QCOMPARE(studioInst.m_prodStudioMap, expectStudioSet);
    QCOMPARE(studioLocalFileWrite, false);

    castLocalFileWrite = true;
    QCOMPARE(castInst.LearningFromAPath(rootpath, false, &castLocalFileWrite), 0);
    QCOMPARE(castInst.m_performers, expectCastSet);
    QCOMPARE(castLocalFileWrite, false);
  }
};

#include "CastManagerTest.moc"
CastManagerTest g_CastManagerTest;
