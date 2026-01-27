#include <QCoreApplication>
#include <QtTest>
#include "OnScopeExit.h"
#include "TDir.h"
#include "PlainTestSuite.h"
// add necessary includes here
#include "BeginToExposePrivateMember.h"
#include "StudiosManager.h"
#include "EndToExposePrivateMember.h"
#include "PathTool.h"
#include "StudioActorManagerTestHelper.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class StudiosManagerTest : public PlainTestSuite {
  Q_OBJECT
 public:
  StudiosManagerTest() : PlainTestSuite{} {}
  TDir mTDir;
  const QString rootpath{mTDir.path()};

  StudioManagerTestHelper studioHelper{mTDir};
  StudiosManager& studioMgr = StudiosManager::getInst();
 private slots:
  void initTestCase() {
    QVERIFY(mTDir.IsValid());

    GlobalMockObject::reset();
    studioHelper.init();
  }
  void cleanupTestCase() {  //
    GlobalMockObject::verify();
  }

  void InitializeImpl_ok() {
    using namespace PathTool::FILE_REL_PATH;
    studioMgr.InitializeImpl("inexist studio list file.txt");
    QVERIFY2(studioMgr.count() == 0, "read from inexist file should return 0 items");

    studioMgr.InitializeImpl(GetVendorsTableFilePath(), GetMononymVendorsListFilePath());
    QVERIFY2(studioMgr.count() == 0, "read from empty file should return 0 items");
  }

  void test_studio_name_in_last_section_ok() {
    // precondition
    QCOMPARE(studioMgr.count(), 0);
    studioHelper.setFileContents(
        "realmadridcf\tRealMadridCF\n"
        "real madrid cf\tRealMadridCF\n"
        "juventus\tJuventus\n"
        "random studio\tRandomStudio",
        "random studio\n" // 单名厂商
        "randomstudio\n");
    QCOMPARE(studioMgr.count(), 4);

    QCOMPARE(studioMgr("Raphaël Varane, Kaka - RealMadridCF"), "RealMadridCF");
    QCOMPARE(studioMgr("Raphaël Varane, Cristiano Ronaldo - Real Madrid CF"), "RealMadridCF");
    QCOMPARE(studioMgr("Raphaël Varane, Álvaro Morata - Real Madrid CF"), "RealMadridCF");

    QCOMPARE(studioMgr("Cristiano Ronaldo, Kaka - RealMadridCF DVD"), "RealMadridCF");
    QCOMPARE(studioMgr("Cristiano Ronaldo, Álvaro Morata - Real Madrid CF DVD"), "RealMadridCF");

    QCOMPARE(studioMgr("juventus - Cristiano Ronaldo, Kaka - RealMadridCF DVD"), "Juventus");
    QCOMPARE(studioMgr("Random Studio - whatever contents here"), "RandomStudio");

    // 不在表中的厂商
    QCOMPARE(studioMgr("Bayern Munich - Thomas Müller, Robert Lewandowski"), "");
  }

  void test_standardStudioNameFrom_ok() {
    studioHelper.setFileContents(
        "realmadridcf\tRealMadridCF\n"
        "real madrid cf\tRealMadridCF\n"
        "schalke04\tSchalke04\n"
        "schalke 04\tSchalke04\n"
        "men\tMEN\n"
        "my 3 gifts\tMy3Gifts\n"
        "my3gifts\tMy3Gifts\n",  // in blacklist, ignored
        "randon studio\nrandonstudio");
    QCOMPARE(studioMgr.count(), 7);

    // FC Bayern Munich here not support
    // FC Barcelona here not support
    QSet<QString> fromList;
    QSet<QString> expectStudiosName;

    // GetCoarseStudioNames name ok
    expectStudiosName = QSet<QString>{"realmadridcf", "real madrid cf"};
    fromList = studioMgr.GetCoarseStudioNames("RealMadridCF");
    QCOMPARE(expectStudiosName, fromList);

    expectStudiosName = QSet<QString>{"schalke04", "schalke 04"};
    fromList = studioMgr.GetCoarseStudioNames("Schalke04");
    QCOMPARE(expectStudiosName, fromList);

    expectStudiosName = QSet<QString>{"men"};
    fromList = studioMgr.GetCoarseStudioNames("MEN");
    QCOMPARE(expectStudiosName, fromList);

    expectStudiosName = QSet<QString>{"alreadylowercase"};
    fromList = studioMgr.GetCoarseStudioNames("alreadylowercase");
    QCOMPARE(expectStudiosName, fromList);

    // hint studio name ok
    QCOMPARE(studioMgr("Schalke 04 - no matter whom"), "Schalke04");
    QCOMPARE(studioMgr("schalke 04 - no matter whom"), "Schalke04");
    QCOMPARE(studioMgr("schalke04 - no matter whom"), "Schalke04");
    QCOMPARE(studioMgr("Schalke04 - no matter whom"), "Schalke04");

    QCOMPARE(studioMgr("Real Madrid CF - Kaka & Cristinao Ronaldo"), "RealMadridCF");
    QCOMPARE(studioMgr("real madrid cf - Kaka & Cristinao Ronaldo"), "RealMadridCF");
    QCOMPARE(studioMgr("RealMadridCF - Kaka & Cristinao Ronaldo"), "RealMadridCF");

    QCOMPARE(studioMgr("MEN - Henry Cavill"), "MEN");
    QCOMPARE(studioMgr("men - Sean O'Pry"), "MEN");

    QCOMPARE(studioMgr("my 3 gifts - Henry Cavill"), "My3Gifts");
    QCOMPARE(studioMgr("My3Gifts - Henry Cavill"), "My3Gifts");
    QCOMPARE(studioMgr("my3gifts - Henry Cavill"), "My3Gifts");

    // with prefix still should ok
    QCOMPARE(studioMgr("[FFL]my 3 gifts - Henry Cavill"), "My3Gifts");
    QCOMPARE(studioMgr("[FL]My3Gifts - Henry Cavill"), "My3Gifts");
    QCOMPARE(studioMgr("[GT]my3gifts - Henry Cavill"), "My3Gifts");

    // studio not int table, return ""
    QCOMPARE(studioMgr("StudioNotInDict - Henry Cavill"), "");
    QCOMPARE(studioMgr("01234567890123456789012 - Henry Cavill"), "");
    QCOMPARE(studioMgr("01234567890123456789012 Henry Cavill"), "");
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

#include "StudiosManagerTest.moc"
REGISTER_TEST(StudiosManagerTest, false)
