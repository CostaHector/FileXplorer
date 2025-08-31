#include "LongPathFinder.h"

int LongPathFinder::MAX_PATH_LENGTH {260};
void LongPathFinder::SetMaxPathLength(const QString& maxPathLengthStr) {
  bool isnumeric = false;
  int maxPathLength = maxPathLengthStr.toInt(&isnumeric);
  if (!isnumeric) {
    qWarning("Insert index[%s] must be a number, unchange", qPrintable(maxPathLengthStr));
  }
  MAX_PATH_LENGTH = maxPathLength;
}

bool LongPathFinder::IsTooLong(const QString& pth) {
  return pth.size() >= MAX_PATH_LENGTH;
}

bool LongPathFinder::IsTooLong(int pathLength) {
  return pathLength >= MAX_PATH_LENGTH;
}

QStringList LongPathFinder::GetNamesAfterSectionDropped(const QStringList& srcLst) const {
  QStringList dst;
  dst.reserve(srcLst.size());
  for (const QString& oldFolderName : srcLst) {
    dst.append(GetNewFolderName(oldFolderName));
  }
  return dst;
}

void LongPathFinder::SetDropSectionWhenTooLong(int section) {
  mDropSection = section;
}

QString LongPathFinder::GetNewFolderName(const QString& oldName) const {
  if (mDropSection == 0) {  // 0: no drop
    return oldName;
  }
  auto oldNameSectionsList = oldName.split(" - ");
  if (oldNameSectionsList.size() <= 1 || oldNameSectionsList.size() < std::abs(mDropSection)) {
    return oldName;
  }
  if (mDropSection > 0) {  // 1 : drop first section, 2 drop second section
    oldNameSectionsList.erase(oldNameSectionsList.begin() + mDropSection - 1);
  } else {  // -1: drop last section; -2: drop second to last second
    oldNameSectionsList.erase(oldNameSectionsList.end() + mDropSection);
  }
  return oldNameSectionsList.join(" - ");
}

