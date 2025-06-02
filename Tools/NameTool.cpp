#include "NameTool.h"
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QLineEdit>
#include <QTextDocumentFragment>

const char* NameTool::FIELD_SEPERATOR{"&|\\band\\b|,|\t|\n|@|#|\\+|\\||/|\\\\"};
const QRegularExpression NameTool::FS_COMP(FIELD_SEPERATOR, QRegularExpression::PatternOption::CaseInsensitiveOption);

const char* NameTool::FREQUENT_NAME_PATTER = "[A-Z]{2,}\\s[A-Z']+(\\s[A-Z]{2,})*";
const QRegularExpression NameTool::NAME_COMP(FREQUENT_NAME_PATTER);

const QString NameTool::INVALID_CHARS("*?\"<>|");
const QSet<QChar> NameTool::INVALID_FILE_NAME_CHAR_SET(INVALID_CHARS.cbegin(), INVALID_CHARS.cend());

constexpr char NameTool::CSV_COMMA;
const QRegularExpression NameTool::CAST_STR_SPLITTER{R"( & |&|\s*,\s*|\r\n|\n| and | fucks | fuck )", QRegularExpression::PatternOption::CaseInsensitiveOption};

QStringList NameTool::operator()(const QString& s) const {
  QStringList ans;
  foreach (QString nm, s.split(NameTool::FS_COMP)) {
    const QString& stdName = nm.trimmed();
    if (!stdName.isEmpty()) {
      ans.append(stdName);
    }
  }
  // don't sort here
  ans.removeDuplicates();
  return ans;
}

QStringList NameTool::castFromSentence(const QString& s) const {
  QStringList cast = operator()(s);
  cast.sort();
  cast.removeDuplicates();
  return cast;
}

QStringList NameTool::castFromUpperCaseSentence(const QString& s) const {
  QRegularExpressionMatchIterator it = NAME_COMP.globalMatch(s);
  QStringList ans;
  QString srcCastName, stdCastName;
  QRegularExpressionMatch match;
  while (it.hasNext()) {
    match = it.next();
    if (match.hasMatch()) {
      srcCastName = match.captured(0);
      stdCastName = CapitaliseFirstLetterLowerOther(srcCastName);
      ans.append(stdCastName);
    }
  }
  ans.sort();
  ans.removeDuplicates();
  return ans;
}

QString NameTool::CapitaliseFirstLetterKeepOther(const QString& sentence) {
  QStringList lines = sentence.split('\n');
  QStringList processedLines;
  foreach (const QString& line, lines) {
      // dont use Qt::SkipEmptyParts; " world" => we want this {" ","world"} instead {"world"}
      QStringList words = line.split(' ');
      for (QString& word : words) {
          if (!word.isEmpty()) {
              word[0] = word[0].toUpper();
              // d'alembert => D'Alembert
              int apostrophePos = word.indexOf('\'');
              if (apostrophePos != -1 && apostrophePos + 1 < word.length()) {
                  word[apostrophePos + 1] = word[apostrophePos + 1].toUpper();
              }
          }
      }
      processedLines << words.join(' ');
  }
  return processedLines.join('\n');
}

QString NameTool::CapitaliseFirstLetterLowerOther(const QString& sentence) {
  const QString& sentenceInLowercase = sentence.toLower();
  return CapitaliseFirstLetterKeepOther(sentenceInLowercase);
}

QString NameTool::ToggleSentenceCase(const QString& sentence) {
  QString toggled;
  toggled.reserve(sentence.size());
  for (QChar c : sentence) {
    if (c.isLetter()) {
      toggled += (c.toLatin1() ^ 0x20);  // trick upper to lower case by bitwise operator
    } else {
      toggled += c;
    }
  }
  return toggled;
}

bool NameTool::ReplaceAndUpdateSelection(QPlainTextEdit& te, SentenceProcessorFunc fTrans) {
  if (fTrans == nullptr) {
    qWarning("fTrans is nullptr");
    return false;
  }
  if (!te.textCursor().hasSelection()) {
    qDebug("skip no text selected in QTextEdit");
    return true;
  }

  QTextCursor curSelection = te.textCursor();
  const int startPos = curSelection.selectionStart();
  QString before = curSelection.selectedText().replace(QChar(0x2029), '\n');
  curSelection.removeSelectedText();
  const QString& after = fTrans(before);
  curSelection.insertText(after);
  const int endPos = startPos + after.size();
  curSelection.setPosition(startPos);
  curSelection.setPosition(endPos, QTextCursor::KeepAnchor);
  te.setTextCursor(curSelection);
  return true;
}

bool NameTool::ReplaceAndUpdateSelection(QTextEdit& te, SentenceProcessorFunc fTrans) {
  if (fTrans == nullptr) {
    qWarning("fTrans is nullptr");
    return false;
  }
  if (!te.textCursor().hasSelection()) {
    qDebug("skip no text selected in QTextEdit");
    return true;
  }

  QTextCursor curSelection = te.textCursor();
  const int startPos = curSelection.selectionStart();
  QString before = curSelection.selectedText().replace(QChar(0x2029), '\n');
  curSelection.removeSelectedText();
  const QString& after = fTrans(before);
  curSelection.insertText(after);
  const int endPos = startPos + after.size();
  curSelection.setPosition(startPos);
  curSelection.setPosition(endPos, QTextCursor::KeepAnchor);
  te.setTextCursor(curSelection);
  return true;
}

bool NameTool::ReplaceAndUpdateSelection(QLineEdit& le, SentenceProcessorFunc fTrans) {
  if (fTrans == nullptr) {
    qWarning("fTrans is nullptr");
    return false;
  }
  if (!le.hasSelectedText()) {
    qDebug("skip no text selected in QLineEdit");
    return true;
  }

  const int startPos = le.selectionStart();
  const int endPos = le.selectionEnd();
  const QString& before = le.selectedText();
  const QString& after = fTrans(before);
  const QString& beforeFullText = le.text();
  const QString& afterFullText = beforeFullText.left(startPos) + after + beforeFullText.mid(endPos);
  le.setText(afterFullText);
  const int newSelectLetterCnt = after.size();
  le.setSelection(startPos, newSelectLetterCnt);
  return true;
}

QStringList NameTool::CastTagSentenceParse2Lst(const QString& sentense, bool bElementUnique) {
  if (sentense.isEmpty()) {
    return {};
  }
  QStringList lst = sentense.split(CAST_STR_SPLITTER);
  lst.sort();
  if (bElementUnique) {
    lst.removeDuplicates();
  }
  return lst;
}

QString NameTool::CastTagSentenceParse2Str(const QString& sentense, bool bElementUnique) {
  return CastTagSentenceParse2Lst(sentense, bElementUnique).join(CSV_COMMA);
}

QStringList NameTool::CastTagSentenceRmvEle2Lst(const QString& sentense, const QString& cast) {
  if (sentense.isEmpty()) {
    return {};
  }
  QStringList lst = sentense.split(CAST_STR_SPLITTER);
  lst.removeAll(cast);
  lst.sort();
  return lst;
}

QString NameTool::CastTagSentenceRmvEle2Str(const QString& sentense, const QString& cast) {
  return CastTagSentenceRmvEle2Lst(sentense, cast).join(CSV_COMMA);
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>
#include <QCheckBox>
#include <QVBoxLayout>
int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  QWidget mw;
  QVBoxLayout lo{&mw};
  QPlainTextEdit te;
  te.setPlainText("Hello world");
  QCheckBox btn{"title/lowercase", &mw};
  btn.setTristate(false);
  lo.addWidget(&btn);
  lo.addWidget(&te);
  mw.show();
  QObject::connect(&btn, &QCheckBox::stateChanged, [&](int state){
    if (state == Qt::CheckState::Checked) {
      NameTool::ReplaceAndUpdateSelection(te, NameTool::CapitaliseFirstLetterKeepOther);
    } else {
      NameTool::ReplaceAndUpdateSelection(te, NameTool::Lower);
    }
  });
  a.exec();
  return 0;
}
#endif


