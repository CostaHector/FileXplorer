#include "NameTool.h"

const char* NameTool::FIELD_SEPERATOR{"&|\\band\\b|,|\t|\n|@|#|\\+|\\||/|\\\\"};
const QRegularExpression NameTool::FS_COMP(FIELD_SEPERATOR, QRegularExpression::PatternOption::CaseInsensitiveOption);

const char* NameTool::FREQUENT_NAME_PATTER = "[A-Z]{2,}\\s[A-Z']+(\\s[A-Z]{2,})*";
const QRegularExpression NameTool::NAME_COMP(FREQUENT_NAME_PATTER);

const QString NameTool::INVALID_CHARS("*?\"<>|");
const QSet<QChar> NameTool::INVALID_FILE_NAME_CHAR_SET(INVALID_CHARS.cbegin(), INVALID_CHARS.cend());

constexpr char NameTool::CSV_COMMA;

QStringList NameTool::operator()(const QString& s) const {
  QStringList ans;
  foreach (QString nm, s.split(NameTool::FS_COMP)) {
    const QString& stdName = nm.trimmed();
    if (!stdName.isEmpty()) {
      ans.append(stdName);
    }
  }
  ans.removeDuplicates();
  return ans;
}
QStringList NameTool::castFromTitledSentence(const QString& s) const {
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
  ans.removeDuplicates();
  return ans;
}

QString NameTool::CapitaliseFirstLetterKeepOther(const QString& sentence) {
  QStringList words = sentence.split(' ', Qt::SkipEmptyParts);
  for (QString& word : words) {
    word.front() = word.front().toUpper();
  }
  // ROND D'ALEMBERT => Rond D'Alembert
  QString caurse = words.join(' ');
  int apostrophyIndex = caurse.indexOf('\'');
  if (apostrophyIndex != -1 && apostrophyIndex + 1 < caurse.size()) {
    caurse[apostrophyIndex + 1] = caurse[apostrophyIndex + 1].toUpper();
  }
  return caurse;
}

QString NameTool::CapitaliseFirstLetterLowerOther(const QString& sentence) {
  const QString& sentenceInLowercase = sentence.toLower();
  return CapitaliseFirstLetterKeepOther(sentenceInLowercase);
}

QString NameTool::ToggleSentenceCase(const QString& sentence) {
  QString toggled;
  for (QChar c : sentence) {
    if (c.isLetter()) {
      toggled += (c.toLatin1() ^ 0x20);  // trick upper to lower case by bitwise operator
    } else {
      toggled += c;
    }
  }
  return toggled;
}
#include <QTextEdit>
#include <QLineEdit>
#include <QTextDocumentFragment>
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
  const QString& before = curSelection.selection().toPlainText();
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

const QRegularExpression CAST_STR_SPLITTER{R"( & |&|, |,|\r\n|\n| and | fucks | fuck )", QRegularExpression::PatternOption::CaseInsensitiveOption};
QStringList NameTool::CastTagStringProcess(const QString& sentense, bool bElementUnique) {
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

QString NameTool::CastTagString(const QString& sentense, bool bElementUnique) {
  return CastTagStringProcess(sentense, bElementUnique).join(CSV_COMMA);
}

QString NameTool::CastTagStringRmv(const QString& sentense, const QString& cast) {
  if (sentense.isEmpty()) {
    return {};
  }
  QStringList lst = sentense.split(CAST_STR_SPLITTER);
  lst.removeAll(cast);
  lst.sort();
  return lst.join(CSV_COMMA);
}
