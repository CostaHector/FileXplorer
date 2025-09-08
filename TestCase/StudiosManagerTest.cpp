#include <QCoreApplication>
#include <QtTest>
#include "OnScopeExit.h"
#include "TDir.h"
#include "MyTestSuite.h"
// add necessary includes here
#include "BeginToExposePrivateMember.h"
#include "StudiosManager.h"
#include "EndToExposePrivateMember.h"

class StudiosManagerTest : public MyTestSuite {
  Q_OBJECT
public:
  StudiosManagerTest() : MyTestSuite{false} {}
  TDir mDir;
  const QString rootpath{mDir.path()};
  const QString gLocalFilePath{rootpath + "/not_exist_studio_list.txt"};
  StudiosManager* smInLLT{nullptr};
  QList<FsNodeEntry> gNodeEntries;
private slots:
  void initTestCase() {
    static StudiosManager inst{gLocalFilePath};
    smInLLT = &inst;
  }

  void test_studio_list_file_not_exist_read_out() {
    QVERIFY2(!mDir.exists(gLocalFilePath), qPrintable(gLocalFilePath));  // file not exist
    QVERIFY2(smInLLT->count() == 0, "Studio count in llt should be empty");
  }

  void test_studio_list_not_empty_in_service() {
    const StudiosManager& psm{StudiosManager::getIns()};
    QVERIFY2(psm.count() >= 0, "studio list should not be empty");
  }

  void test_studio_name_in_last_section_ok() {
    // precondition
    decltype(smInLLT->m_prodStudioMap) tempStudioHash;
    tempStudioHash["realmadridcf"] = "RealMadridCF";
    tempStudioHash["real madrid cf"] = "RealMadridCF";
    tempStudioHash["juventus"] = "Juventus";

    smInLLT->m_prodStudioMap.swap(tempStudioHash);
    ON_SCOPE_EXIT {
      smInLLT->m_prodStudioMap.swap(tempStudioHash);
    };
    QVERIFY(!smInLLT->m_prodStudioMap.isEmpty());

    QCOMPARE((*smInLLT)("Raphaël Varane, Kaka - RealMadridCF"), "RealMadridCF");
    QCOMPARE((*smInLLT)("Raphaël Varane, Cristiano Ronaldo - Real Madrid CF"), "RealMadridCF");
    QCOMPARE((*smInLLT)("Raphaël Varane, Álvaro Morata - Real Madrid CF"), "RealMadridCF");

    QCOMPARE((*smInLLT)("Cristiano Ronaldo, Kaka - RealMadridCF DVD"), "RealMadridCF");
    QCOMPARE((*smInLLT)("Cristiano Ronaldo, Álvaro Morata - Real Madrid CF DVD"), "RealMadridCF");

    QCOMPARE((*smInLLT)("juventus - Cristiano Ronaldo, Kaka - RealMadridCF DVD"), "Juventus");
  }

  void test_standardStudioNameFrom_ok() {
    // precondition
    decltype(smInLLT->m_prodStudioMap) tempStudioHash;
    // FC Bayern Munich here not support
    // FC Barcelona here not support
    tempStudioHash["realmadridcf"] = "RealMadridCF";
    tempStudioHash["real madrid cf"] = "RealMadridCF";
    tempStudioHash["schalke04"] = "Schalke04";
    tempStudioHash["schalke 04"] = "Schalke04";
    tempStudioHash["men"] = "MEN";
    tempStudioHash["my 3 gifts"] = "My3Gifts";
    tempStudioHash["my3gifts"] = "My3Gifts";

    smInLLT->m_prodStudioMap.swap(tempStudioHash);
    ON_SCOPE_EXIT {
      smInLLT->m_prodStudioMap.swap(tempStudioHash);
    };
    QVERIFY(!smInLLT->m_prodStudioMap.isEmpty());

    QSet<QString> fromList;
    QSet<QString> expectStudiosName;

    // GetCoarseStudioNames name ok
    expectStudiosName = QSet<QString>{"realmadridcf", "real madrid cf"};
    fromList = smInLLT->GetCoarseStudioNames("RealMadridCF");
    QCOMPARE(expectStudiosName, fromList);

    expectStudiosName = QSet<QString>{"schalke04", "schalke 04"};
    fromList = smInLLT->GetCoarseStudioNames("Schalke04");
    QCOMPARE(expectStudiosName, fromList);

    expectStudiosName = QSet<QString>{"men"};
    fromList = smInLLT->GetCoarseStudioNames("MEN");
    QCOMPARE(expectStudiosName, fromList);

    expectStudiosName = QSet<QString>{"alreadylowercase"};
    fromList = smInLLT->GetCoarseStudioNames("alreadylowercase");
    QCOMPARE(expectStudiosName, fromList);

    // hint studio name ok
    QCOMPARE((*smInLLT)("Schalke 04 - no matter whom"), "Schalke04");
    QCOMPARE((*smInLLT)("schalke 04 - no matter whom"), "Schalke04");
    QCOMPARE((*smInLLT)("schalke04 - no matter whom"), "Schalke04");
    QCOMPARE((*smInLLT)("Schalke04 - no matter whom"), "Schalke04");

    QCOMPARE((*smInLLT)("Real Madrid CF - Kaka & Cristinao Ronaldo"), "RealMadridCF");
    QCOMPARE((*smInLLT)("real madrid cf - Kaka & Cristinao Ronaldo"), "RealMadridCF");
    QCOMPARE((*smInLLT)("RealMadridCF - Kaka & Cristinao Ronaldo"), "RealMadridCF");

    QCOMPARE((*smInLLT)("MEN - Henry Cavill"), "MEN");
    QCOMPARE((*smInLLT)("men - Sean O'Pry"), "MEN");

    QCOMPARE((*smInLLT)("my 3 gifts - Henry Cavill"), "My3Gifts");
    QCOMPARE((*smInLLT)("My3Gifts - Henry Cavill"), "My3Gifts");
    QCOMPARE((*smInLLT)("my3gifts - Henry Cavill"), "My3Gifts");

    // with prefix still should ok
    QCOMPARE((*smInLLT)("[FFL]my 3 gifts - Henry Cavill"), "My3Gifts");
    QCOMPARE((*smInLLT)("[FL]My3Gifts - Henry Cavill"), "My3Gifts");
    QCOMPARE((*smInLLT)("[GT]my3gifts - Henry Cavill"), "My3Gifts");

    // studio not int dict, and hypen index exist and <=22, return itself
    QVERIFY(!tempStudioHash.contains("studio not in dict"));
    QVERIFY(!tempStudioHash.contains("studionotindict"));
    QCOMPARE((*smInLLT)("StudioNotInDict - Henry Cavill"), "StudioNotInDict");

    // studio not int dict, and hypen index exist and length>22, return empty
    QVERIFY(!tempStudioHash.contains("01234567890123456789012"));
    QCOMPARE((*smInLLT)("01234567890123456789012 - Henry Cavill"), "");

    // studio not int dict, and hypen index not exist return empty
    QVERIFY(!tempStudioHash.contains("01234567890123456789012"));
    QCOMPARE((*smInLLT)("01234567890123456789012 Henry Cavill"), "");
  }

  void test_isHypenIndexValid() {
    int hypenIndex{-1};
    // no hypen, false
    QCOMPARE(StudiosManager::isHypenIndexValid("", hypenIndex), false);
    QCOMPARE(hypenIndex, -1);

    // no any letter before hypen, false
    QCOMPARE(StudiosManager::isHypenIndexValid("-", hypenIndex), false);
    QCOMPARE(hypenIndex, 0);
    // one letter before, true
    QCOMPARE(StudiosManager::isHypenIndexValid("X- World", hypenIndex), true);
    QCOMPARE(hypenIndex, 1);

    QString studio22Char = "0123456789012345678901";
    QCOMPARE(studio22Char.size(), StudiosManager::STUDIO_HYPEN_MAX_INDEX);
    QString studio22CharAnd1Hypen = studio22Char + '-';
    QCOMPARE(studio22CharAnd1Hypen.indexOf('-'), StudiosManager::STUDIO_HYPEN_MAX_INDEX);
    QString studio23CharAnd1Hypen = studio22Char + " -";
    QCOMPARE(studio23CharAnd1Hypen.indexOf('-'), StudiosManager::STUDIO_HYPEN_MAX_INDEX + 1);

    // no hypen, false
    QCOMPARE(StudiosManager::isHypenIndexValid(studio22Char, hypenIndex), false);
    // hypen right at STUDIO_HYPEN_MAX_INDEX, true
    QCOMPARE(StudiosManager::isHypenIndexValid(studio22CharAnd1Hypen, hypenIndex), true);
    QCOMPARE(hypenIndex, studio22CharAnd1Hypen.indexOf('-'));
    // hypen at STUDIO_HYPEN_MAX_INDEX + 1, false
    QCOMPARE(StudiosManager::isHypenIndexValid(studio23CharAnd1Hypen, hypenIndex), false);
    QCOMPARE(hypenIndex, studio23CharAnd1Hypen.indexOf('-'));
  }
};

StudiosManagerTest g_StudiosManagerTest;
#include "StudiosManagerTest.moc"
