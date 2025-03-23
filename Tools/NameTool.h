#ifndef NAMETOOL_H
#define NAMETOOL_H

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
class NameTool {
 public:
  QStringList operator()(const QString& s) const;
  QStringList fromArticleCapitalizedNames(const QString& s) const;

  static QString CapitaliseEachWordFirstLetterOnly(const QString& sentence);
  static QString CapitaliseEachWordFirstLetterLowercaseOthers(const QString& sentence);
  static QString ToggleSentenceCase(const QString& sentence);

  static const char* FIELD_SEPERATOR;
  static const QRegularExpression FS_COMP;

  static const char* FREQUENT_NAME_PATTER;
  static const QRegularExpression NAME_COMP;
};
#endif  // NAMETOOL_H
