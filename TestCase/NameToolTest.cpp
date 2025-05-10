#include <QCoreApplication>
#include <QtTest>
#include "pub/MyTestSuite.h"

// add necessary includes here
#include "Tools/NameTool.h"
#include <QLineEdit>
#include <QTextEdit>
#include <QTextDocumentFragment>

class NameToolTest : public MyTestSuite {
  Q_OBJECT

 public:
  NameToolTest() : MyTestSuite{false} {}
 private slots:
  void test_AndAsSubStringOfNameRatherThanFS() {
    const QString& s = "Andrew Garfield and Andrew Tale";
    QCOMPARE(m_nameTool(s), (QStringList{"Andrew Garfield", "Andrew Tale"}));
  }

  void test_NameTool_operator() {
    const QStringList lst_A_B{"A", "B"};
    const QString& commaAsFS = "A , B";
    QCOMPARE(m_nameTool(commaAsFS), lst_A_B);
    const QString& verticalBarAsFS = "A | B";
    QCOMPARE(m_nameTool(verticalBarAsFS), lst_A_B);
    const QString& addAsFS = "A + B";
    QCOMPARE(m_nameTool(addAsFS), lst_A_B);
    const QString& ampersandAsFS = "A & B";
    QCOMPARE(m_nameTool(ampersandAsFS), lst_A_B);
    const QString& andAsFS = "A and B";
    QCOMPARE(m_nameTool(andAsFS), lst_A_B);
    const QString& AndIgnoreCase = "A And B";
    QCOMPARE(m_nameTool(AndIgnoreCase), lst_A_B);
    const QString& atAsFS = "@A @B";
    QCOMPARE(m_nameTool(atAsFS), lst_A_B);
    const QString& slashAsFS = "A/B";
    QCOMPARE(m_nameTool(slashAsFS), lst_A_B);
    const QString& backSlashAsFS = "A\\B";
    QCOMPARE(m_nameTool(backSlashAsFS), lst_A_B);
    const QString& tagAsFS = "A#B";
    QCOMPARE(m_nameTool(tagAsFS), lst_A_B);
    const QString& tabAsFS = "A\tB";
    QCOMPARE(m_nameTool(tabAsFS), lst_A_B);
    const QString& newlineAsFS = "A\nB";
    QCOMPARE(m_nameTool(newlineAsFS), lst_A_B);
    const QString& newlineAndCommaAsFS = "A\n,B";
    QCOMPARE(m_nameTool(newlineAndCommaAsFS), lst_A_B);
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
    QCOMPARE((QStringList{"Aa Bb", "Cc Dd"}), m_nameTool.castFromTitledSentence(s));
  }

  void test_3wordsCapitalizedName() {
    const QString& s = "AA BB CC, DD EE FF";
    QCOMPARE((QStringList{"Aa Bb Cc", "Dd Ee Ff"}), m_nameTool.castFromTitledSentence(s));
  }

  void test_invalid3wordCapitalizedName() {
    const QString& s = "A B'CC, DD E'F";  // should not process this one
    QVERIFY((QStringList{"A B'Cc", "Dd E'F"}) != m_nameTool.castFromTitledSentence(s));
  }

  void test_capitalizedNameWithApostrophe() {
    const QString& s = "AA B'CC, DD EE FF";
    const QStringList expects{"Aa B'Cc", "Dd Ee Ff"};
    QCOMPARE(expects, m_nameTool.castFromTitledSentence(s));
  }

  void test_someFactoryDetail() {
    const QString& s = "ROND D'ALEMBERT, AA B'CDE. ROND D'ALEMBERT was a French mathematician, philosopher, and writer.";
    const QStringList expectCastLst{"Rond D'Alembert", "Aa B'Cde"};
    QCOMPARE(expectCastLst, m_nameTool.castFromTitledSentence(s));
  }

  void test_Capital_ignore_other() {
    QCOMPARE(NameTool::CapitaliseFirstLetterKeepOther("henry cavill"), "Henry Cavill");
    QCOMPARE(NameTool::CapitaliseFirstLetterKeepOther("HENRY CAVILL"), "HENRY CAVILL");
    QCOMPARE(NameTool::CapitaliseFirstLetterKeepOther("NBA"), "NBA");
  }

  void test_Capital_lower_other() {
    QCOMPARE(NameTool::CapitaliseFirstLetterLowerOther("henry cavill"), "Henry Cavill");
    QCOMPARE(NameTool::CapitaliseFirstLetterLowerOther("HENRY CAVILL"), "Henry Cavill");
    QCOMPARE(NameTool::CapitaliseFirstLetterLowerOther("NBA"), "Nba");
    // Apostrophe
    QCOMPARE(NameTool::CapitaliseFirstLetterLowerOther("ROND D'ALEMBERT"), "Rond D'Alembert");
    QCOMPARE(NameTool::CapitaliseFirstLetterLowerOther("MICHAEL O'JUMPING"), "Michael O'Jumping");
  }

  void test_ToggleCase() {  //
    QCOMPARE(NameTool::ToggleSentenceCase("Henry Cavill"), "hENRY cAVILL");
    QCOMPARE(NameTool::ToggleSentenceCase("wI-fI"), "Wi-Fi");
  }

  void test_ReplaceAndUpdateSelection_QLineEdit() {  //
    QLineEdit le;
    QVERIFY(!NameTool::ReplaceAndUpdateSelection(le, nullptr));         // function nullptr
    QVERIFY(NameTool::ReplaceAndUpdateSelection(le, NameTool::Lower));  // no selected
    QVERIFY(!le.hasSelectedText());                                     // has no selection

    static const QString SRC_TEXT = "HELLO WORLD";
    le.setText(SRC_TEXT);
    le.setSelection(0, SRC_TEXT.size());  // all selected
    QVERIFY(NameTool::ReplaceAndUpdateSelection(le, NameTool::Lower));
    QVERIFY(le.hasSelectedText());
    QCOMPARE(le.selectedText(), "hello world");

    QVERIFY(NameTool::ReplaceAndUpdateSelection(le, NameTool::CapitaliseFirstLetterKeepOther));
    QVERIFY(le.hasSelectedText());
    QCOMPARE(le.selectedText(), "Hello World");
  }

  void test_ReplaceAndUpdateSelection_QTextEdit() {  //
    QTextEdit te;
    QVERIFY(!NameTool::ReplaceAndUpdateSelection(te, nullptr));         // function nullptr
    QVERIFY(NameTool::ReplaceAndUpdateSelection(te, NameTool::Lower));  // no selected
    QVERIFY(!te.textCursor().hasSelection());                           // has no selection

    static const QString SRC_TEXT = "HELLO WORLD";
    te.setText(SRC_TEXT);
    QTextCursor curSelection = te.textCursor();
    curSelection.setPosition(0);
    curSelection.setPosition(SRC_TEXT.size(), QTextCursor::KeepAnchor);
    te.setTextCursor(curSelection);
    QVERIFY(NameTool::ReplaceAndUpdateSelection(te, NameTool::Lower));

    QVERIFY(te.textCursor().hasSelection());
    QCOMPARE(te.textCursor().selectedText(), "hello world");

    QVERIFY(NameTool::ReplaceAndUpdateSelection(te, NameTool::CapitaliseFirstLetterKeepOther));
    QVERIFY(te.textCursor().hasSelection());
    QCOMPARE(te.textCursor().selectedText(), "Hello World");
  }

 private:
  NameTool m_nameTool;
};

NameToolTest g_NameToolTest;
#include "NameToolTest.moc"
