#include "FdBasedDbModel.h"
#include "DataFormatter.h"
#include "PathTool.h"
#include "NameTool.h"
#include "MountHelper.h"
#include "Logger.h"
#include <QSqlQuery>
#include "MovieDBModelField.h"

FdBasedDbModel::FdBasedDbModel(QObject* parent, QSqlDatabase con)  //
    : SqlTableModelPub{parent, con} {
  setEditStrategy(QSqlTableModel::EditStrategy::OnManualSubmit);
}

void FdBasedDbModel::setTable(const QString& tableName) {
  QSqlTableModel::setTable(tableName);
  m_rootPath = MountPathTableNameMapper::toMountPath(tableName);
  LOG_D("tableName:%s, m_rootPath:%s", qPrintable(tableName), qPrintable(m_rootPath));
}

QVariant FdBasedDbModel::data(const QModelIndex& idx, int role) const {
  if (!idx.isValid()) {
    return QVariant();
  }
  if (role == Qt::DisplayRole) {
    using namespace DataFormatter;
    if (idx.column() == MovieDBModelField::Size) {
      return formatFileSizeGMKB(QSqlTableModel::data(idx, Qt::ItemDataRole::DisplayRole).toLongLong());
    } else if (idx.column() == MovieDBModelField::Duration) {
      return formatDurationISOMs(QSqlTableModel::data(idx, Qt::ItemDataRole::DisplayRole).toLongLong());
    }
  }
  return QSqlTableModel::data(idx, role);
}

QString FdBasedDbModel::absolutePath(const QModelIndex& curIndex) const {
  if (!curIndex.isValid()) {
    return {};
  }
  const QModelIndex& preLeft = curIndex.siblingAtColumn(MovieDBModelField::PrePathLeft);
  const QModelIndex& preRight = curIndex.siblingAtColumn(MovieDBModelField::PrePathRight);
  return PathTool::RMFComponent::joinParentPath(data(preLeft, Qt::ItemDataRole::DisplayRole).toString(), data(preRight, Qt::ItemDataRole::DisplayRole).toString());
}

QString FdBasedDbModel::fileName(const QModelIndex& curIndex) const {
  if (!curIndex.isValid()) {
    return {};
  }
  const QModelIndex& nameIndex = curIndex.siblingAtColumn(MovieDBModelField::Name);
  return data(nameIndex, Qt::ItemDataRole::DisplayRole).toString();
}

QString FdBasedDbModel::fullInfo(const QModelIndex& curIndex) const {
  if (!curIndex.isValid()) {
    return {};
  }
  return data(curIndex.siblingAtColumn(MovieDBModelField::Name)).toString()    //
         + '\t'                                                          //
         + data(curIndex.siblingAtColumn(MovieDBModelField::Size)).toString()  //
         + '\t'                                                          //
         + data(curIndex.siblingAtColumn(MovieDBModelField::PrePathRight)).toString();
}

void FdBasedDbModel::SetStudio(const QModelIndexList& tagColIndexes, const QString& studio) {
  foreach (const QModelIndex& ind, tagColIndexes) {
    setDataStatic(*this, ind, studio);
  }
}

void FdBasedDbModel::SetCastOrTags(const QModelIndexList& tagColIndexes, const QString& sentence) {
  QString strLst{NameTool::CastTagSentenceParse2Str(sentence, true)};
  foreach (const QModelIndex& ind, tagColIndexes) {
    setDataStatic(*this, ind, strLst);
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
    setDataStatic(*this, ind, NameTool::CastTagSentenceParse2Str(beforeStr, true));
  }
}

void FdBasedDbModel::RmvCastOrTags(const QModelIndexList& tagColIndexes, const QString& cast) {
  if (cast.isEmpty()) {
    return;
  }
  foreach (const QModelIndex& ind, tagColIndexes) {
    setDataStatic(*this, ind, NameTool::CastTagSentenceRmvEle2Str(                                 //
                     QSqlTableModel::data(ind, Qt::DisplayRole).toString(),  //
                     cast));
  }
}

QList<qint64> FdBasedDbModel::GetSelectionFileSizes(const QModelIndexList& indexes) const {
  QList<qint64> fileSizes;
  QModelIndex szInd;
  for (const QModelIndex& idx : indexes) {
    szInd = idx.siblingAtColumn(MovieDBModelField::Size);
    fileSizes.push_back(QSqlTableModel::data(szInd, Qt::ItemDataRole::DisplayRole).toLongLong());
  }
  return fileSizes;
}

QList<int> FdBasedDbModel::GetSelectionDurations(const QModelIndexList& indexes) const {
  QList<int> durations;
  QModelIndex durInd;
  for (const QModelIndex& idx : indexes) {
    durInd = idx.siblingAtColumn(MovieDBModelField::Duration);
    durations.push_back(QSqlTableModel::data(durInd, Qt::ItemDataRole::DisplayRole).toInt());
  }
  return durations;
}
