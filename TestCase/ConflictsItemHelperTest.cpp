#include <QCoreApplication>
#include <QtTest>

// add necessary includes here
#include "Tools/ConflictsItemHelper.h"
#include "pub/FileSystemRelatedTest.h"

#include <QSet>

class ConflictsItemHelperTest : public FileSystemRelatedTest {
  Q_OBJECT
 public:
  ConflictsItemHelperTest() : FileSystemRelatedTest{"TestEnv_ConflictsItem", "EDITABLE"} {}

 private slots:
  void test_enviromentMet() {
    QVERIFY(QDir(TEST_DIR).exists("conflictFolder/conflict.mp4"));
    QVERIFY(QDir(TEST_DIR).exists("conflictFolder/not conflict video.mp4"));
    QVERIFY(QDir(TEST_DIR).exists("1 conflict.txt"));
    QVERIFY(QDir(TEST_DIR).exists("z noconflict.txt"));
    QVERIFY(QDir(TEST_DIR).exists("from/conflictFolder/conflict.mp4"));
    QVERIFY(QDir(TEST_DIR).exists("from/1 conflict.txt"));
    QVERIFY(QDir(TEST_DIR).exists("from/y noconflict.txt"));
  }

  void test_HelperConstructorsWhenCut() {
    const QSet<QString> expectConflicts{"conflictFolder/conflict.mp4", "conflictFolder", "1 conflict.txt"};
    CCMMode mode = CCMMode::CUT;
    const QString l = QDir(TEST_DIR).absoluteFilePath("from");
    const QString r = TEST_DIR;
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
    CCMMode mode = CCMMode::LINK;
    const QString l = QDir(TEST_DIR).absoluteFilePath("from");
    const QString r = TEST_DIR;
    const auto& conflictIF0 =
        ConflictsItemHelper(l, r, QDir(l, "", QDir::SortFlag::NoSort, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot).entryList(), mode);
    QVERIFY2(not conflictIF0, "There must no conflicts");
    QSet<QString> actualSet0{conflictIF0.commonList.cbegin(), conflictIF0.commonList.cend()};
    QCOMPARE(actualSet0, expectConflicts);
  }
};

//QTEST_MAIN(ConflictsItemHelperTest)

//#include "ConflictsItemHelperTest.moc"
