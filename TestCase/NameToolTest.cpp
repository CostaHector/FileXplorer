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
    const QStringList& sentenseExamples{"Andrew Tale and Andrew Garfield",  //
                                        "Andrew Tale And Andrew Garfield",  //
                                        "Andrew Tale AND Andrew Garfield"};
    const QStringList expectList{"Andrew Tale", "Andrew Garfield"}; // not sorted
    foreach (const QString& sentense, sentenseExamples) {
      QCOMPARE(m_nameTool(sentense), expectList);
    }
  }

  void test_NameTool_operator() {
    const QStringList expectLst{"A", "B"};
    const QString addAsFS = "A + B";
    const QStringList commaAsFSLst{"A , B", "A, B", "A, B"};
    const QString verticalBarAsFS = "A | B";
    const QString ampersandAsFS = "A & B";
    const QStringList andAsFSLst{"A and B", "A AND B", "A And B"};
    const QString atAsFS = "@A @B";
    const QString slashAsFS = "A/B";
    const QString backSlashAsFS = "A\\B";
    const QString tagAsFS = "A#B";
    const QString tabAsFS = "A\tB";
    const QString newlineAsFS = "A\nB";
    const QString newlineAndCommaAsFS = "A\n,B";
    QCOMPARE(m_nameTool(addAsFS), expectLst);
    foreach (const QString& commaAsFS, commaAsFSLst) {
      QCOMPARE(m_nameTool(commaAsFS), expectLst);
    }
    QCOMPARE(m_nameTool(verticalBarAsFS), expectLst);
    QCOMPARE(m_nameTool(ampersandAsFS), expectLst);
    foreach (const QString& andAsFS, andAsFSLst) {
      QCOMPARE(m_nameTool(andAsFS), expectLst);
    }
    QCOMPARE(m_nameTool(atAsFS), expectLst);
    QCOMPARE(m_nameTool(slashAsFS), expectLst);
    QCOMPARE(m_nameTool(backSlashAsFS), expectLst);
    QCOMPARE(m_nameTool(tagAsFS), expectLst);
    QCOMPARE(m_nameTool(tabAsFS), expectLst);
    QCOMPARE(m_nameTool(newlineAsFS), expectLst);
    QCOMPARE(m_nameTool(newlineAndCommaAsFS), expectLst);
  }

  void test_moreThan2Perfs() {
    const QString& s = "A, B, and C";
    const QStringList perf3{"A", "B", "C"};
    QCOMPARE(m_nameTool(s), perf3);
  }

  void test_humanNaturePerfs() {
    const QString& s = "A.M. and O'Clock";
    const QStringList expectSingleQuote{"A.M.", "O'Clock"};
    QCOMPARE(m_nameTool(s), expectSingleQuote);
  }
  void test_unexpectedSpace() {
    const QString& s = "A,B ,C&D &E|F |G#H #I@J @K/L /M\\N \\O";
    const QStringList expectIgnoreSpace{"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O"};
    QCOMPARE(m_nameTool(s), expectIgnoreSpace);
  }
  void test_duplicateNames() {
    const QString& s = "A, B, B";
    const QStringList expectUniquePerf{"A", "B"};
    QCOMPARE(m_nameTool(s), expectUniquePerf);
  }
  void test_sequenceStable() {
    const QString& s = "B, A";
    const QStringList expectsStableSequence{"B", "A"};
    QCOMPARE(m_nameTool(s), expectsStableSequence);
  }

  void test_2wordsCapitalizedName() {
    const QString& s = "AA BB and CC DD";
    const QStringList expect{"Aa Bb", "Cc Dd"};
    const QStringList actual{m_nameTool.castFromUpperCaseSentence(s)};
    QCOMPARE(expect, actual);
  }

  void test_3wordsCapitalizedName() {
    const QString& s = "AA BB CC, DD EE FF";
    const QStringList expects{"Aa Bb Cc", "Dd Ee Ff"};
    QCOMPARE(expects, m_nameTool.castFromUpperCaseSentence(s));
  }

  void test_invalid3wordCapitalizedName() {
    const QString& s = "A B'CC, DD E'F";  // should not process this one
    const QStringList actual{m_nameTool.castFromUpperCaseSentence(s)};
    const QStringList expect{"A B'Cc", "Dd E'F"};
    QVERIFY(expect != actual);
  }

  void test_capitalizedNameWithApostrophe() {
    const QString& s = "AA B'CC, DD EE FF";
    const QStringList expects{"Aa B'Cc", "Dd Ee Ff"};
    QCOMPARE(expects, m_nameTool.castFromUpperCaseSentence(s));
  }

  void test_someFactoryDetail() {
    const QString& s = "ROND D'ALEMBERT, AA B'CDE. ROND D'ALEMBERT was a French mathematician, philosopher, and writer.";
    const QStringList expectCastLst{"Aa B'Cde", "Rond D'Alembert"};
    QCOMPARE(expectCastLst, m_nameTool.castFromUpperCaseSentence(s));
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

  void test_CastTagString() {
    QString sentense{"C, B, B, A"};
    const QStringList castSortedUnique{"A", "B", "C"};
    const QStringList castSortedDuplicate{"A", "B", "B", "C"};
    QCOMPARE(NameTool::CastTagSentenceParse2Lst(sentense, true), castSortedUnique);
    QCOMPARE(NameTool::CastTagSentenceParse2Lst(sentense, false), castSortedDuplicate);
    const QString castSortedUniqueStr{"A,B,C"};
    const QString castSortedDuplicateStr{"A,B,B,C"};
    QCOMPARE(NameTool::CastTagSentenceParse2Str(sentense, true), castSortedUniqueStr);
    QCOMPARE(NameTool::CastTagSentenceParse2Str(sentense, false), castSortedDuplicateStr);
  }

  void test_CastTagStringRmv() {
    QString sentense{"Chris Pines, Henry Cavill, Henry Cavill, Chris Evans"};
    const QStringList afterNothingRmved{"Chris Evans", "Chris Pines", "Henry Cavill", "Henry Cavill"};
    const QStringList afterRmved{"Chris Evans", "Chris Pines"};
    QCOMPARE(NameTool::CastTagSentenceRmvEle2Lst(sentense, "Alex"), afterNothingRmved);
    QCOMPARE(NameTool::CastTagSentenceRmvEle2Lst(sentense, "Henry Cavill"), afterRmved);
    const QString afterNothingRmvedStr{"Chris Evans,Chris Pines,Henry Cavill,Henry Cavill"};
    const QString afterRmvedStr{"Chris Evans,Chris Pines"};
    QCOMPARE(NameTool::CastTagSentenceRmvEle2Str(sentense, "Alex"), afterNothingRmvedStr);
    QCOMPARE(NameTool::CastTagSentenceRmvEle2Str(sentense, "Henry Cavill"), afterRmvedStr);
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
