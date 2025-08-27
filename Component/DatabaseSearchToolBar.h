#ifndef DATABASESEARCHTOOLBAR_H
#define DATABASESEARCHTOOLBAR_H

#include <QToolBar>
#include <QComboBox>
#include <QLineEdit>
#include <QAction>
#include "QuickWhereClause.h"

// GUID_IN_UNDERSCORE | ROOTPATH
class Guid2RootPathComboxBox : public QComboBox {
public:
  explicit Guid2RootPathComboxBox(QWidget* parent = nullptr);
  void AddItem(const QString& guidUnderscore, const QString& rootPath);
  QString CurrentTableName() const;
  QString CurrentGuid() const;
  QString CurrentRootPath() const;
  QStringList ToQStringList() const;
};

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
  QComboBox* m_whereCB{nullptr};
  QuickWhereClause* m_quickWhereClause{nullptr};
  QAction* _QUICK_WHERE_CLAUSE_ACT{nullptr};
private:
  void EmitWhereClauseChangedSignal();
};

class MovieDBSearchToolBar : public DatabaseSearchToolBar {
  Q_OBJECT
public:
  explicit MovieDBSearchToolBar(const QString& title, QWidget* parent);
  inline QString GetCurrentTableName() const {
    return m_tablesCB->CurrentTableName();
  }
  inline QString GetMovieTableRootPath() const {
    return m_tablesCB->CurrentRootPath();
  }
  QString AskUserDropWhichTable();
  void AddATable(const QString& newTableName);
  void InitTables(const QStringList& tbls);
  void InitCurrentIndex();
signals:
  void movieTableChanged(const QString& newTable);
private:
  void extraSignalSubscribe() override;
  Guid2RootPathComboxBox* m_tablesCB{nullptr};
};

class CastDatabaseSearchToolBar : public DatabaseSearchToolBar {
public:
  explicit CastDatabaseSearchToolBar(const QString& title, QWidget* parent);
private:
  void extraSignalSubscribe() override {}
};
#endif // DATABASESEARCHTOOLBAR_H
