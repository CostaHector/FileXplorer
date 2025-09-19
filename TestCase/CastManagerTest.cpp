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
    CastManager& instCast = CastManager::getInst();
    instCast.InitializeImpl(gLocalFilePath);
    cmInLLT = &instCast;

    StudiosManager& instStudio = StudiosManager::getInst();
    instStudio.ResetStateForTestImpl(gStudioLocalFilePath);
    smInLLT = &instStudio;

    static const char JSON_CONTENTS[]{R"({
    "Bitrate": "",
    "Cast": [
        "Cast1NotExist",
        "Cast2NotExist"
    ],
    "Detail": "This is just a json example.",
    "Duration": 0,
    "Hot": [
    ],
    "Name": "My Good Boy",
    "Rate": 4,
    "Resolution": "720p",
    "Size": "126113854",
    "Studio": "StudioNotExist",
    "Tags": [
        "nonporn"
    ],
    "Uploaded": "20231022"
}
)"};
    QVERIFY(mDir.IsValid());
    gNodeEntries = QList<FsNodeEntry>  //
        {
         FsNodeEntry{"cast_list.txt", false, {}},                    //
         FsNodeEntry{"My Good Boy.json", false, JSON_CONTENTS},      //
         FsNodeEntry{"studio_list.txt", false, {}},                  //
         FsNodeEntry{"SuperMan - Henry Cavill 1.jpg", false, {}},    //
         FsNodeEntry{"SuperMan - Henry Cavill 999.mp4", false, {}},  //
         FsNodeEntry{"SuperMan - Henry Cavill.jpg", false, {}},      //
         FsNodeEntry{"SuperMan - Henry Cavill.json", false, {}},
         };
    mDir.createEntries(gNodeEntries);
  }
  void cleanup() {
    QFile fiCast{gLocalFilePath};
    if (fiCast.size() > 0) {  // clear file
      fiCast.resize(0);
    }
    QFile fiStudio{gStudioLocalFilePath};
    if (fiStudio.size() > 0) {  // clear file
      fiStudio.resize(0);
    }
  }

  void test_cast_list_file_empty_nothing_read_out() {
    const QFile castListFi{gLocalFilePath};
    QVERIFY2(castListFi.exists(), qPrintable(gLocalFilePath));  // file exist
    QCOMPARE(castListFi.size(), 0);                             // empty file
    QVERIFY2(cmInLLT->count() == 0, "Cast count in llt should be empty");
  }

  void test_cast_list_not_empty_in_service() {
    const CastManager& pmInsService{CastManager::getInst()};
    QVERIFY2(pmInsService.count() >= 0, "Cast count in service should greater than 0");
  }

  void test_sentenceSplit() {
    // precondition
    const QStringList expectCastList{"Matt Dallas", "Chris Pine", "Jensen Ackles"};
    decltype(cmInLLT->m_casts) tempPerfs;
    for (const QString& star : expectCastList) {
      tempPerfs.insert(star.toLower());
    }
    cmInLLT->m_casts.swap(tempPerfs);
    ON_SCOPE_EXIT {
      cmInLLT->m_casts.swap(tempPerfs);
    };
    QVERIFY(!cmInLLT->m_casts.isEmpty());

    // procedure
    const QStringList& actualCastList = (*cmInLLT)("Matt Dallas - Chris Pine and Jensen Ackles.");
    QCOMPARE(actualCastList, expectCastList);
  }

  void test_filterOutNameFromWordsList() {
    // precondition
    const QStringList allCastList{"Jean le Rond d'Alembert", "Frenkie de Jong", "L Hospital", "James"};
    // 4 word(not support now), 3 word, 2 word, 1 word
    const QStringList expectCastList{"Frenkie de Jong", "L Hospital", "James"};
    decltype(cmInLLT->m_casts) tempPerfs;
    for (const QString& star : allCastList) {
      tempPerfs.insert(star.toLower());
    }
    cmInLLT->m_casts.swap(tempPerfs);
    ON_SCOPE_EXIT {
      cmInLLT->m_casts.swap(tempPerfs);
    };
    QVERIFY(!cmInLLT->m_casts.isEmpty());

    // single quote not used to split
    // and/And(insensitive) used to split
    // continous /\ used to split
    const QString sentence{"Jean le Rond d'Alembert AND Frenkie de Jong///L Hospital\\\\James"};
    const QStringList& expectWordSection{"Jean", "le", "Rond", "d'Alembert", "Frenkie", "de", "Jong", "L", "Hospital", "James"};

    const auto& actualWordsList = cmInLLT->SplitSentence(sentence);
    QCOMPARE(actualWordsList, expectWordSection);

    const auto& actualCast1 = cmInLLT->FilterPerformersOut(actualWordsList);
    const auto& actualCast2 = (*cmInLLT)(sentence);
    QCOMPARE(actualCast1, actualCast2);

    QCOMPARE(actualCast1, expectCastList);
  }

  void test_one_char_seperator() {
    // precondition
    const QStringList expectCastList{"U", "V", "W", "X", "Y", "Z"};
    decltype(cmInLLT->m_casts) tempPerfs;
    for (const QString& star : expectCastList) {
      tempPerfs.insert(star.toLower());
    }
    cmInLLT->m_casts.swap(tempPerfs);
    ON_SCOPE_EXIT {
      cmInLLT->m_casts.swap(tempPerfs);
    };
    QVERIFY(!cmInLLT->m_casts.isEmpty());

    const auto& perfsList = (*cmInLLT)("U! + V; / W. \\\\ X & Y and Z?");
    QCOMPARE(perfsList, expectCastList);
  }

  void test_with_new_line_seperator() {
    // precondition
    const QStringList expectCastList{"Matt Dallas", "Chris Pine", "Jensen Ackles"};
    decltype(cmInLLT->m_casts) tempPerfs;
    for (const QString& star : expectCastList) {
      tempPerfs.insert(star.toLower());
    }
    cmInLLT->m_casts.swap(tempPerfs);
    ON_SCOPE_EXIT {
      cmInLLT->m_casts.swap(tempPerfs);
    };
    QVERIFY(!cmInLLT->m_casts.isEmpty());

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
    cmInLLT->m_casts.swap(emptyCastSet);
    ON_SCOPE_EXIT {
      cmInLLT->m_casts.swap(emptyCastSet);
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
    QCOMPARE(cmInLLT->m_casts, expectCastSet);
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
    QCOMPARE(cmInLLT->m_casts, expectCastSet);
    QCOMPARE(castLocalFileWrite, false);  // skipped, no write
  }
};

#include "CastManagerTest.moc"
REGISTER_TEST(CastManagerTest, false)
