#ifndef SQLTABLEMODELPUB_H
#define SQLTABLEMODELPUB_H

#include <QSqlTableModel>

class SqlTableModelPub : public QSqlTableModel {
  Q_OBJECT
public:
  explicit SqlTableModelPub(QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase());
  bool SetFilterAndSelect(const QString &filter);
  void sort(int column, Qt::SortOrder order) override;

signals:
  void dirtyStatesChanged(bool bDirty);

private:
  void EmitDirtyState();
  QString m_lastFilter;
  bool m_lastFilterSucceed {false};
  bool m_isLastDirty{false};
};
#endif // SQLTABLEMODELPUB_H
