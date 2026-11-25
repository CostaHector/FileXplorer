#ifndef CASTDBMODEL_H
#define CASTDBMODEL_H

#include "SqlTableModelPub.h"
#include <QFileInfo>
#include <QDir>
class QItemSelection;

class CastDbModel : public SqlTableModelPub {
public:
  explicit CastDbModel(const QString& imageHostFullPath, QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase());
  static bool isDbValidAndOpened(const QSqlDatabase& db);

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

  QDir rootDirectory() const { return QDir{rootPath()}; }

  QString rootPath() const { return m_imageHostPath; }

  QString fileName(const QModelIndex& curIndex) const;

  QString filePath(const QModelIndex& curIndex) const;

  QFileInfo fileInfo(const QModelIndex& curIndex) const;

  QString oriPath(const QModelIndex& curIndex) const;

  QString psonFilePath(const QModelIndex& curIndex) const;

  QStringList foldersRelativePath2RootPath(const QModelIndexList& selectedRows) const;

  QString oriAtIndex(const QModelIndex& curIndex) const;
  QString tagsAtIndex(const QModelIndex& curIndex) const;
  int rateAtIndex(const QModelIndex& curIndex) const;
  int sizeAtIndex(const QModelIndex& curIndex) const;
  int heightAtIndex(const QModelIndex& curIndex) const;
  QString coverImageFullPathAtIndex(const QModelIndex& curIndex, bool bUseThumbnail) const;

  QModelIndexList GetAllRowsIndexes() const;

  int RefreshImgsFieldValue(const QModelIndexList& selectedRows);
  int RefreshVidsFieldValue(const QModelIndexList& indices, QSqlDatabase videoDb);

  int DeleteSelectionRange(const QItemSelection& selectionRangeList);

  int DumpRecordsIntoPsonFile(const QModelIndexList& selectedRows);

  bool submitSaveAllChanges();
  bool revertAllChanges();

private:
  static int MigrateCastsOriFolder(const QString& imageHostPath, const QString& oldOri, const QString& newOri, const QString& actorName);
  static int MigrateCastNameFolder(const QString& imageHostPath, const QString& oriName, const QString& oldActorName, const QString& newActorName);

  const QString m_imageHostPath;
};

namespace CastDbModelErrorCode {
enum MigrateOriErrorCode {
  ACTOR_NAME_CONTAINS_INVALID_CHAR = -1000,
  ORI_EMPTY,
  ORI_CONTAINS_INVALID_CHAR,
  MKDIR_NEW_ORI_FAILED,
  MKPATH_NEW_ORI_ACTOR_NAME_FAILED,
  RENAME_OLD_ORI_ACTOR_TO_NEW_ORI_ACTOR_FAILED,
  BOTH_OLD_ORI_ACTOR_AND_NEW_ORI_ACTOR_FOLDERS_INEXIST,
};
enum MigrateCastNameErrorCode {
  ORI_NAME_CONTAINS_INVALID_CHAR = -1000,
  NEW_ACTOR_NAME_EMPTY,
  NEW_ACTOR_NAME_CONTAINS_INVALID_CHAR,
  MKDIR_ORI_NAME_FAILED,
  MKDIR_NEW_ACTOR_NAME_FAILED,
  RENAME_OLD_ACTOR_NAME_TO_NEW_ACTOR_NAME_FAILED,
  RENAME_FILES_UNDER_NEW_ACTOR_NAME_FAILED,
  BOTH_OLD_ACTOR_AND_NEW_ACTOR_FOLDERS_INEXIST,
};
}

#endif // CASTDBMODEL_H
