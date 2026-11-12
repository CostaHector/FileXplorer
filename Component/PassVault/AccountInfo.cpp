#include "AccountInfo.h"
#include <QRegularExpression>

QString AccountInfo::toCsvLine() const {
  // in others fields there are "comma, new line"
  QString escapedOthers = othersStr;
  escapedOthers.replace("\\", "\\\\").replace("\n", "\\n").replace("\r", "\\r").replace(",", "\\,");
  return typeStr + ',' + nameStr + ',' + accountStr + ',' + pwdStr + ',' + escapedOthers;
}

bool AccountInfo::FromCsvLine(const QString& csvLine, AccountInfo& acc) {
  static const QRegularExpression COMMA_SPLITTER{"(?<!\\\\),"};
  QStringList parts = csvLine.split(COMMA_SPLITTER);
  if (parts.size() != 5) {
    return false;
  }
  const QString& othersStr = parts[4].replace("\\,", ",").replace("\\r", "\r").replace("\\n", "\n").replace("\\\\", "\\");
  acc = AccountInfo{parts[0], parts[1], parts[2], parts[3], othersStr};
  return true;
}

constexpr int AccountInfo::COLUMN_COUNT;
constexpr const char* AccountInfo::HOR_HEADER_TITLES[];
constexpr int AccountInfo::SORT_COLUMN;
constexpr AccountInfo::Role AccountInfo::DEF_SORT_ROLE;
constexpr int AccountInfo::DATA_TYPE_ROLE;