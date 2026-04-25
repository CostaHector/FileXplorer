#ifndef VALUECHECKER_H
#define VALUECHECKER_H
#include <QVariant>
#include <climits>

namespace ValueChecker {
using RawVariantChecker = bool (*)(const QVariant& v);

inline bool GeneralNoChecker(const QVariant& /*v*/) {
  return true;
}

template<int MIN = INT_MIN, int MAX = INT_MAX>
bool GeneralIntRangeChecker(const QVariant& v);
extern template bool GeneralIntRangeChecker<INT_MIN, INT_MAX>(const QVariant&);
extern template bool GeneralIntRangeChecker<0, 10>(const QVariant&);      // used in rating score
extern template bool GeneralIntRangeChecker<0, 16>(const QVariant&);      // used in extra small enum
extern template bool GeneralIntRangeChecker<0, 100>(const QVariant&);     // used in small enum, or volume
extern template bool GeneralIntRangeChecker<0, 255>(const QVariant&);     // used in opacity
extern template bool GeneralIntRangeChecker<0, 500>(const QVariant&);     // used in large enum
extern template bool GeneralIntRangeChecker<0, INT_MAX>(const QVariant&); // no negative number

inline bool GeneralIntChecker(const QVariant& v) {
  return GeneralIntRangeChecker<>(v);
}

inline bool GeneralCharChecker(const QVariant& v) {
  if (!v.isValid()) {
    return false;
  }
  return true;
}

inline bool GeneralBoolChecker(const QVariant& v) {
  if (!v.isValid()) {
    return false;
  }
  return v.canConvert<bool>();
}

inline bool GeneralDoubleChecker(const QVariant& v) {
  if (!v.isValid()) {
    return false;
  }
  bool bDouble{false};
  v.toDouble(&bDouble);
  return bDouble;
}

inline bool GeneralCStrChecker(const QVariant& v) {
  if (!v.isValid()) {
    return false;
  }
  return v.canConvert<QString>();
}

bool GeneralIntRangeChecker(const QString& v);

template<bool isPathOptionalAllowed = false>
bool GeneralFilePathStrChecker(const QString& filePath);
bool GeneralFolderPathStrChecker(const QString& folderPath);

bool GeneralFilePathChecker(const QVariant& v);
bool GeneralFilePathOptionalChecker(const QVariant& v);
bool GeneralFolderPathChecker(const QVariant& v);

bool GeneralSequenceChecker(const QVariant& v);

} // namespace ValueChecker

#endif // VALUECHECKER_H
