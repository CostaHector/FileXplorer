#ifndef CONVERTBOLDUNICODECHARSETTOASCII_H
#define CONVERTBOLDUNICODECHARSETTOASCII_H

#include <QTextDecoder>

#include <QHash>
#include <QString>
class ConvertBoldUnicodeCharsetToAscii {
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
  static QHash<QString, char> getBoldToTextDict();
  static const QHash<QString, char> BOLD_TO_TEXT_DICT;
};

#endif  // CONVERTBOLDUNICODECHARSETTOASCII_H
