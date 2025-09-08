#ifndef CONVERTUNICODECHARSETTOASCII_H
#define CONVERTUNICODECHARSETTOASCII_H

#include <QHash>
#include <QString>
class ConvertUnicodeCharsetToAscii {
 public:
  QString operator()(const QString& boldStr) const;

  QStringList BatchCovert2Text(const QStringList& bolds) {
    QStringList ans;
    ans.reserve(bolds.size());
    for (const QString& bolStr : bolds) {
      ans.append(operator()(bolStr));
    }
    return ans;
  }

 private:
  static constexpr QChar UNICODE_FIRST_TWO_BYTE{0xD835};
  static QHash<QString, char> getBoldToTextDict();
  static const QHash<QString, char> BOLD_TO_TEXT_DICT;
};

#endif  // CONVERTUNICODECHARSETTOASCII_H
