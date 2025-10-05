#include "AccountInfo.h"
#include <QStringList>
#include <QRegularExpression>
const QStringList AccountInfo::HORIZONTAL_HEAD{"Index", "Type", "Name", "Account"};

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
