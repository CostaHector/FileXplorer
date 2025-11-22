#ifndef DATABASESEARCHTOOLBAR_H
#define DATABASESEARCHTOOLBAR_H

#include <QToolBar>
#include <QComboBox>
#include <QLineEdit>
#include <QAction>
#include "QuickWhereClauseDialog.h"

class DatabaseSearchToolBar: public QToolBar{
  Q_OBJECT
public:
  explicit DatabaseSearchToolBar(const QString& title, QWidget* parent);
  inline QString GetCurrentWhereClause() const {
    return m_whereCB->currentText();
  }
  inline void SetWhereClause(const QString& newWhereClause) {
    m_whereCB->setCurrentText(newWhereClause);
  }
  inline void onGetFocus() {
    m_whereCB->setFocus();
    m_whereCB->lineEdit()->selectAll();
  }
signals:
  void whereClauseChanged(const QString& whereClause);
protected:
  void onQuickWhereClause();
  void subscribe();
  virtual void extraSignalSubscribe() = 0;
  virtual QuickWhereClauseDialog* CreateQuickWhereClauseDialog() = 0;
  QComboBox* m_whereCB{nullptr};
  QuickWhereClauseDialog* m_quickWhereClause{nullptr};
  QAction* _QUICK_WHERE_CLAUSE_ACT{nullptr};
private:
  void EmitWhereClauseChangedSignal();
};

namespace MovieDBSearchToolBarMock {
inline std::pair<bool, QString>& QryDropWhichTableMock() {
  static std::pair<bool, QString> accept2TableNamePair;
  return accept2TableNamePair;
}
}

class MovieDBSearchToolBar : public DatabaseSearchToolBar {
  Q_OBJECT
public:
  explicit MovieDBSearchToolBar(const QString& title, QWidget* parent);
  inline QString GetCurrentTableName() const {
    return m_tablesCB->currentText();
  }
  QString GetMovieTableMountPath() const;
  QString AskUserDropWhichTable();
  void AddATable(const QString& newTableName);
  void InitTables(const QStringList& tbls);
  void InitCurrentIndex();
  QStringList toMovieTableCandidates() const;
signals:
  void movieTableChanged(const QString& newTable);
private:
  QuickWhereClauseDialog* CreateQuickWhereClauseDialog() override;
  void extraSignalSubscribe() override;
  QComboBox* m_tablesCB{nullptr};
};

class CastDatabaseSearchToolBar: public DatabaseSearchToolBar {
public:
  explicit CastDatabaseSearchToolBar(const QString& title, QWidget* parent);
private:
  QuickWhereClauseDialog* CreateQuickWhereClauseDialog() override;
  void extraSignalSubscribe() override {}
};
#endif // DATABASESEARCHTOOLBAR_H
