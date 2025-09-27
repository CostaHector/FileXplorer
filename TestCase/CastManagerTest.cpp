#include <QCoreApplication>
#include <QtTest>
#include "GlbDataProtect.h"
#include "OnScopeExit.h"
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

class CastManagerTest : public PlainTestSuite {
  Q_OBJECT
 public:
  CastManagerTest() : PlainTestSuite{} {}
  TDir mDir;
  const QString rootpath{mDir.path()};
  const QString gLocalFilePath{rootpath + "/cast_list.txt"};
  const QString gStudioLocalFilePath{rootpath + "/studio_list.txt"};
  QList<FsNodeEntry> gNodeEntries;
  CastManager* cmInLLT{nullptr};
  StudiosManager* smInLLT{nullptr};
 private slots:
  void initTestCase() {
    // 1. at first cast_list.txt and studio_list.txt file not exist
    CastManager& instCast = CastManager::getInst();
    {
      instCast.InitializeImpl(gLocalFilePath);
      QVERIFY(instCast.CastSet().isEmpty());
    }
    StudiosManager& instStudio = StudiosManager::getInst();
    {
      instStudio.InitializeImpl(gStudioLocalFilePath);
      QVERIFY(instStudio.ProStudioMap().isEmpty());
    }

    QVERIFY(mDir.IsValid());
    gNodeEntries = QList<FsNodeEntry>  //
        {
            FsNodeEntry{"cast_list.txt", false, "any random cast1\nanother random cast2"},    //
            FsNodeEntry{"My Good Boy.json", false, JsonTestPrecoditionTools::JSON_CONTENTS},  //
            FsNodeEntry{"studio_list.txt", false, "any random studio\tAnyRandomStudio"},      //
            FsNodeEntry{"SuperMan - Henry Cavill 1.jpg", false, {}},                          //
            FsNodeEntry{"SuperMan - Henry Cavill 999.mp4", false, {}},                        //
            FsNodeEntry{"SuperMan - Henry Cavill.jpg", false, {}},                            //
            FsNodeEntry{"SuperMan - Henry Cavill.json", false, {}},
        };
    QCOMPARE(mDir.createEntries(gNodeEntries), gNodeEntries.size());

    // 2. ResetStateForTestImpl ok
    {
      instCast.ResetStateForTestImpl(gLocalFilePath);
      QCOMPARE(instCast.CastSet().size(), 2); // 2 elements
      cmInLLT = &instCast;
    }
    {
      instStudio.ResetStateForTestImpl(gStudioLocalFilePath);
      QCOMPARE(instStudio.ProStudioMap().size(), 1); // 1 element
      smInLLT = &instStudio;
    }
  }

  void test_sentenceSplit() {
    // precondition
    const QStringList expectCastList{"Matt Dallas", "Chris Pine", "Jensen Ackles"};
    CAST_MGR_DATA_T tempPerfs;
    for (const QString& star : expectCastList) {
      tempPerfs.insert(star.toLower());
    }
    cmInLLT->CastSet().swap(tempPerfs);
    ON_SCOPE_EXIT {
      cmInLLT->CastSet().swap(tempPerfs);
    };
    QVERIFY(!cmInLLT->CastSet().isEmpty());

    // procedure
    const QStringList& actualCastList = (*cmInLLT)("Matt Dallas - Chris Pine and Jensen Ackles.");
    QCOMPARE(actualCastList, expectCastList);
  }

  void test_filterOutNameFromWordsList() {
    // precondition
    const QStringList allCastList{"Jean le Rond d'Alembert", "Frenkie de Jong", "L Hospital", "James"};
    // 4 word(not support now), 3 word, 2 word, 1 word
    const QStringList expectCastList{"Frenkie de Jong", "L Hospital", "James"};
    CAST_MGR_DATA_T tempPerfs;
    for (const QString& star : allCastList) {
      tempPerfs.insert(star.toLower());
    }
    cmInLLT->CastSet().swap(tempPerfs);
    ON_SCOPE_EXIT {
      cmInLLT->CastSet().swap(tempPerfs);
    };
    QVERIFY(!cmInLLT->CastSet().isEmpty());

    // single quote not used to split
    // and/And(insensitive) used to split
    // continous /\ used to split
    const QString sentence{"Jean le Rond d'Alembert AND Frenkie de Jong///L Hospital\\\\James"};
    const QStringList& expectWordSection{"Jean", "le", "Rond", "d'Alembert", "Frenkie", "de", "Jong", "L", "Hospital", "James"};

    QCOMPARE(cmInLLT->SplitSentence(""), (QStringList{})); // empty string in empty list out

    const auto& actualWordsList = cmInLLT->SplitSentence(sentence);
    QCOMPARE(actualWordsList, expectWordSection);

    QCOMPARE(cmInLLT->FilterPerformersOut({}), (QStringList{})); // empty string in empty list out
    const auto& actualCast1 = cmInLLT->FilterPerformersOut(actualWordsList);
    const auto& actualCast2 = (*cmInLLT)(sentence);
    QCOMPARE(actualCast1, actualCast2);

    QCOMPARE(actualCast1, expectCastList);
  }

  void test_one_char_seperator() {
    // precondition
    const QStringList expectCastList{"U", "V", "W", "X", "Y", "Z"};
    CAST_MGR_DATA_T tempPerfs;
    for (const QString& star : expectCastList) {
      tempPerfs.insert(star.toLower());
    }
    cmInLLT->CastSet().swap(tempPerfs);
    ON_SCOPE_EXIT {
      cmInLLT->CastSet().swap(tempPerfs);
    };
    QVERIFY(!cmInLLT->CastSet().isEmpty());

    const auto& perfsList = (*cmInLLT)("U! + V; / W. \\\\ X & Y and Z?");
    QCOMPARE(perfsList, expectCastList);
  }

  void test_with_new_line_seperator() {
    // precondition
    const QStringList expectCastList{"Matt Dallas", "Chris Pine", "Jensen Ackles"};
    CAST_MGR_DATA_T tempPerfs;
    for (const QString& star : expectCastList) {
      tempPerfs.insert(star.toLower());
    }
    cmInLLT->CastSet().swap(tempPerfs);
    ON_SCOPE_EXIT {
      cmInLLT->CastSet().swap(tempPerfs);
    };
    QVERIFY(!cmInLLT->CastSet().isEmpty());

    const auto& perfsList = (*cmInLLT)("Matt Dallas \n Chris Pine \r\n Jensen Ackles");
    QCOMPARE(perfsList, expectCastList);
  }

  void test_LearningFromAPath_write_local_file_succeed() {
    // precondition
    QDir dir{rootpath};
    const QString jsonAbsFile{dir.absoluteFilePath("My Good Boy.json")};
    QVERIFY(QFile::exists(jsonAbsFile));

    using namespace JsonKey;
    const QVariantHash& dict = JsonHelper::MovieJsonLoader(jsonAbsFile);

    const auto studioIt = dict.constFind(ENUM_2_STR(Studio));
    QVERIFY(studioIt != dict.cend());
    const QString& studioName{"StudioNotExist"};
    // studio not exist, studionotexist => StudioNotExist
    QHash<QString, QString> expectStudioSet{{"studionotexist", "StudioNotExist"},  //
                                            {"studio not exist", "StudioNotExist"}};
    QCOMPARE(studioIt.value().toString(), studioName);

    const auto castIt = dict.constFind(ENUM_2_STR(Cast));
    QVERIFY(castIt != dict.cend());
    const QStringList& castLst{"Cast1NotExist", "Cast2NotExist"};
    QSet<QString> expectCastSet{"cast1notexist", "cast2notexist"};
    QCOMPARE(castIt.value().toStringList(), castLst);

    // procedure
    QHash<QString, QString> emptyStudioSet;
    smInLLT->ProStudioMap().swap(emptyStudioSet);
    ON_SCOPE_EXIT {
      smInLLT->ProStudioMap().swap(emptyStudioSet);
    };

    QSet<QString> emptyCastSet;
    cmInLLT->CastSet().swap(emptyCastSet);
    ON_SCOPE_EXIT {
      cmInLLT->CastSet().swap(emptyCastSet);
    };

    // cached should updated, local file should not write
    bool studioLocalFileWrite{false};
    QCOMPARE(smInLLT->LearningFromAPath(rootpath, &studioLocalFileWrite), 2);
    QCOMPARE(smInLLT->ProStudioMap(), expectStudioSet);
    QCOMPARE(studioLocalFileWrite, true);
    const QFile fiStudio{gStudioLocalFilePath};
    QVERIFY(fiStudio.size() > 0);

    bool castLocalFileWrite{false};
    QCOMPARE(cmInLLT->LearningFromAPath(rootpath, &castLocalFileWrite), 2);
    QCOMPARE(cmInLLT->CastSet(), expectCastSet);
    QCOMPARE(castLocalFileWrite, true);
    const QFile fiCast{gLocalFilePath};
    QVERIFY(fiCast.size() > 0);

    // do it again, nothing new should append
    studioLocalFileWrite = true;
    QCOMPARE(smInLLT->LearningFromAPath(rootpath, &studioLocalFileWrite), 0);
    QCOMPARE(smInLLT->ProStudioMap(), expectStudioSet);
    QCOMPARE(studioLocalFileWrite, false);  // skipped, no write

    castLocalFileWrite = true;
    QCOMPARE(cmInLLT->LearningFromAPath(rootpath, &castLocalFileWrite), 0);
    QCOMPARE(cmInLLT->CastSet(), expectCastSet);
    QCOMPARE(castLocalFileWrite, false);  // skipped, no write
  }
};

#include "CastManagerTest.moc"
REGISTER_TEST(CastManagerTest, false)
