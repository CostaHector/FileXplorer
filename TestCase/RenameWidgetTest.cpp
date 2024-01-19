#include <QtTest>

// add necessary includes here
#include "Tools/RenameWidget.h"

class RenameCaseTest : public QObject {
  Q_OBJECT
 public:
 private slots:
  void init() {}
  void cleanup() {}

  void test_CapitalWeak(){
    QCOMPARE(RenameWidget_Case::CapitaliseEachWordFirstLetterOnly("henry cavill"), "Henry Cavill");
    QCOMPARE(RenameWidget_Case::CapitaliseEachWordFirstLetterOnly("HENRY CAVILL"), "HENRY CAVILL");
  }
  void test_CapitalStrong(){
    QCOMPARE(RenameWidget_Case::CapitaliseEachWordFirstLetterLowercaseOthers("henry cavill"), "Henry Cavill");
    QCOMPARE(RenameWidget_Case::CapitaliseEachWordFirstLetterLowercaseOthers("HENRY CAVILL"), "Henry Cavill");
  }
  void test_ToggleCase() {
    QCOMPARE(RenameWidget_Case::ToggleSentenceCase("Henry Cavill"), "hENRY cAVILL");
  }
};

QTEST_MAIN(RenameCaseTest)
#include "RenameWidgetTest.moc"
\
