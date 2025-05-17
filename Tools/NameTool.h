#ifndef NAMETOOL_H
#define NAMETOOL_H

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QSet>
class QTextEdit;
class QLineEdit;

class NameTool {
 public:
  QStringList operator()(const QString& s) const;
  QStringList castFromUpperCaseSentence(const QString& s) const;

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

  typedef std::function<QString(const QString&)> SentenceProcessorFunc;
  static bool ReplaceAndUpdateSelection(QTextEdit& te, SentenceProcessorFunc fTrans);
  static bool ReplaceAndUpdateSelection(QLineEdit& le, SentenceProcessorFunc fTrans);

  static QStringList CastTagSentenceParse2Lst(const QString& sentense, bool bElementUnique);
  static QString CastTagSentenceParse2Str(const QString& sentense, bool bElementUnique);
  static QStringList CastTagSentenceRmvEle2Lst(const QString& sentense, const QString& cast);
  static QString CastTagSentenceRmvEle2Str(const QString& sentense, const QString& cast);

  static constexpr char CSV_COMMA = ',';
  static const QRegularExpression CAST_STR_SPLITTER;
 private:
  static const QString INVALID_CHARS;
  static const QSet<QChar> INVALID_FILE_NAME_CHAR_SET;
};
#endif  // NAMETOOL_H
