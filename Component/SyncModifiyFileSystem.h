#ifndef SYNCMODIFIYFILESYSTEM_H
#define SYNCMODIFIYFILESYSTEM_H

#include <QStringList>

// when rename items under basicPath, synchronized to the specified path

class SyncModifiyFileSystem {
 public:
  SyncModifiyFileSystem();

  bool operator()(QString& path) const;

  static void LoadFromMemory();

  static bool SetBasicPath(const QString& basicPath);
  static bool SetSynchronizedToPaths(const QString& synchronizedToPath);

  static bool m_syncModifyFileSystemSwitch;
  static bool m_alsoSyncReversebackSwitch;

  static QString m_basicPath;
  static QString m_synchronizedToPath;
};

#endif  // SYNCMODIFIYFILESYSTEM_H
