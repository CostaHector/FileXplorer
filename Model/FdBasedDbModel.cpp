#include "FdBasedDbModel.h"
#include "DataFormatter.h"
#include "PathTool.h"
#include "NameTool.h"
#include "MountHelper.h"
#include "Logger.h"
#include <QSqlQuery>
#include "TableFields.h"

FdBasedDbModel::FdBasedDbModel(QObject* parent, QSqlDatabase con)  //
    : QSqlTableModel{parent, con} {
  setEditStrategy(QSqlTableModel::EditStrategy::OnManualSubmit);
}

void FdBasedDbModel::setTable(const QString& tableName) {
  const QString validTableName = tableName;
  QSqlTableModel::setTable(validTableName);
  const QString& guidFromTableName = GUID();
  m_rootPath = MountHelper::FindRootByGUIDWin(guidFromTableName);
  LOG_D("tableName:%s, GUID:%s, m_rootPath:%s", qPrintable(this->tableName()), qPrintable(guidFromTableName), qPrintable(m_rootPath));
}

QVariant FdBasedDbModel::data(const QModelIndex& idx, int role) const {
  if (!idx.isValid()) {
    return QVariant();
  }
  if (role == Qt::DisplayRole) {
    using namespace DataFormatter;
    if (idx.column() == MOVIE_TABLE::Size) {
      return formatFileSizeGMKB(QSqlTableModel::data(idx, Qt::ItemDataRole::DisplayRole).toLongLong());
    } else if (idx.column() == MOVIE_TABLE::Duration) {
      return formatDurationISOMs(QSqlTableModel::data(idx, Qt::ItemDataRole::DisplayRole).toLongLong());
    }
  }
  return QSqlTableModel::data(idx, role);
}

QString FdBasedDbModel::absolutePath(const QModelIndex& curIndex) const {
  const QModelIndex& preLeft = curIndex.siblingAtColumn(MOVIE_TABLE::PrePathLeft);
  const QModelIndex& preRight = curIndex.siblingAtColumn(MOVIE_TABLE::PrePathRight);
  return PathTool::Path2Join(data(preLeft, Qt::ItemDataRole::DisplayRole).toString(),  //
                             data(preRight, Qt::ItemDataRole::DisplayRole).toString());
}

QString FdBasedDbModel::fileName(const QModelIndex& curIndex) const {
  const QModelIndex& nameIndex = curIndex.siblingAtColumn(MOVIE_TABLE::Name);
  return data(nameIndex, Qt::ItemDataRole::DisplayRole).toString();
}

QString FdBasedDbModel::fullInfo(const QModelIndex& curIndex) const {
  return data(curIndex.siblingAtColumn(MOVIE_TABLE::Name)).toString()    //
         + '\t'                                                          //
         + data(curIndex.siblingAtColumn(MOVIE_TABLE::Size)).toString()  //
         + '\t'                                                          //
         + data(curIndex.siblingAtColumn(MOVIE_TABLE::PrePathRight)).toString();
}

void FdBasedDbModel::SetStudio(const QModelIndexList& tagColIndexes, const QString& studio) {
  foreach (const QModelIndex& ind, tagColIndexes) {
    setData(ind, studio);
  }
}

void FdBasedDbModel::SetCastOrTags(const QModelIndexList& tagColIndexes, const QString& sentence) {
  QString strLst{NameTool::CastTagSentenceParse2Str(sentence, true)};
  foreach (const QModelIndex& ind, tagColIndexes) {
    setData(ind, strLst);
  }
}

void FdBasedDbModel::AddCastOrTags(const QModelIndexList& tagColIndexes, const QString& sentence) {
  if (sentence.isEmpty()) {
    return;
  }
  QString beforeStr;
  foreach (const QModelIndex& ind, tagColIndexes) {
    beforeStr = QSqlTableModel::data(ind, Qt::DisplayRole).toString();
    if (!beforeStr.isEmpty()) {
      beforeStr += NameTool::CSV_COMMA;
    }
    beforeStr += sentence;
    setData(ind, NameTool::CastTagSentenceParse2Str(beforeStr, true));
  }
}

void FdBasedDbModel::RmvCastOrTags(const QModelIndexList& tagColIndexes, const QString& cast) {
  if (cast.isEmpty()) {
    return;
  }
  foreach (const QModelIndex& ind, tagColIndexes) {
    setData(ind, NameTool::CastTagSentenceRmvEle2Str(                                 //
                     QSqlTableModel::data(ind, Qt::DisplayRole).toString(),  //
                     cast));
  }
}
