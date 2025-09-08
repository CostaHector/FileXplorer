#include "ConvertUnicodeCharsetToAscii.h"
#include "Logger.h"

constexpr QChar ConvertUnicodeCharsetToAscii::UNICODE_FIRST_TWO_BYTE;
const QHash<QString, char> ConvertUnicodeCharsetToAscii::BOLD_TO_TEXT_DICT = getBoldToTextDict();

QString ConvertUnicodeCharsetToAscii::operator()(const QString& boldStr) const {
  QString ansStr;
  ansStr.reserve(boldStr.size());

  int i = 0;
  while (i < boldStr.size()) {
    if (QChar(0xD835) == boldStr[i]) {
      if (i + 1 >= boldStr.size()) {
        LOG_W("Incomplete char find in byte[%d]", i);
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

QHash<QString, char> ConvertUnicodeCharsetToAscii::getBoldToTextDict() {
  QHash<QString, char> dict;
  dict.reserve((26 << 4) + 10); // 26*(uppercase + lowercase)*(bold + normal) + extra preserve
  // not bold: 0XDC00
  for (int i = 0; i < 26; ++i){
    QString left = UNICODE_FIRST_TWO_BYTE;
    left.append(QChar(0XDC00 + i));
    dict.insert(left, 'A' + i);
  }
  for (int i = 0; i < 26; ++i){
    QString left = UNICODE_FIRST_TWO_BYTE;
    left.append(QChar(0XDC00 + 26 + i));
    dict.insert(left, 'a' + i);
  }
  // bold: 0XDDD4
  for (int i = 0; i < 26; ++i){
    QString left = UNICODE_FIRST_TWO_BYTE;
    left.append(QChar(0XDDD4 + i));
    dict.insert(left, 'A' + i);
  }
  for (int i = 0; i < 26; ++i){
    QString left = UNICODE_FIRST_TWO_BYTE;
    left.append(QChar(0XDDD4 + 26 + i));
    dict.insert(left, 'a' + i);
  }
  // Italic
  for (int i = 0; i < 26; ++i){
    QString left = UNICODE_FIRST_TWO_BYTE;
    left.append(QChar(0XDC68 + i));
    dict.insert(left, 'A' + i);
  }
  for (int i = 0; i < 26; ++i){
    QString left = UNICODE_FIRST_TWO_BYTE;
    left.append(QChar(0XDC68 + 26 + i));
    dict.insert(left, 'a' + i);
  }
  return dict;
}

