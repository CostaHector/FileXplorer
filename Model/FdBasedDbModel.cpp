#include "FdBasedDbModel.h"
#include "public/DisplayEnhancement.h"
#include "public/PathTool.h"
#include "Tools/FileDescriptor/MountHelper.h"
#include <QSqlQuery>

FdBasedDbModel::FdBasedDbModel(QObject* parent, QSqlDatabase con)  //
    : QSqlTableModel{parent, con} {
  setEditStrategy(QSqlTableModel::EditStrategy::OnManualSubmit);
}

void FdBasedDbModel::setTable(const QString& tableName) {
  QSqlTableModel::setTable(tableName);
  const QString& guidFromTableName = GUID();
  m_rootPath = MountHelper::FindRootByGUIDWin(guidFromTableName);
  qDebug("GUID:%s, m_rootPath:%s", qPrintable(guidFromTableName), qPrintable(m_rootPath));
}

QVariant FdBasedDbModel::data(const QModelIndex& idx, int role) const {
  if (!idx.isValid()) {
    return QVariant();
  }
  if (role == Qt::DisplayRole) {
    using namespace FILE_PROPERTY_DSP;
    if (idx.column() == MOVIE_TABLE::Size) {
      return sizeToHumanReadFriendly(QSqlTableModel::data(idx, Qt::ItemDataRole::DisplayRole).toLongLong());
    } else if (idx.column() == MOVIE_TABLE::Duration) {
      return durationToHumanReadFriendly(QSqlTableModel::data(idx, Qt::ItemDataRole::DisplayRole).toLongLong());
    }
  }
  return QSqlTableModel::data(idx, role);
}

QString FdBasedDbModel::absolutePath(const QModelIndex& curIndex) const {
  const QModelIndex& preLeft = curIndex.siblingAtColumn(MOVIE_TABLE::PrePathLeft);
  const QModelIndex& preRight = curIndex.siblingAtColumn(MOVIE_TABLE::PrePathRight);
  return PATHTOOL::Path2Join(data(preLeft, Qt::ItemDataRole::DisplayRole).toString(),  //
                             data(preRight, Qt::ItemDataRole::DisplayRole).toString());
}
