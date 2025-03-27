#include <QCoreApplication>
#include <QtTest>
#include "pub/MyTestSuite.h"

// add necessary includes here
#include "Tools/NameTool.h"

class NameToolTest : public MyTestSuite {
  Q_OBJECT

 public:
 private slots:
  void initTestCase() {}
  void cleanupTestCase() {}

  void init() {}
  void cleanup() {}

  void test_CommaAsFS() {
    const QString& s = "A , B";
    QCOMPARE(m_nameTool(s), (QStringList{"A", "B"}));
  }
  void test_VerticalBarAsFS() {
    const QString& s = "A | B";
    QCOMPARE(m_nameTool(s), (QStringList{"A", "B"}));
  }
  void test_AddAsFS() {
    const QString& s = "A + B";
    QCOMPARE(m_nameTool(s), (QStringList{"A", "B"}));
  }
  void test_AmpersandAsFS() {
    const QString& s = "A & B";
    QCOMPARE(m_nameTool(s), (QStringList{"A", "B"}));
  }
  void test_AndAsFS() {
    const QString& s = "A and B";
    QCOMPARE(m_nameTool(s), (QStringList{"A", "B"}));
  }
  void test_AndIgnoreCase() {
    const QString& s = "A And B";
    QCOMPARE(m_nameTool(s), (QStringList{"A", "B"}));
  }
  void test_AndAsSubStringOfNameRatherThanFS() {
    const QString& s = "Andrew Garfield and Andrew Tale";
    QCOMPARE(m_nameTool(s), (QStringList{"Andrew Garfield", "Andrew Tale"}));
  }
  void test_atAsFS() {
    const QString& s = "@A @B";
    QCOMPARE(m_nameTool(s), (QStringList{"A", "B"}));
  }
  void test_slashAsFS() {
    const QString& s = "A/B";
    QCOMPARE(m_nameTool(s), (QStringList{"A", "B"}));
  }
  void test_backSlashAsFS() {
    const QString& s = "A\\B";
    QCOMPARE(m_nameTool(s), (QStringList{"A", "B"}));
  }
  void test_tagAsFS() {
    const QString& s = "A#B";
    QCOMPARE(m_nameTool(s), (QStringList{"A", "B"}));
  }
  void test_tabAsFS() {
    const QString& s = "A\tB";
    QCOMPARE(m_nameTool(s), (QStringList{"A", "B"}));
  }
  void test_newlineAsFS() {
    const QString& s = "A\nB";
    QCOMPARE(m_nameTool(s), (QStringList{"A", "B"}));
  }
  void test_newlineAndCommaAsFS() {
    const QString& s = "A\n,B";
    QCOMPARE(m_nameTool(s), (QStringList{"A", "B"}));
  }
  void test_mostCommonPerfs() {
    const QString& s = "A, B, and C";
    QCOMPARE(m_nameTool(s), (QStringList{"A", "B", "C"}));
  }
  void test_humanNaturePerfs() {
    const QString& s = "A.M. and O'Clock";
    QCOMPARE(m_nameTool(s), (QStringList{"A.M.", "O'Clock"}));
  }
  void test_unexpectedSpace() {
    const QString& s = "A,B ,C&D &E|F |G#H #I@J @K/L /M\\N \\O";
    QCOMPARE(m_nameTool(s), (QStringList{"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O"}));
  }
  void test_duplicateNames() {
    const QString& s = "A, B, B";
    QCOMPARE(m_nameTool(s), (QStringList{"A", "B"}));
  }
  void test_sequenceStable() {
    const QString& s = "B, A";
    QCOMPARE(m_nameTool(s), (QStringList{"B", "A"}));
  }

  void test_2wordsCapitalizedName() {
    const QString& s = "AA BB and CC DD";
    QCOMPARE((QStringList{"Aa Bb", "Cc Dd"}), m_nameTool.fromArticleCapitalizedNames(s));
  }

  void test_3wordsCapitalizedName() {
    const QString& s = "AA BB CC, DD EE FF";
    QCOMPARE((QStringList{"Aa Bb Cc", "Dd Ee Ff"}), m_nameTool.fromArticleCapitalizedNames(s));
  }

  void test_invalid3wordCapitalizedName() {
    const QString& s = "A B'CC, DD E'F";  // should not process this one
    QVERIFY((QStringList{"A B'Cc", "Dd E'F"}) != m_nameTool.fromArticleCapitalizedNames(s));
  }

  void test_capitalizedNameWithApostrophe() {
    const QString& s = "AA B'CC, DD EE FF";
    QCOMPARE((QStringList{"Aa B'Cc", "Dd Ee Ff"}), m_nameTool.fromArticleCapitalizedNames(s));
  }

  void test_someFactoryDetail() {
    const QString& s = "ROND D'ALEMBERT, AA B'CDE. ROND D'ALEMBERT was a French mathematician, philosopher, and writer.";
    QCOMPARE((QStringList{"Rond D'Alembert", "Aa B'Cde"}), m_nameTool.fromArticleCapitalizedNames(s));
  }

  void test_CapitalWeak() {
    QCOMPARE(NameTool::CapitaliseFirstLetterKeepOther("henry cavill"), "Henry Cavill");
    QCOMPARE(NameTool::CapitaliseFirstLetterKeepOther("HENRY CAVILL"), "HENRY CAVILL");
  }

  void test_CapitalStrong() {
    QCOMPARE(NameTool::CapitaliseFirstLetterLowerOther("henry cavill"), "Henry Cavill");
    QCOMPARE(NameTool::CapitaliseFirstLetterLowerOther("HENRY CAVILL"), "Henry Cavill");
  }

  void test_CapitalStrongWithApostrophe() {  //
    QCOMPARE(NameTool::CapitaliseFirstLetterLowerOther("ROND D'ALEMBERT"), "Rond D'Alembert");
  }
  void test_ToggleCase() {  //
    QCOMPARE(NameTool::ToggleSentenceCase("Henry Cavill"), "hENRY cAVILL");
  }

 private:
  NameTool m_nameTool;
};

NameToolTest g_NameToolTest;
#include "NameToolTest.moc"
