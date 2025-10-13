#ifndef SQLTABLEMODELPUB_H
#define SQLTABLEMODELPUB_H

#include <QSqlTableModel>

class SqlTableModelPub : public QSqlTableModel {
public:
  using QSqlTableModel::QSqlTableModel;
  bool SetFilterAndSelect(const QString &filter);
private:
  void ResetLastFilter(const QString &filter, bool bIsLastSucceed);
  QString m_lastFilter;
  bool m_lastFilterSucceed {false};
};
#endif // SQLTABLEMODELPUB_H
