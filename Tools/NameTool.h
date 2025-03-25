#ifndef NAMETOOL_H
#define NAMETOOL_H

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QSet>

class NameTool {
 public:
  QStringList operator()(const QString& s) const;
  QStringList fromArticleCapitalizedNames(const QString& s) const;

  static QString CapitaliseFirstLetterKeepOther(const QString& sentence);
  static QString CapitaliseFirstLetterLowerOther(const QString& sentence);
  static QString ToggleSentenceCase(const QString& sentence);
  static inline QString Upper(const QString& s) { return s.toUpper(); }
  static inline QString Lower(const QString& s) { return s.toLower(); }

  static const char* FIELD_SEPERATOR;
  static const QRegularExpression FS_COMP;

  static const char* FREQUENT_NAME_PATTER;
  static const QRegularExpression NAME_COMP;

  static inline bool IsFileNameInvalid(const QString& filename) {
    QSet<QChar> nameSet{filename.cbegin(), filename.cend()};
    return nameSet.intersects(INVALID_FILE_NAME_CHAR_SET);
  }

 private:
  static const QString INVALID_CHARS;
  static const QSet<QChar> INVALID_FILE_NAME_CHAR_SET;
};
#endif  // NAMETOOL_H
