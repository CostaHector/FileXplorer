#ifndef SYNCMODIFIYFILESYSTEM_H
#define SYNCMODIFIYFILESYSTEM_H

#include <QString>

// when rename items under basicPath, synchronized to the specified path

class SyncModifiyFileSystem {
 public:
  bool operator()(QString& path) const;

  static void LoadFromMemory();
  static void SetSyncOperationSwitch(const bool bOn);
  static void SetSyncReverseBackSwitch(const bool bReverse);
  static bool SetBasicPath(const QString& basicPath);
  static bool SetSynchronizedToPaths(const QString& syncToPath);

  static bool m_syncOperationSw;
  static bool m_syncBackSw;
  static QString m_basicPath;
  static QString m_syncToPath;
};

#endif  // SYNCMODIFIYFILESYSTEM_H
