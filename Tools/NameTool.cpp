#include "NameTool.h"

const char* NameTool::FIELD_SEPERATOR = "&|\\band\\b|,|\t|\n|@|#|\\+|\\||/|\\\\";
const QRegularExpression NameTool::FS_COMP(FIELD_SEPERATOR, QRegularExpression::PatternOption::CaseInsensitiveOption);

const char* NameTool::FREQUENT_NAME_PATTER = "[A-Z]{2,}\\s[A-Z']+(\\s[A-Z]{2,})*";
const QRegularExpression NameTool::NAME_COMP(FREQUENT_NAME_PATTER);

const QString NameTool::INVALID_CHARS("*?\"<>|");
const QSet<QChar> NameTool::INVALID_FILE_NAME_CHAR_SET(INVALID_CHARS.cbegin(), INVALID_CHARS.cend());

QStringList NameTool::operator()(const QString& s) const {
  QStringList ans;
  foreach (QString nm, s.split(NameTool::FS_COMP)) {
    const QString& stdName = nm.trimmed();
    if (not stdName.isEmpty()) {
      ans.append(stdName);
    }
  }
  ans.removeDuplicates();
  return ans;
}
QStringList NameTool::fromArticleCapitalizedNames(const QString& s) const {
  QRegularExpressionMatchIterator it = NAME_COMP.globalMatch(s);
  QStringList ans;
  while (it.hasNext()) {
    QRegularExpressionMatch match = it.next();
    if (match.hasMatch()) {
      const QString& caurseName = match.captured(0);
      const QString& stdName = CapitaliseFirstLetterLowerOther(caurseName);
      ans.append(stdName);
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
  if (apostrophyIndex != -1 and apostrophyIndex + 1 < caurse.size()) {
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
