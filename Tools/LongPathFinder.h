#ifndef LONGPATHFINDER_H
#define LONGPATHFINDER_H

#include <QStringList>

class LongPathFinder {
 public:
  LongPathFinder() = default;
  static bool IsTooLong(const QString& pth);
  static bool IsTooLong(int pathLength);

  int operator()(const QString& path);

  QStringList GetNamesAfterSectionDropped(const QStringList& srcLst) const;

  void SetDropSectionWhenTooLong(int section);
  QString GetNewFolderName(const QString& oldName) const;
  int CheckTooLongPathCount() const;

  mutable QString m_status;
  // -1:drop last section, 0: no drop; 1: drop first section
  int mDropSection{-1};
  // 260 = "C:\" + 256 + '\0' = 3+256+1
  // driver letter=3; effective path=256; end=1
  static constexpr int MAX_PATH_LENGTH = 260;
};

#endif  // LONGPATHFINDER_H
