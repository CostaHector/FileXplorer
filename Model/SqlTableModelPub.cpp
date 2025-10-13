#include "SqlTableModelPub.h"
#include "NotificatorMacro.h"

void SqlTableModelPub::ResetLastFilter(const QString &filter, bool bIsLastSucceed) {
  m_lastFilter = filter;
  m_lastFilterSucceed = bIsLastSucceed;
  if (!m_lastFilterSucceed) {
    LOG_ERR_NP("filter failed", filter);
    return;
  }
  LOG_D("filter changed to %s", qPrintable(filter));
}

bool SqlTableModelPub::SetFilterAndSelect(const QString &filter) {
  setFilter(filter);
  if(select()) {
    ResetLastFilter(filter, true);
    return true;
  }
  if(!m_lastFilterSucceed && filter == m_lastFilter) {
    return false; // skip
  }
  const QString lastTable = tableName();
  setTable("");
  setTable(lastTable);
  setFilter(filter);
  if(select()) {
    ResetLastFilter(filter, true);
    return true;
  }
  ResetLastFilter(filter, false);
  return false;
}
