#include "ConvertUnicodeCharsetToAscii.h"

constexpr QChar ConvertUnicodeCharsetToAscii::UNICODE_FIRST_TWO_BYTE;
const QHash<QString, char> ConvertUnicodeCharsetToAscii::BOLD_TO_TEXT_DICT = getBoldToTextDict();

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
  return dict;
}

