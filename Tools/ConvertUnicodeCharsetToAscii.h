#ifndef CONVERTUNICODECHARSETTOASCII_H
#define CONVERTUNICODECHARSETTOASCII_H

#include <QHash>
#include <QString>
class ConvertUnicodeCharsetToAscii {
 public:
  QString operator()(const QString& boldStr) const {
    QString ansStr;
    ansStr.reserve(boldStr.size());

    int i = 0;
    while (i < boldStr.size()) {
      if (QChar(0xD835) == boldStr[i]) {
        if (i + 1 >= boldStr.size()) {
          qWarning("Incomplete char find in byte[%d]", i);
          return boldStr;
          break;
        }
        const QString& b = boldStr.mid(i, 2);
        auto it = BOLD_TO_TEXT_DICT.find(b);
        if (it == BOLD_TO_TEXT_DICT.cend()) {
          ansStr += b;
        } else {
          ansStr += it.value();
        }
        i += 2;
        continue;
      }
      ansStr += boldStr[i];
      ++i;
    }
    return ansStr;
  }

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
