#ifndef FDBASEDDB_H
#define FDBASEDDB_H

#include "DbManager.h"

class FdBasedDb : public DbManager {
 public:
  using DbManager::DbManager;
  int ReadADirectory(const QString& tableName, const QString& folderAbsPath);
  FD_ERROR_CODE Adt(const QString& tableName, const QString& peerPath, VolumeUpdateResult* pAdt = nullptr);
  int SetDuration(const QString& tableName);
  int ExportDurationStudioCastTagsToJson(const QString& tableName) const;
  int UpdateStudioCastTagsByJson(const QString& tableName, const QString& peerPath) const;

  static const QString CREATE_TABLE_TEMPLATE;
  static const QString INSERT_MOVIE_RECORD_FULL_TEMPLATE;
  static const QString INSERT_MOVIE_RECORD_TEMPLATE;
  static const QString UPDATE_PATH_TEMPLATE;
  static const QString SELECT_DURATION_0_TEMPLATE;
  static const QString UPDATE_DURATION_0_TEMPLATE;
  static const QString SELECT_DURATION_STUDIO_CAST_TAGS_TEMPLATE;
  static const QString UPDATE_STUDIO_CAST_TAGS_TEMPLATE;

  enum class QUERY_KEY_INFO_FIELED {
    PrePathLeft = 0,
    PrePathRight,
    Name,
    Size,
  };
  static const QString QUERY_KEY_INFO_TEMPLATE;
  static const QString WHERE_NAME_CORE_TEMPLATE;

 private:
  FD_ERROR_CODE Insert(const QString& tableName, const QSet<QByteArray>&needInsertFds, const QHash<QByteArray, QString>& newFd2Pth, int& insertCnt);
  FD_ERROR_CODE Delete(const QString& tableName, const QSet<QByteArray>&needDeleteFds, int& deleteCnt);
  FD_ERROR_CODE Update(const QString& tableName, const QSet<QByteArray>& needUpdateFds, const QHash<QByteArray, QString>& newFd2Pth, int& updateCnt);
};
#endif  // FDBASEDDB_H
