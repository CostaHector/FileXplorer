#include <QCoreApplication>
#include <QtTest>

// add necessary includes here
#include "Tools/ConflictsItemHelper.h"
#include "pub/FileSystemRelatedTest.h"
#include <QSet>

class ConflictsItemHelperTest : public FileSystemRelatedTest {
  Q_OBJECT
 public:
  ConflictsItemHelperTest() : FileSystemRelatedTest{"TestEnv_ConflictsItem"} {}

 private slots:
  void init() {
    // conflictFolder{conflict.mp4,not conflict video.mp4}
    // from{conflictFolder{conflict.mp4}, 1 conflict.txt, x noconflict.txt, y noconflict.txt}
    // 1 conflict.txt, z noconflict.txt
    m_rootHelper << FileSystemNode{"conflictFolder"}
                 << FileSystemNode{"from"}
                 << FileSystemNode{"1 conflict.txt", false, ""}
                 << FileSystemNode{"z noconflict.txt", false, ""};
    m_rootHelper.GetSubHelper("conflictFolder")
        << FileSystemNode{"conflict.mp4", false, ""}
        << FileSystemNode{"not conflict video.mp4", false, ""};
    m_rootHelper.GetSubHelper("from")
        << FileSystemNode{"1 conflict.txt", false, ""}
        << FileSystemNode{"x noconflict.txt", false, ""}
        << FileSystemNode{"y noconflict.txt", false, ""};
  }
  void test_enviromentMet() {
    QVERIFY(QDir(ROOT_DIR).exists("conflictFolder/conflict.mp4"));
    QVERIFY(QDir(ROOT_DIR).exists("conflictFolder/not conflict video.mp4"));
    QVERIFY(QDir(ROOT_DIR).exists("1 conflict.txt"));
    QVERIFY(QDir(ROOT_DIR).exists("z noconflict.txt"));
    QVERIFY(QDir(ROOT_DIR).exists("from/conflictFolder/conflict.mp4"));
    QVERIFY(QDir(ROOT_DIR).exists("from/1 conflict.txt"));
    QVERIFY(QDir(ROOT_DIR).exists("from/y noconflict.txt"));
  }

  void test_HelperConstructorsWhenCut() {
    const QSet<QString> expectConflicts{"conflictFolder/conflict.mp4", "conflictFolder", "1 conflict.txt"};
    CCMMode mode = CCMMode::CUT_OP;
    const QString l = QDir(ROOT_DIR).absoluteFilePath("from");
    const QString r = ROOT_DIR;
    const auto& conflictIF0 =
        ConflictsItemHelper(l, r, QDir(l, "", QDir::SortFlag::NoSort, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot).entryList(), mode);
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
    CCMMode mode = CCMMode::LINK_OP;
    const QString l = QDir(ROOT_DIR).absoluteFilePath("from");
    const QString r = ROOT_DIR;
    const auto& conflictIF0 =
        ConflictsItemHelper(l, r, QDir(l, "", QDir::SortFlag::NoSort, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot).entryList(), mode);
    QVERIFY2(not conflictIF0, "There must no conflicts");
    QSet<QString> actualSet0{conflictIF0.commonList.cbegin(), conflictIF0.commonList.cend()};
    QCOMPARE(actualSet0, expectConflicts);
  }
};

//QTEST_MAIN(ConflictsItemHelperTest)
#include "ConflictsItemHelperTest.moc"
