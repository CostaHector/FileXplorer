#include "SqlTableModelPub.h"
#include "NotificatorMacro.h"
#include <QSqlError>

SqlTableModelPub::SqlTableModelPub(QObject *parent, QSqlDatabase db) : QSqlTableModel{parent, db} {
  // 统统用手动提交
  setEditStrategy(QSqlTableModel::EditStrategy::OnManualSubmit);
  connect(this, &QAbstractItemModel::dataChanged, this, &SqlTableModelPub::EmitDirtyState);
  connect(this, &QAbstractItemModel::rowsInserted, this, &SqlTableModelPub::EmitDirtyState);
  connect(this, &QAbstractItemModel::rowsRemoved, this, &SqlTableModelPub::EmitDirtyState);
  connect(this, &QAbstractItemModel::modelReset, this, &SqlTableModelPub::EmitDirtyState);
  // 补漏. 需要监控headerDataChanged信号
  // 1. 若修改cache未入库, removeRows时直接删除暂存行, revertRow → beginRemoveRows/endRemoveRows → rowsRemoved
  // 2. 若修改cache已入库, removeRows时, headerDataChanged(Vertical)
  connect(this, &QAbstractItemModel::headerDataChanged, this, [this](Qt::Orientation orientation, int first, int last) {
    if (orientation == Qt::Orientation::Vertical) {
      EmitDirtyState();
    }
  });
}

void SqlTableModelPub::EmitDirtyState() {
  const bool dirty = isDirty();
  if (dirty == m_isLastDirty) {
    return;
  }
  m_isLastDirty = dirty;
  emit dirtyStatesChanged(dirty);
}

bool SqlTableModelPub::SetFilterAndSelect(const QString& filter) {
  // 空 filter 永远强制刷新，不参与去重; 非空 filter先去重
  if (!filter.isEmpty() && filter == m_lastFilter && !m_lastFilterSucceed) {
    // 同一个 filter 上次就失败，说明语法/字段有问题，不重复发 SQL
    LOG_INFO_P("Skip re-applying identical failed filter",
               "filter[%s] failed previously; not retrying until it changes",
               qPrintable(filter));
    return false;
  }
  if (isDirty()) {
    LOG_WARN_P("Filter change rejected: pending edits",
               "Table[%s] is dirty; refuse to change filter[%s] and re-select. "
               "Call submitAll() or revertAll() first.",
               qPrintable(tableName()), qPrintable(filter));
    return false;
  }
  setFilter(filter);
  const bool ok = select();
  if (!ok) {
    // 优先 databaseText()：SQLite 的具体原因（no such column / syntax error）在这里
    QString filterErrorDetail = lastError().databaseText();
    if (filterErrorDetail.isEmpty()) {
      filterErrorDetail = lastError().text();
    }
    LOG_WARN_P("Filter select failed", "Table[%s] filter[%s] failed: %s", qPrintable(tableName()), qPrintable(filter), qPrintable(filterErrorDetail));
  }
  m_lastFilter = filter;
  m_lastFilterSucceed = ok;
  return ok;
}

void SqlTableModelPub::sort(int column, Qt::SortOrder order) {
  if (isDirty()) {
    LOG_WARN_P("Sort rejected: pending edits",
               "Table[%s] is dirty; Sorting blocked to prevent data loss."
               "Call submitAll() or revertAll() first.",
               qPrintable(tableName()));
    return;
  }
  QSqlTableModel::sort(column, order);
}
