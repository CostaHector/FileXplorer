#include "ValueChecker.h"
#include <QFileInfo>

namespace ValueChecker {

template<int MIN, int MAX>
bool GeneralIntRangeChecker(const QVariant& v) {
  if (!v.isValid()) { return false; }
  bool ok = false;
  int val = v.toInt(&ok);
  if (!ok) {
    return false;
  }
  return MIN <= val && val <= MAX;
}

template bool GeneralIntRangeChecker<INT_MIN, INT_MAX>(const QVariant&);
template bool GeneralIntRangeChecker<0, 10>(const QVariant&);
template bool GeneralIntRangeChecker<0, 16>(const QVariant&);
template bool GeneralIntRangeChecker<0, 100>(const QVariant&);
template bool GeneralIntRangeChecker<0, 255>(const QVariant&);
template bool GeneralIntRangeChecker<0, 500>(const QVariant&);
template bool GeneralIntRangeChecker<0, INT_MAX>(const QVariant&);

template<bool isPathOptionalAllowed>
bool GeneralFilePathStrChecker(const QString& filePath) {
  if (filePath.isEmpty()) {
    return isPathOptionalAllowed;
  }
  return QFileInfo(filePath).isFile();
}
template bool GeneralFilePathStrChecker<false>(const QString& filePath);
template bool GeneralFilePathStrChecker<true>(const QString& filePath);

bool GeneralFolderPathStrChecker(const QString& folderPath) {
  if (folderPath.isEmpty()) {
    return false;
  }
  return QFileInfo(folderPath).isDir();
}

bool GeneralFilePathChecker(const QVariant& v) {
  if (!v.isValid()) {
    return false;
  }
  return GeneralFilePathStrChecker<false>(v.toString());
}

bool GeneralFilePathOptionalChecker(const QVariant& v) {
  if (!v.isValid()) {
    return false;
  }
  return GeneralFilePathStrChecker<true>(v.toString());
}

bool GeneralFolderPathChecker(const QVariant& v) {
  if (!v.isValid()) {
    return false;
  }
  return GeneralFolderPathStrChecker(v.toString());
}

bool GeneralSequenceChecker(const QVariant& v) {
  if (!v.isValid()) {
    return false;
  }
  const QString seqStr = v.toString();
  const int len = seqStr.length();
  if (len == 0 || len > 10) {
    return false;
  }
  bool seen[10] = {false};
  for (QChar ch : seqStr) {
    if (!ch.isDigit()) {
      return false;
    }
    const int num = ch.digitValue();
    if (seen[num]) {
      return false;
    }
    seen[num] = true;
  }
  return true;
}

}
