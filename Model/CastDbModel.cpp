#include "CastDbModel.h"
#include "CastBaseDb.h"
#include "CastDBActions.h"
#include "MemoryKey.h"
#include "PublicVariable.h"
#include "PublicTool.h"
#include "TableFields.h"
#include "PublicMacro.h"
#include "JsonHelper.h"
#include "CastPsonFileHelper.h"
#include "QuickWhereClauseHelper.h"
#include "StringTool.h"
#include <QPainter>
#include <QPixmap>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>

constexpr int CastDbModel::MAX_RATE;

CastDbModel::CastDbModel(QObject* parent, QSqlDatabase db)
    : QSqlTableModel{parent, db},
      m_imageHostPath{Configuration()                                              //
                          .value(MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.name,  //
                                 MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.v)     //
                          .toString()} {
  setEditStrategy(QSqlTableModel::EditStrategy::OnManualSubmit);

  if (isDbValidAndOpened(db)) {
    // the only one cast table. set it right now
    if (db.tables().contains(DB_TABLE::PERFORMERS)) {
      setTable(DB_TABLE::PERFORMERS);
      select();
    } else {
      LOG_D("Init table at first");
    }
  }

  if (!QFileInfo{m_imageHostPath}.isDir()) {
    LOG_W("ImageHostPath[%s] is not a directory or not exist. cast view function abnormal", qPrintable(m_imageHostPath));
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
  static const QPixmap PERFORMER_SCORE_BOARD[MAX_RATE + 1]  //
      {
          FileTool::GetRatePixmap(0, CastDbModel::MAX_RATE),   //
          FileTool::GetRatePixmap(1, CastDbModel::MAX_RATE),   //
          FileTool::GetRatePixmap(2, CastDbModel::MAX_RATE),   //
          FileTool::GetRatePixmap(3, CastDbModel::MAX_RATE),   //
          FileTool::GetRatePixmap(4, CastDbModel::MAX_RATE),   //
          FileTool::GetRatePixmap(5, CastDbModel::MAX_RATE),   //
          FileTool::GetRatePixmap(6, CastDbModel::MAX_RATE),   //
          FileTool::GetRatePixmap(7, CastDbModel::MAX_RATE),   //
          FileTool::GetRatePixmap(8, CastDbModel::MAX_RATE),   //
          FileTool::GetRatePixmap(9, CastDbModel::MAX_RATE),   //
          FileTool::GetRatePixmap(10, CastDbModel::MAX_RATE),  //
      };                                                       //
  if (role == Qt::DecorationRole && index.column() == PERFORMER_DB_HEADER_KEY::Rate) {
    const int sc = QSqlTableModel::data(index, Qt::DisplayRole).toInt();
    return PERFORMER_SCORE_BOARD[(sc > MAX_RATE) ? MAX_RATE : (sc < 0 ? 0 : sc)];
  }

  return QSqlTableModel::data(index, role);
}

bool CastDbModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (!index.isValid()) {
    return false;
  }
  if (index.column() == PERFORMER_DB_HEADER_KEY::Name) {
    const QString oldName{data(index, Qt::DisplayRole).toString()};
    const QString newName{value.toString()};
    if (oldName == newName) {
      return false;
    }
    // rename folder and files
    const QString imgOriPath{oriPath(index)};
    if (CastBaseDb::WhenCastNameRenamed(imgOriPath, oldName, newName) < 0) {
      LOG_C("Rename failed, not write into db");
      return false;
    }
  }
  return QSqlTableModel::setData(index, value, role);
}

QString CastDbModel::fileName(const QModelIndex& curIndex) const {
  if (!curIndex.isValid()) {
    return "";
  }
  const QModelIndex& nameIndex = curIndex.siblingAtColumn(PERFORMER_DB_HEADER_KEY::Name);
  return data(nameIndex, Qt::ItemDataRole::DisplayRole).toString();
}

QString CastDbModel::filePath(const QModelIndex& curIndex) const {
  if (!curIndex.isValid()) {
    return "";
  }
  return oriPath(curIndex) + '/' + fileName(curIndex);
}

QFileInfo CastDbModel::fileInfo(const QModelIndex& curIndex) const {
  return QFileInfo{filePath(curIndex)};
}

QString CastDbModel::oriPath(const QModelIndex& curIndex) const {
  if (!curIndex.isValid()) {
    return "";
  }
  const QModelIndex& oriIndex = curIndex.siblingAtColumn(PERFORMER_DB_HEADER_KEY::Ori);
  return m_imageHostPath + '/' + data(oriIndex, Qt::ItemDataRole::DisplayRole).toString();
}

QString CastDbModel::psonFilePath(const QModelIndex& curIndex) const {
  if (!curIndex.isValid()) {
    return "";
  }
  return filePath(curIndex) + '/' + fileName(curIndex) + ".pson";
}

QModelIndexList CastDbModel::GetAllRowsIndexes() const {
  QItemSelection allSelection(index(0, PERFORMER_DB_HEADER_KEY::Name), index(rowCount() - 1, PERFORMER_DB_HEADER_KEY::Name));
  return allSelection.indexes();
}

int CastDbModel::SyncImageFieldsFromImageHost(const QModelIndexList& selectedRowsIndexes) {
  if (selectedRowsIndexes.isEmpty()) {
    LOG_D("No need to Sync. nothing selected");
    return 0;
  }
  int succeedCnt = 0;
  for (const auto& indr : selectedRowsIndexes) {
    const int r = indr.row();
    QSqlRecord imgUpdatedRec = record(r);
    if (!CastBaseDb::UpdateRecordImgsField(imgUpdatedRec, m_imageHostPath)) {  // skipped
      continue;
    }
    if (!setRecord(r, imgUpdatedRec)) {
      LOG_D("Update image field failed", qPrintable(imgUpdatedRec.value(PERFORMER_DB_HEADER_KEY::Name).toString()));
      return FD_ERROR_CODE::FD_SET_RECORDS_FAILED;
    }
    ++succeedCnt;
  }
  if (succeedCnt >= 0) {
    submitSaveAllChanges();
  }
  LOG_D("%d/%d records was updated succeed", succeedCnt, selectedRowsIndexes.size());
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
    const QString ori{needDumpRec.value(PERFORMER_DB_HEADER_KEY::Ori).toString()};
    const QString castName{needDumpRec.value(PERFORMER_DB_HEADER_KEY::Name).toString()};
    const QString prepath{ori + '/' + castName};
    if (!imageHostDir.exists(prepath) && !imageHostDir.mkpath(prepath)) {
      LOG_W("Create folder [%s] under [%s] failed", qPrintable(prepath), qPrintable(m_imageHostPath));
      return FD_ERROR_CODE::FD_CAST_PSON_PREPATH_MAKE_FAILED;
    }
    const QString psonPath{CastPsonFileHelper::PsonPath(m_imageHostPath, ori, castName)};
    const QVariantHash pson = CastPsonFileHelper::PerformerJsonJoiner(needDumpRec);
    if (QFile::exists(psonPath) && JsonHelper::MovieJsonLoader(psonPath) == pson) {
      LOG_D("pson[%s] unchange at skip it", qPrintable(psonPath));
      continue;  // unchange
    }
    succeedCnt += JsonHelper::DumpJsonDict(pson, psonPath);
  }
  LOG_D("%d/%d records was delete succeed", succeedCnt, totalCnt);
  return succeedCnt;
}

int CastDbModel::DeleteSelectionRange(const QItemSelection& selectionRangeList) {
  if (selectionRangeList.isEmpty()) {
    LOG_D("No need to delete. nothing selected");
    return 0;
  }
  int rowN = rowCount();
  // [beg, end) should in [0, rowN)
  const auto checkIfRangeValid = [rowN](int rowBegin, int rowEnd) -> bool {  //
    return (rowBegin <= rowEnd)                                              //
           && (0 <= rowBegin)                                                //
           && (rowEnd <= rowN);
  };

  int totalValidRowCnt = 0;
  int succeedCnt = 0;
  for (auto it = selectionRangeList.crbegin(); it != selectionRangeList.crend(); ++it) {
    const int startRow = it->top();  // [top, bottom]
    const int bottomRow = it->bottom();
    if (!checkIfRangeValid(startRow, bottomRow + 1)) {
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
  if (succeedCnt >= 0) {
    submitSaveAllChanges();
  }
  LOG_D("%d/%d records was delete succeed", succeedCnt, totalValidRowCnt);
  return succeedCnt;
}

int CastDbModel::RefreshVidsForRecords(const QModelIndexList& selectedRowsIndexes, QSqlDatabase videoDb) {
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
    int row = index.row();
    QSqlRecord rec = record(row);

    // 获取演员信息
    QString performer = rec.value(PERFORMER_DB_HEADER_KEY::Name).toString();
    QString akas = rec.value(PERFORMER_DB_HEADER_KEY::AKA).toString();

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

    // 更新记录
    if (rec.value(PERFORMER_DB_HEADER_KEY::Vids).toString() == vidPaths) {
      continue;  // unchange no need update
    }
    rec.setValue(PERFORMER_DB_HEADER_KEY::Vids, vidPaths);
    if (!setRecord(row, rec)) {
      LOG_W("update the %dth row record[%s] failed %s", row, qPrintable(performer), qPrintable(lastError().text()));
      return FD_ERROR_CODE::FD_SET_RECORDS_FAILED;
    }
    affectedRecordCnt++;
    LOG_D("Updated %d videos for %s", vidCount, qPrintable(performer));
  }

  if (affectedRecordCnt > 0) {
    submitSaveAllChanges();
  }

  LOG_D("`Vids` Fields of %d selected records get updated, %d videos find out", affectedRecordCnt, vidsCntFindOut);
  return affectedRecordCnt;
}

int CastDbModel::MigrateCastsTo(const QModelIndexList& selectedRowsIndexes, const QString& destinationPath) {
  if (selectedRowsIndexes.isEmpty()) {
    LOG_D("No need to migrate. nothing selected");
    return 0;
  }

  QString newOri;
  if (!CastBaseDb::IsNewOriFolderPathValid(destinationPath, m_imageHostPath, newOri)) {
    LOG_E("Abort Migrate destPath[%s] or newOri[%s] invalid", qPrintable(destinationPath), qPrintable(newOri));
    return FD_CAST_NEW_ORI_PATH_INVALID;
  }

  QDir imageHostDir{m_imageHostPath};
  int migrateCastCnt{0};
  for (const auto& indr : selectedRowsIndexes) {
    const int r = indr.row();
    QSqlRecord rec = record(r);
    const int ret = CastBaseDb::MigrateToNewOriFolder(rec, imageHostDir, newOri);
    if (ret < FD_ERROR_CODE::FD_SKIP) {
      return -1;
    }
    if (ret == FD_ERROR_CODE::FD_SKIP) {
      continue;
    }
    ++migrateCastCnt;
    setRecord(r, rec);
  }
  if (migrateCastCnt >= 0) {
    submitSaveAllChanges();
  }
  LOG_D("%d cast migrate to newOri[%s] succeed", migrateCastCnt, qPrintable(newOri));
  return migrateCastCnt;
}

bool CastDbModel::submitSaveAllChanges() {
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
  if (!submitAll()) {  // todo: cause view lose selection
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

bool CastDbModel::repopulate() {
  return select();
}

bool CastDbModel::onRevert() {
  if (!isDirty()) {
    LOG_D("Table not dirty.", "Skip revert");
    return false;
  }
  revertAll();
  select();
  LOG_D("Revert succeed", "All changes revert");
  return true;
}
