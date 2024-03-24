#ifndef NAMETOOL_H
#define NAMETOOL_H

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
class NameTool {
 public:
  QStringList operator()(const QString& s) const {
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
  QStringList fromArticleCapitalizedNames(const QString& s) const {
    QRegularExpressionMatchIterator it = NAME_COMP.globalMatch(s);
    QStringList ans;
    while (it.hasNext()) {
      QRegularExpressionMatch match = it.next();
      if (match.hasMatch()) {
        const QString& caurseName = match.captured(0);
        const QString& stdName = CapitaliseEachWordFirstLetterLowercaseOthers(caurseName);
        ans.append(stdName);
      }
    }
    ans.removeDuplicates();
    return ans;
  }

  static QString CapitaliseEachWordFirstLetterOnly(const QString& sentence) {
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

  static QString CapitaliseEachWordFirstLetterLowercaseOthers(const QString& sentence) {
    const QString& sentenceInLowercase = sentence.toLower();
    return CapitaliseEachWordFirstLetterOnly(sentenceInLowercase);
  }

  static QString ToggleSentenceCase(const QString& sentence) {
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

  static const char* FIELD_SEPERATOR;
  static const QRegularExpression FS_COMP;

  static const char* FREQUENT_NAME_PATTER;
  static const QRegularExpression NAME_COMP;
};
#endif  // NAMETOOL_H
