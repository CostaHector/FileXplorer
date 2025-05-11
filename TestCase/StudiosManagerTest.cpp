#include <QCoreApplication>
#include <QtTest>
#include "TestCase/PathRelatedTool.h"
#include "TestCase/pub/OnScopeExit.h"
#include "pub/MyTestSuite.h"
// add necessary includes here
#include "pub/BeginToExposePrivateMember.h"
#include "Tools/StudiosManager.h"
#include "pub/EndToExposePrivateMember.h"

const QString rootpath = TestCaseRootPath() + "/test/TestEnv_JsonCastStudio";
const QString gLocalFilePath{rootpath + "/not_exist_studio_list.txt"};
class StudiosManagerTest : public MyTestSuite {
  Q_OBJECT
 public:
  StudiosManagerTest() : MyTestSuite{false}, smInLLT{gLocalFilePath} {}
  StudiosManager smInLLT;

 private slots:
  void cleanup() {
    QVERIFY(!QFile::exists(gLocalFilePath));
  }

  void test_studio_list_file_not_exist_read_out() {
    QVERIFY2(!QFile::exists(gLocalFilePath), qPrintable(gLocalFilePath));  // file not exist
    QVERIFY2(smInLLT.count() == 0, "Studio count in llt should be empty");
  }

  void test_studio_list_not_empty_in_service() {
    const StudiosManager& psm{StudiosManager::getIns()};
    QVERIFY2(psm.count() > 0, "studio list should not be empty");
  }

  void test_standardStudioNameFrom_ok() {
    // precondition
    decltype(smInLLT.m_prodStudioMap) tempStudioHash;
    // FC Bayern Munich here not support
    // FC Barcelona here not support
    tempStudioHash["realmadridcf"] = "RealMadridCF";
    tempStudioHash["real madrid cf"] = "RealMadridCF";
    tempStudioHash["schalke04"] = "Schalke04";
    tempStudioHash["schalke 04"] = "Schalke04";
    tempStudioHash["men"] = "MEN";
    tempStudioHash["my 3 gifts"] = "My3Gifts";
    tempStudioHash["my3gifts"] = "My3Gifts";

    smInLLT.m_prodStudioMap.swap(tempStudioHash);
    ON_SCOPE_EXIT {
      smInLLT.m_prodStudioMap.swap(tempStudioHash);
    };
    QVERIFY(!smInLLT.m_prodStudioMap.isEmpty());

    QSet<QString> fromList;
    QSet<QString> expectStudiosName;

    // GetCoarseStudioNames name ok
    expectStudiosName = QSet<QString>{"realmadridcf", "real madrid cf"};
    fromList = smInLLT.GetCoarseStudioNames("RealMadridCF");
    QCOMPARE(expectStudiosName, fromList);

    expectStudiosName = QSet<QString>{"schalke04", "schalke 04"};
    fromList = smInLLT.GetCoarseStudioNames("Schalke04");
    QCOMPARE(expectStudiosName, fromList);

    expectStudiosName = QSet<QString>{"men"};
    fromList = smInLLT.GetCoarseStudioNames("MEN");
    QCOMPARE(expectStudiosName, fromList);

    expectStudiosName = QSet<QString>{"alreadylowercase"};
    fromList = smInLLT.GetCoarseStudioNames("alreadylowercase");
    QCOMPARE(expectStudiosName, fromList);

    // hint studio name ok
    QCOMPARE(smInLLT("Schalke 04 - no matter whom"), "Schalke04");
    QCOMPARE(smInLLT("schalke 04 - no matter whom"), "Schalke04");
    QCOMPARE(smInLLT("schalke04 - no matter whom"), "Schalke04");
    QCOMPARE(smInLLT("Schalke04 - no matter whom"), "Schalke04");

    QCOMPARE(smInLLT("Real Madrid CF - Kaka & Cristinao Ronaldo"), "RealMadridCF");
    QCOMPARE(smInLLT("real madrid cf - Kaka & Cristinao Ronaldo"), "RealMadridCF");
    QCOMPARE(smInLLT("RealMadridCF - Kaka & Cristinao Ronaldo"), "RealMadridCF");

    QCOMPARE(smInLLT("MEN - Henry Cavill"), "MEN");
    QCOMPARE(smInLLT("men - Sean O'Pry"), "MEN");

    QCOMPARE(smInLLT("my 3 gifts - Henry Cavill"), "My3Gifts");
    QCOMPARE(smInLLT("My3Gifts - Henry Cavill"), "My3Gifts");
    QCOMPARE(smInLLT("my3gifts - Henry Cavill"), "My3Gifts");

    // with prefix still should ok
    QCOMPARE(smInLLT("[FFL]my 3 gifts - Henry Cavill"), "My3Gifts");
    QCOMPARE(smInLLT("[FL]My3Gifts - Henry Cavill"), "My3Gifts");
    QCOMPARE(smInLLT("[GT]my3gifts - Henry Cavill"), "My3Gifts");

    // studio not int dict, and length<=22, return itself
    QVERIFY(!tempStudioHash.contains("studio not in dict"));
    QVERIFY(!tempStudioHash.contains("studionotindict"));
    QCOMPARE(smInLLT("StudioNotInDict - Henry Cavill"), "StudioNotInDict");

    // studio not int dict, and length>22, return empty
    QVERIFY(!tempStudioHash.contains("01234567890123456789012"));
    QCOMPARE(smInLLT("01234567890123456789012 - Henry Cavill"), "");
  }
};

StudiosManagerTest g_StudiosManagerTest;
#include "StudiosManagerTest.moc"
