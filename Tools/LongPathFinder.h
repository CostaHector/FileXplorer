#ifndef LONGPATHFINDER_H
#define LONGPATHFINDER_H

#include <QString>
#include <QList>


class LongPathFinder
{
 public:
  LongPathFinder() = default;
  int operator()(const QString& path);
  // folderName/folderName.jpeg, i.e. 2 * N + 1(slash) + 5(dot and extension)
  static inline bool IsFolderLengthShort(const int& folderPathLen) {
    return 2 * folderPathLen + 1 + 5 < MAX_PATH_LENGTH;
  }
  // len(Prefix/folderName) = N
  // folderNameLen = N - backSlashInd - 1
  // N + 1(slash) + folderNameLen + 5(dot and ext) = 2N - backSlashInd + 5
  static bool IsLengthBelowSysLimit(const QString& path) {
    const int N = path.size();
    if (IsFolderLengthShort(N)) {
      return true;
    }
    const int backSlashInd = path.lastIndexOf('/');
    return 2 * N - backSlashInd + 5 < MAX_PATH_LENGTH;
  }

  void SetDropSectionWhenTooLong(int section);
  QString GetNewFolderName(const QString& oldName) const;
  int StillTooLongPathCount() const {
    return m_status.size() - pres.size();
  }
  int CheckTooLongPathCount();

  QStringList pres;
  QStringList olds;
  QStringList news;
  QString m_status;
  int dropSection{-1};
  static constexpr int MAX_PATH_LENGTH = 260 - 1; // path[260] and the last '\0'
};

#endif // LONGPATHFINDER_H
