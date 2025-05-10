#include <QCoreApplication>
#include <QtTest>
#include "TestCase/pub/MyTestSuite.h"
#include "public/PublicMacro.h"

// add necessary includes here
#include "Component/JsonEditorWidget/LineEditCSV.h"

class LineEditCSVTest : public MyTestSuite {
  Q_OBJECT

 public:
  LineEditCSVTest() : MyTestSuite{false} {}
 private slots:
  void test_hotSceneSL() {
    LineEditCSV uniqueEleLe{ENUM_TO_STRING(CAST), "", true};
    uniqueEleLe.setText("0,25,20,5");

    QVariantList expectVarLst{0, 5, 20, 25};
    QCOMPARE(uniqueEleLe.GetVariantList(), expectVarLst);

    const QVariantList specifyReadLst{0, 25, 7};
    const QVariantList expectLst{0, 7, 25};
    QCOMPARE(uniqueEleLe.ReadFromVariantList(specifyReadLst), 3);
    QCOMPARE(uniqueEleLe.GetVariantList(), expectLst);
  }

  void test_LineEditCSV_unique() {  //
    const QStringList noConflictElement{"A", "B", "C"};
    LineEditCSV uniqueEleLe{ENUM_TO_STRING(CAST), "", true};
    uniqueEleLe.setText("A, B, B, C");
    QCOMPARE(uniqueEleLe.GetStringList(), noConflictElement);
    QCOMPARE(uniqueEleLe.GetFormName(), ENUM_TO_STRING(CAST));
    uniqueEleLe.clear();

    uniqueEleLe.ReadFromStringList(noConflictElement);
    QCOMPARE(uniqueEleLe.text(), "A,B,C");
    // each item already exists
    QCOMPARE(uniqueEleLe.AppendFromStringList(noConflictElement), noConflictElement.size());
    QCOMPARE(uniqueEleLe.GetStringList(), noConflictElement);

    QStringList expectFinalLst{"0", "A", "B", "C"};  // will auto sort when append called
    QCOMPARE(uniqueEleLe.AppendFromStringList(QStringList{"0"}), expectFinalLst.size());
    QCOMPARE(uniqueEleLe.GetStringList(), expectFinalLst);

    // space trimmed
    const QStringList castLst                                                //
        {" Jobs",          "Jobs ",         " Jobs ",        "Jobs",         //
         "Chris Evans ",   " Chris Evans",  " Chris Evans ", "Chris Evans",  //
         " Henry Cavill ", " Henry Cavill", "Henry Cavill ", "Henry Cavill"};
    const QStringList trimmedExpectAns{"Chris Evans", "Henry Cavill", "Jobs"};
    QCOMPARE(uniqueEleLe.ReadFromStringList(castLst), trimmedExpectAns.size());
    QCOMPARE(uniqueEleLe.GetStringList(), trimmedExpectAns);
  }

  void test_LineEditCSV_can_conflict() {  //
    const QStringList conflictElement{"A", "B", "B", "C"};
    LineEditCSV canDuplicateEleLe{ENUM_TO_STRING(CAST), "", false};
    canDuplicateEleLe.setText("A, B, B, C");
    QCOMPARE(canDuplicateEleLe.GetStringList(), conflictElement);
    QCOMPARE(canDuplicateEleLe.GetFormName(), ENUM_TO_STRING(CAST));
    canDuplicateEleLe.clear();

    canDuplicateEleLe.ReadFromStringList(conflictElement);
    QCOMPARE(canDuplicateEleLe.text(), "A,B,B,C");

    // each item already exists, no unique check, double element count
    QStringList expectFinalLst{"A", "A", "B", "B", "B", "B", "C", "C"};  // will auto sort when append called
    QCOMPARE(canDuplicateEleLe.AppendFromStringList(conflictElement), expectFinalLst.size());
    QCOMPARE(canDuplicateEleLe.GetStringList(), expectFinalLst);
  }
};

LineEditCSVTest g_LineEditCSVTest;
#include "LineEditCSVTest.moc"
