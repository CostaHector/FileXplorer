#include "CastDbModel.h"
#include "CastBaseDb.h"
#include "PathTool.h"
#include "PublicVariable.h"
#include "CastPsonFileHelper.h"
#include "RateHelper.h"
#include "JsonHelper.h"
#include "JsonRenameRegex.h"
#include "QuickWhereClauseHelper.h"
#include "StringTool.h"
#include "NotificatorMacro.h"
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QItemSelection>

// 注意： CastDbModel自身不得主动调用 submitSaveAllChanges

CastDbModel::CastDbModel(const QString& imageHostFullPath, QObject* parent, QSqlDatabase db)
  : SqlTableModelPub{parent, db}
  , m_imageHostPath{imageHostFullPath}
{
  if (!QFileInfo{imageHostFullPath}.isDir()) {
    LOG_W("ImageHostPath[%s] is not a directory or not exist. cast view function abnormal", qPrintable(imageHostFullPath));
    return;
  }
  if (!isDbValidAndOpened(db)) {
    return;
  }
  // the only one cast table. set it right now
  if (db.tables().contains(DB_TABLE::PERFORMERS)) {
    setTable(DB_TABLE::PERFORMERS);
    select();
  } else {
    LOG_D("Need create a table at first");
  }
}

bool CastDbModel::isDbValidAndOpened(const QSqlDatabase& db) {
  if (!db.isValid()) {
    LOG_W("db is invalid: %s", qPrintable(db.lastError().text()));
    return false;
  }
  if (!db.isOpen()) {
    LOG_W("db is not open: %s", qPrintable(db.lastError().text()));
    return false;
  }
  return true;
}

QVariant CastDbModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return {};
  }
  if (role == Qt::DecorationRole && index.column() == CastDbModelField::Rate) {
    const int sc = QSqlTableModel::data(index, Qt::DisplayRole).toInt();
    return RateHelper::GetRatePixmap(sc);
  }

  return QSqlTableModel::data(index, role);
}

bool CastDbModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (!index.isValid()) {
    return false;
  }
  if (index.column() == CastDbModelField::Name) {
    const QString oldActorName{index.data(Qt::DisplayRole).toString()};
    const QString newActorName{value.toString()};
    const QString oriName{index.siblingAtColumn(CastDbModelField::Ori).data(Qt::DisplayRole).toString()};
    if (MigrateCastNameFolder(rootPath(), oriName, oldActorName, newActorName) < 0) {
      LOG_E("Rename Actor failed");
      return false;
    }
    // 交给基类写值
  } else if (index.column() == CastDbModelField::Ori) {
    const QString oldOri{index.data(Qt::DisplayRole).toString()};
    const QString newOri{value.toString()};
    const QString actorName{index.siblingAtColumn(CastDbModelField::Name).data(Qt::DisplayRole).toString()};
    if (MigrateCastsOriFolder(rootPath(), oldOri, newOri, actorName) < 0) {
      LOG_E("Migrate ori failed");
      return false;
    }
    // 交给基类写值
  }
  return QSqlTableModel::setData(index, value, role);
}

QString CastDbModel::fileName(const QModelIndex& curIndex) const {
  if (!curIndex.isValid()) {
    return "";
  }
  return curIndex.siblingAtColumn(CastDbModelField::Name).data(Qt::ItemDataRole::DisplayRole).toString();
}

QString CastDbModel::filePath(const QModelIndex& curIndex) const {
  if (!curIndex.isValid()) {
    return "";
  }
  QString folderFullPath;
  folderFullPath += m_imageHostPath;
  folderFullPath += '/';
  folderFullPath += curIndex.siblingAtColumn(CastDbModelField::Ori).data(Qt::ItemDataRole::DisplayRole).toString();
  folderFullPath += '/';
  folderFullPath += curIndex.siblingAtColumn(CastDbModelField::Name).data(Qt::ItemDataRole::DisplayRole).toString();
  return folderFullPath;
}

QFileInfo CastDbModel::fileInfo(const QModelIndex& curIndex) const {
  return QFileInfo{filePath(curIndex)};
}

QString CastDbModel::oriPath(const QModelIndex& curIndex) const {
  if (!curIndex.isValid()) {
    return "";
  }
  QString oriFullPath;
  oriFullPath += m_imageHostPath;
  oriFullPath += '/';
  oriFullPath += curIndex.siblingAtColumn(CastDbModelField::Ori).data(Qt::ItemDataRole::DisplayRole).toString();
  return oriFullPath;
}

QString CastDbModel::psonFilePath(const QModelIndex& curIndex) const {
  if (!curIndex.isValid()) {
    return "";
  }
  QString psonFullPath;
  psonFullPath += filePath(curIndex); // actor folder path
  psonFullPath += '/';
  psonFullPath += fileName(curIndex); // actor name
  psonFullPath += ".pson";            // suffix
  return psonFullPath;
}

QString CastDbModel::oriAtIndex(const QModelIndex& curIndex) const {
  if (!curIndex.isValid()) {
    return "";
  }
  return curIndex.siblingAtColumn(CastDbModelField::Ori).data(Qt::ItemDataRole::DisplayRole).toString();
}

QString CastDbModel::tagsAtIndex(const QModelIndex& curIndex) const {
  if (!curIndex.isValid()) {
    return "";
  }
  // comma seperated string
  return curIndex.siblingAtColumn(CastDbModelField::Tags).data(Qt::ItemDataRole::DisplayRole).toString();
}

int CastDbModel::rateAtIndex(const QModelIndex& curIndex) const {
  if (!curIndex.isValid()) {
    return -1;
  }
  return curIndex.siblingAtColumn(CastDbModelField::Rate).data(Qt::ItemDataRole::DisplayRole).toInt();
}

int CastDbModel::sizeAtIndex(const QModelIndex& curIndex) const {
  if (!curIndex.isValid()) {
    return -1;
  }
  return curIndex.siblingAtColumn(CastDbModelField::Size).data(Qt::ItemDataRole::DisplayRole).toInt();
}

int CastDbModel::heightAtIndex(const QModelIndex& curIndex) const {
  if (!curIndex.isValid()) {
    return -1;
  }
  return curIndex.siblingAtColumn(CastDbModelField::Height).data(Qt::ItemDataRole::DisplayRole).toInt();
}

QString CastDbModel::coverImageFullPathAtIndex(const QModelIndex& curIndex, bool bUseThumbnail) const {
  if (!curIndex.isValid()) {
    return "";
  }

  QString coverImageFullPathAtIndex{m_imageHostPath};
  coverImageFullPathAtIndex += '/';
  coverImageFullPathAtIndex += curIndex.siblingAtColumn(CastDbModelField::Ori).data(Qt::ItemDataRole::DisplayRole).toString();
  coverImageFullPathAtIndex += '/';
  const QString folderName = curIndex.siblingAtColumn(CastDbModelField::Name).data(Qt::ItemDataRole::DisplayRole).toString();
  coverImageFullPathAtIndex += folderName;
  if (bUseThumbnail) {
    return PathTool::GetThumbnailDecorationImgPathFromFolder(coverImageFullPathAtIndex, folderName);
  }
  const QString imgsSepratedInNewLine = curIndex.siblingAtColumn(CastDbModelField::Imgs).data(Qt::DisplayRole).toString();
  if (imgsSepratedInNewLine.isEmpty()) {
    return "";
  }
  const int firstNewLineIndex = imgsSepratedInNewLine.indexOf(StringTool::PERFS_VIDS_IMGS_SPLIT_CHAR);
  const QString imageFileName = firstNewLineIndex == -1 ? imgsSepratedInNewLine : imgsSepratedInNewLine.left(firstNewLineIndex);
  coverImageFullPathAtIndex += '/';
  coverImageFullPathAtIndex += imageFileName;
  return coverImageFullPathAtIndex;
}

QStringList CastDbModel::foldersRelativePath2RootPath(const QModelIndexList& selectedRows) const {
  QStringList oriSlashFolders;
  oriSlashFolders.reserve(selectedRows.size());
  for (const auto& ind: selectedRows) {
    QString ori2Name;
    ori2Name += ind.siblingAtColumn(CastDbModelField::Ori).data(Qt::ItemDataRole::DisplayRole).toString();
    ori2Name += '/';
    ori2Name += ind.siblingAtColumn(CastDbModelField::Name).data(Qt::ItemDataRole::DisplayRole).toString();
    oriSlashFolders.push_back(ori2Name);
  }
  return oriSlashFolders;
}

QModelIndexList CastDbModel::GetAllRowsIndexes() const {
  QItemSelection allSelection(index(0, CastDbModelField::Name), index(rowCount() - 1, CastDbModelField::Name));
  return allSelection.indexes();
}

int CastDbModel::RefreshImgsFieldValue(const QModelIndexList& selectedRowsIndexes) {
  if (selectedRowsIndexes.isEmpty()) {
    LOG_D("No need to Sync. nothing selected");
    return 0;
  }
  int succeedCnt = 0;
  for (const auto& indr : selectedRowsIndexes) {
    const QString oriFullPath = oriPath(indr);
    const QString actorName = fileName(indr);
    const QString newImagesFieldValue = CastBaseDb::GetImgsFieldNewValue(oriFullPath, actorName);
    if (indr.siblingAtColumn(CastDbModelField::Imgs).data(Qt::DisplayRole) == newImagesFieldValue) {
      continue;
    }
    if (!setData(indr.siblingAtColumn(CastDbModelField::Imgs), newImagesFieldValue, Qt::EditRole)) {
      LOG_W("Update image field[%s] failed", qPrintable(actorName));
      return FD_ERROR_CODE::FD_SET_RECORDS_FAILED;
    }
    ++succeedCnt;
  }
  LOG_D("%d/%d records was updated succeed", succeedCnt, selectedRowsIndexes.size());
  return succeedCnt;
}

int CastDbModel::RefreshVidsFieldValue(const QModelIndexList& selectedRowsIndexes, QSqlDatabase videoDb) {
  if (selectedRowsIndexes.isEmpty()) {
    LOG_D("No records selected for video refresh");
    return 0;
  }
  if (!isDbValidAndOpened(videoDb)) {
    return FD_ERROR_CODE::FD_DB_OPEN_FAILED;
  }

  QSqlQuery query(videoDb);
  static const auto ProcessQueryResults = [](QSqlQuery& query, int* vidCount = nullptr) -> QString {
    QStringList videoPaths;
    videoPaths.reserve(std::max(query.size(), 1));
    while (query.next()) {
      videoPaths << QuickWhereClauseHelper::GetMovieFullPathFromSqlQry(query);
    }
    if (vidCount != nullptr) {
      *vidCount = videoPaths.size();
    }
    std::sort(videoPaths.begin(), videoPaths.end());
    return videoPaths.join(StringTool::PERFS_VIDS_IMGS_SPLIT_CHAR);
  };

  int affectedRecordCnt = 0;
  int vidsCntFindOut = 0;
  for (const auto& index : selectedRowsIndexes) {
    query.finish();

    // 获取演员信息
    QString performer = index.siblingAtColumn(CastDbModelField::Name).data(Qt::DisplayRole).toString();
    QString akas = index.siblingAtColumn(CastDbModelField::ALIAS).data(Qt::DisplayRole).toString();

    // 构建并执行查询
    QString queryStr = QuickWhereClauseHelper::GetSelectMovieByCastStatement(performer, akas, DB_TABLE::MOVIES);
    if (!query.exec(queryStr)) {
      LOG_W("Query failed: %s", qPrintable(query.lastError().text()));
      continue;
    }

    // 处理查询结果
    int vidCount = 0;
    QString vidPaths = ProcessQueryResults(query, &vidCount);
    vidsCntFindOut += vidCount;

    if (index.siblingAtColumn(CastDbModelField::Vids).data(Qt::DisplayRole) == vidPaths) {
      continue; // unchange no need update
    }
    // 更新记录
    if (!setData(index.siblingAtColumn(CastDbModelField::Vids), vidPaths, Qt::EditRole)) {
      LOG_W("update the %dth row record[%s] failed %s", index.row(), qPrintable(performer), qPrintable(lastError().text()));
      return FD_ERROR_CODE::FD_SET_RECORDS_FAILED;
    }

    affectedRecordCnt++;
    LOG_D("Updated %d videos for %s", vidCount, qPrintable(performer));
  }

  LOG_D("`Vids` Fields of %d selected records get updated, %d videos find out", affectedRecordCnt, vidsCntFindOut);
  return affectedRecordCnt;
}

int CastDbModel::DeleteSelectionRange(const QItemSelection& selectionRangeList) {
  if (selectionRangeList.isEmpty()) {
    LOG_D("No need to delete. nothing selected");
    return 0;
  }
  int rowN = rowCount();
  // [beg, end) should in [0, rowN)
  const auto checkIfRangeValid = [rowN](int rowFront, int rowBack) -> bool { //
    return 0 <= rowFront && rowFront <= rowBack && rowBack < rowN;
  };

  int totalValidRowCnt = 0;
  int succeedCnt = 0;
  for (auto it = selectionRangeList.crbegin(); it != selectionRangeList.crend(); ++it) {
    const int startRow = it->top(); // [top, bottom]
    const int bottomRow = it->bottom();
    if (!checkIfRangeValid(startRow, bottomRow)) {
      LOG_W("row:[%d, %d) out of range [0, %d)", startRow, bottomRow + 1, rowN);
      continue;
    }
    const int curRowsCnt = it->height();
    totalValidRowCnt += curRowsCnt;
    bool bRemoveRet = removeRows(startRow, curRowsCnt);
    if (!bRemoveRet) {
      LOG_D("dropRet[%d] records range[%d, %d] failed", bRemoveRet, startRow, bottomRow);
      continue;
    }
    succeedCnt += curRowsCnt;
  }
  LOG_D("%d/%d records was delete succeed", succeedCnt, totalValidRowCnt);
  return succeedCnt;
}

int CastDbModel::DumpRecordsIntoPsonFile(const QModelIndexList& selectedRowsIndexes) {
  if (selectedRowsIndexes.isEmpty()) {
    LOG_D("No need to Dump. nothing selected");
    return 0;
  }
  QDir imageHostDir{m_imageHostPath};
  int totalCnt{selectedRowsIndexes.size()};
  int succeedCnt = 0;
  for (const auto& indr : selectedRowsIndexes) {
    const int r = indr.row();
    const auto& needDumpRec = record(r);
    const QString ori{needDumpRec.value(CastDbModelField::Ori).toString()};
    const QString castName{needDumpRec.value(CastDbModelField::Name).toString()};
    const QString prepath{ori + '/' + castName};
    if (!imageHostDir.exists(prepath) && !imageHostDir.mkpath(prepath)) {
      LOG_W("Create folder [%s] under [%s] failed", qPrintable(prepath), qPrintable(m_imageHostPath));
      return FD_ERROR_CODE::FD_CAST_PSON_PREPATH_MAKE_FAILED;
    }
    const QString psonPath{CastPsonFileHelper::PsonPath(m_imageHostPath, ori, castName)};
    const QVariantHash pson = CastPsonFileHelper::PerformerJsonJoiner(needDumpRec);
    if (QFile::exists(psonPath) && JsonHelper::MovieJsonLoader(psonPath) == pson) {
      LOG_D("pson[%s] unchange at skip it", qPrintable(psonPath));
      continue; // unchange
    }
    succeedCnt += JsonHelper::DumpJsonDict(pson, psonPath);
  }
  LOG_D("%d/%d records was delete succeed", succeedCnt, totalCnt);
  return succeedCnt;
}

bool CastDbModel::submitSaveAllChanges() {
  // 在 OnManualSubmit 模式下，submitAll() 成功后，模型会被重新填充（repopulated），任何展示该模型的视图都会丢失其选中项
  // 会重新执行查询（select())来刷新模型, modelBeginReset级别的, 所以索引即使时 QPersistentModelIndex 也会失效
  if (!isDirty()) {
    LOG_D("Table[%s] not dirty, skip submit", qPrintable(DB_TABLE::PERFORMERS));
    return true;
  }

  QSqlDatabase db = database();
  if (!isDbValidAndOpened(db)) {
    LOG_W("db invalid or not open");
    return false;
  }
  if (!db.transaction()) {
    LOG_W("Begin transaction failed: %s", qPrintable(db.lastError().text()));
    return false;
  }
  if (!submitAll()) { // view中要保护视图中的选中和当前选中
    LOG_W("SubmitAll failed[%s], rollback now", qPrintable(lastError().text()));
    if (!db.rollback()) {
      LOG_W("Rollback also failed: %s", qPrintable(db.lastError().text()));
    }
    return false;
  }
  if (!db.commit()) {
    LOG_W("Commit failed[%s]", qPrintable(db.lastError().text()));
    return false;
  }
  return true;
}

bool CastDbModel::revertAllChanges() {
  if (!isDirty()) {
    LOG_INFO_NP("Table not dirty.", "Skip revert");
    return false;
  }
  revertAll(); // 会自动调用select重新查询并丢弃未提交的插入
  LOG_OK_NP("Revert succeed", "All changes revert");
  return true;
}

// 修改Actor归属的Ori字段, 返回是否有修改文件名, 是:1, 否或者跳过:0, 失败<0
int CastDbModel::MigrateCastsOriFolder(const QString& imageHostPath, const QString& oldOri, const QString& newOri, const QString& actorName) {
  using namespace CastDbModelErrorCode;
  if (newOri == oldOri) {
    LOG_D("Skip Migrate, Old/New Ori Name equals[%s] ", qPrintable(newOri));
    return 0;
  }
  if (actorName.contains(JSON_RENAME_REGEX::INVALID_CHARS_IN_FILENAME)) {
    LOG_W("Abort Migrate, actorName[%s] invalid", qPrintable(newOri));
    return MigrateOriErrorCode::ACTOR_NAME_CONTAINS_INVALID_CHAR;
  }

  if (newOri.isEmpty()) {
    LOG_W("new Ori cannot be empty");
    return MigrateOriErrorCode::ORI_EMPTY;
  }
  if (newOri.contains(JSON_RENAME_REGEX::INVALID_CHARS_IN_FILENAME)) {
    LOG_W("Abort Migrate, Ori Folder Name[%s] invalid", qPrintable(newOri));
    return MigrateOriErrorCode::ORI_CONTAINS_INVALID_CHAR;
  }
  QDir imageHostDir{imageHostPath};
  if (!imageHostDir.exists(newOri)) { // 确保 newOri 前置存在
    if (!imageHostDir.mkdir(newOri)) {
      LOG_W("Abort cannot mkdir[newOri:%s]", qPrintable(newOri));
      return MigrateOriErrorCode::MKDIR_NEW_ORI_FAILED;
    }
  }
  if (actorName.isEmpty()) {
    // 新插入的行, actorName值为空时, 可以直接指定合法的 newOri， (副作用： 创建好 newOri)
    return 0;
  }

  QDir newActorDir{imageHostDir.absoluteFilePath(newOri)};
  if (!newActorDir.exists(actorName)) {
    if (oldOri.isEmpty() && !newActorDir.mkdir(actorName)) { // 首次填写oldOri值时， 允许创建actorName文件夹， 否则， 只允许移动
      LOG_W("Abort, cannot QDir(newOri:%s).mkdir(ActorName:%s)", qPrintable(newOri), qPrintable(actorName));
      return MigrateOriErrorCode::MKPATH_NEW_ORI_ACTOR_NAME_FAILED;
    }
  }
  if (oldOri.isEmpty()) {
    // 无需迁移: 新插入一行时, 初始内容为空, 创建完成newOri/actorName就行
    return 0;
  }

  // 需要迁移
  const QString oldOri2ActorName{oldOri + '/' + actorName};
  const QString newOri2ActorName{newOri + '/' + actorName};
  const bool bIsOldOri2ActorFolderExist{imageHostDir.exists(oldOri2ActorName)};
  const bool bIsNewOri2ActorFolderExist{imageHostDir.exists(newOri2ActorName)};
  if (bIsOldOri2ActorFolderExist) {
    bool migrateResult = imageHostDir.rename(oldOri2ActorName, newOri2ActorName);
    if (!migrateResult) { // 搬到目标 newOri 出错
      LOG_W("Migrate castName[%s] from oldOri[%s]->newOri[%s] failed", qPrintable(actorName), qPrintable(oldOri), qPrintable(newOri));
      return MigrateOriErrorCode::RENAME_OLD_ORI_ACTOR_TO_NEW_ORI_ACTOR_FAILED;
    }
    LOG_OK_P("Migrate ok", "castName[%s] from oldOri[%s]->newOri[%s] succeed", qPrintable(actorName), qPrintable(oldOri), qPrintable(newOri));
    return 1;
  } else {
    if (bIsNewOri2ActorFolderExist) {
      // 已经搬走到目标newOri了
      return 0;
    } else {
      // 旧的不存在, 并且新的目标也不存在
      LOG_W("Migrate abort. Both Ori/Actor [%s] and [%s] inexist", qPrintable(oldOri2ActorName), qPrintable(newOri2ActorName));
      return MigrateOriErrorCode::BOTH_OLD_ORI_ACTOR_AND_NEW_ORI_ACTOR_FOLDERS_INEXIST;
    }
  }
}

// 修改Name字段, 返回迁移文件数量
int CastDbModel::MigrateCastNameFolder(const QString& imageHostPath, const QString& oriName, const QString& oldActorName, const QString& newActorName) {
  using namespace CastDbModelErrorCode;
  if (oldActorName == newActorName) {
    LOG_D("Actor name unchange[%s]", qPrintable(oldActorName));
    return 0;
  }
  if (oriName.contains(JSON_RENAME_REGEX::INVALID_CHARS_IN_FILENAME)) {
    LOG_W("oriName[%s] invalid should not contain invalid chars", qPrintable(newActorName));
    return MigrateCastNameErrorCode::ORI_NAME_CONTAINS_INVALID_CHAR;
  }
  if (newActorName.isEmpty()) {
    LOG_W("newActorName cannot be empty");
    return MigrateCastNameErrorCode::NEW_ACTOR_NAME_EMPTY;
  }
  if (newActorName.contains(JSON_RENAME_REGEX::INVALID_CHARS_IN_FILENAME)) {
    LOG_W("New name[%s] invalid should not contain invalid chars", qPrintable(newActorName));
    return MigrateCastNameErrorCode::NEW_ACTOR_NAME_CONTAINS_INVALID_CHAR;
  }
  if (oriName.isEmpty()) {
    // 新插入的行, oriName为空时, 可以直接指定合法的 newActorName
    return 0;
  }

  QDir imageHostDir{imageHostPath};
  if (!imageHostDir.exists(oriName)) {
    if (!imageHostDir.mkdir(oriName)) {
      LOG_W("Abort cannot mkdir[oriName:%s]", qPrintable(oriName));
      return MigrateCastNameErrorCode::MKDIR_ORI_NAME_FAILED;
    }
  }

  QDir oriDir{imageHostDir.absoluteFilePath(oriName)};
  if (oldActorName.isEmpty()) {
    // 无需迁移: 新插入一行时, 初始内容为空, 无需移动, 直接创建一个oriName即可
    if (!oriDir.exists(newActorName)) {
      if (!oriDir.mkdir(newActorName)) {
        LOG_W("Abort, cannot mkdir[newActorName:%s]", qPrintable(newActorName));
        return MigrateCastNameErrorCode::MKDIR_NEW_ACTOR_NAME_FAILED;
      }
    }
    // 已经存在oriName/newName, 成功
    return 0;
  }

  // 需要迁移
  const bool bIsOri2oldActorNameExist{oriDir.exists(oldActorName)};
  const bool bIsOri2NewActorNameExist{oriDir.exists(newActorName)};

  if (bIsOri2oldActorNameExist) {
    // 先搬动最上层
    // 1. rename folder [under oriName]: oldActorName -> newActorName
    int renameCount = 0;
    bool migrateResult = oriDir.rename(oldActorName, newActorName);
    if (!migrateResult) {
      // 搬到目标 actorName folder 出错
      LOG_W("rename folder [under oriName:%s]: oldActorName[%s] -> newActorName[%s] failed", qPrintable(oriName), qPrintable(oldActorName), qPrintable(newActorName));
      return MigrateCastNameErrorCode::RENAME_OLD_ACTOR_NAME_TO_NEW_ACTOR_NAME_FAILED;
    }
    ++renameCount;

    // 2. rename files under [oriName/newActorName]: oldActorName****->newActorName****
    oriDir.setPath(oriDir.absoluteFilePath(newActorName));
    oriDir.setFilter(QDir::Filter::Files);
    oriDir.setSorting(QDir::SortFlag::Name);
    oriDir.setNameFilters({oldActorName + "*"});
    const int oldLength = oldActorName.size();
    for (const QString& oldFileName : oriDir.entryList()) {
      const QString newFileName = newActorName + oldFileName.mid(oldLength);
      if (newFileName == oldFileName) {
        continue;
      }
      if (!oriDir.rename(oldFileName, newFileName)) {
        LOG_W("Rename files under [oriName/newActorName:%s]: [%s]->[%s] failed", qPrintable(newActorName), qPrintable(oldFileName), qPrintable(newFileName));
        return MigrateCastNameErrorCode::RENAME_FILES_UNDER_NEW_ACTOR_NAME_FAILED;
      }
      ++renameCount;
    }
    LOG_OK_P("Migrate ok", "oldOri[%s] actorName[%s]->[%s]succeed", qPrintable(oriName), qPrintable(oldActorName), qPrintable(newActorName));
    return renameCount;
  } else {
    if (bIsOri2NewActorNameExist) {
      // 已经搬走到目标newOri了
      return 0;
    } else {
      // 旧的不存在, 并且新的目标也不存在
      LOG_W("Migrate abort. OriName:%s. Both oldActorName:%s and newActorName:%s inexist", qPrintable(oriName), qPrintable(oldActorName), qPrintable(newActorName));
      return MigrateCastNameErrorCode::BOTH_OLD_ACTOR_AND_NEW_ACTOR_FOLDERS_INEXIST;
    }
  }
}