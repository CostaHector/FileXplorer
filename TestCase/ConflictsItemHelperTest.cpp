#include <QCoreApplication>
#include <QtTest>

// add necessary includes here
#include "Tools/ConflictsItemHelper.h"
#include "pub/FileSystemTestSuite.h"
#include <QSet>

class ConflictsItemHelperTest : public FileSystemTestSuite {
  Q_OBJECT
 public:
  ConflictsItemHelperTest() : FileSystemTestSuite{"TestEnv_ConflictsItem", true, false} {}

 private slots:
  void init() {
    // conflictFolder{conflict.mp4,not conflict video.mp4}
    // from{conflictFolder{conflict.mp4}, 1 conflict.txt, x noconflict.txt, y noconflict.txt}
    // 1 conflict.txt
    // z noconflict.txt
    m_rootHelper << FileSystemNode{"conflictFolder"}                //
                 << FileSystemNode{"from"}                          //
                 << FileSystemNode{"1 conflict.txt", false, ""}     //
                 << FileSystemNode{"z noconflict.txt", false, ""};  //
    m_rootHelper.GetSubHelper("conflictFolder")                     //
        << FileSystemNode{"conflict.mp4", false, ""}                //
        << FileSystemNode{"not conflict video.mp4", false, ""};     //
    m_rootHelper.GetSubHelper("from")                               //
        << FileSystemNode{"conflictFolder"}                         //
        << FileSystemNode{"1 conflict.txt", false, ""}              //
        << FileSystemNode{"x noconflict.txt", false, ""}            //
        << FileSystemNode{"y noconflict.txt", false, ""};           //
    m_rootHelper.GetSubHelper("from/conflictFolder")                //
        << FileSystemNode{"conflict.mp4", false, ""};               //
  }

  void test_enviromentMet() {
    QVERIFY(QDir(mTestPath).exists("conflictFolder/conflict.mp4"));
    QVERIFY(QDir(mTestPath).exists("conflictFolder/not conflict video.mp4"));
    QVERIFY(QDir(mTestPath).exists("1 conflict.txt"));
    QVERIFY(QDir(mTestPath).exists("z noconflict.txt"));
    QVERIFY(QDir(mTestPath).exists("from/conflictFolder/conflict.mp4"));
    QVERIFY(QDir(mTestPath).exists("from/1 conflict.txt"));
    QVERIFY(QDir(mTestPath).exists("from/y noconflict.txt"));
  }

  void test_HelperConstructorsWhenCut() {
    const QSet<QString> expectConflicts{"conflictFolder/conflict.mp4", "conflictFolder", "1 conflict.txt"};
    CCMMode::Mode mode = CCMMode::CUT_OP;
    const QString l = QDir(mTestPath).absoluteFilePath("from");
    const QString r = mTestPath;
    const auto& conflictIF0 = ConflictsItemHelper(l, r, QDir(l, "", QDir::SortFlag::NoSort, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot).entryList(), mode);
    QVERIFY2(conflictIF0, "There must be some items conflicts");
    QSet<QString> actualSet0{conflictIF0.commonList.cbegin(), conflictIF0.commonList.cend()};
    QCOMPARE(actualSet0, expectConflicts);

    const auto& conflictIF1 = ConflictsItemHelper(l, r, mode);
    QVERIFY2(conflictIF1, "There must be some items conflicts");
    QSet<QString> actualSet1{conflictIF1.commonList.cbegin(), conflictIF1.commonList.cend()};
    QCOMPARE(actualSet1, expectConflicts);

    QList<QFileInfo> fList = QDir(l, "", QDir::SortFlag::NoSort, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot).entryInfoList();
    QStringList lAbsPath;
    for (auto& fi : fList) {
      lAbsPath.append(fi.absoluteFilePath());
    }
    const auto& conflictIF2 = ConflictsItemHelper(lAbsPath, r, mode);
    QVERIFY2(conflictIF2, "There must be some items conflicts");
    QSet<QString> actualSet2{conflictIF2.commonList.cbegin(), conflictIF2.commonList.cend()};
    QCOMPARE(actualSet2, expectConflicts);
  }

  void test_ConflictsWhenLink() {
    const QSet<QString> expectConflicts;
    CCMMode::Mode mode = CCMMode::LINK_OP;
    const QString l = QDir(mTestPath).absoluteFilePath("from");
    const QString r = mTestPath;
    const auto& conflictIF0 = ConflictsItemHelper(l, r, QDir(l, "", QDir::SortFlag::NoSort, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot).entryList(), mode);
    QVERIFY2(not conflictIF0, "There must no conflicts");
    QSet<QString> actualSet0{conflictIF0.commonList.cbegin(), conflictIF0.commonList.cend()};
    QCOMPARE(actualSet0, expectConflicts);
  }
};

#include "ConflictsItemHelperTest.moc"
ConflictsItemHelperTest g_ConflictsItemHelperTest;
