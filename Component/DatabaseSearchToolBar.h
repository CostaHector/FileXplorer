#ifndef DATABASESEARCHTOOLBAR_H
#define DATABASESEARCHTOOLBAR_H

#include <QToolBar>
#include <QComboBox>
#include <QLineEdit>
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

class MovieDBSearchToolBar : public QToolBar {
  Q_OBJECT
public:
  explicit MovieDBSearchToolBar(const QString& title, QWidget* parent);
  inline QString GetCurrentTableName() const {
    return m_tablesCB->CurrentTableName();
  }
  inline QString GetMovieTableRootPath() const {
    return m_tablesCB->CurrentRootPath();
  }
  inline QString GetCurrentWhereClause() const {
    return m_searchCB->currentText();
  }
  inline void SetWhereClause(const QString& newWhereClause) {
    m_searchCB->setCurrentText(newWhereClause);
  }
  QString AskUserDropWhichTable();
  void AddATable(const QString& newTableName);
  void InitTables(const QStringList& tbls);
  void InitCurrentIndex();

  inline void onGetFocus() {
    m_searchCB->setFocus();
    m_searchCB->lineEdit()->selectAll();
  }
signals:
  void movieTableChanged(const QString& newTable);
  void whereClauseChanged(const QString& whereClause);
private:
  void onQuickWhereClause();

  void subscribe();
  QComboBox* m_searchCB{nullptr};
  Guid2RootPathComboxBox* m_tablesCB{nullptr};
  QuickWhereClause* m_quickWhereClause{nullptr};
};

class CastDatabaseSearchToolBar : public QToolBar {
  Q_OBJECT
public:
  explicit CastDatabaseSearchToolBar(const QString& title, QWidget* parent);
  inline void onGetFocus() {
    m_nameClauseCB->setFocus();
    m_nameClauseCB->lineEdit()->selectAll();
  }
  inline void SetWhereClause(const QString& newWhereClause) {
    m_nameClauseCB->setCurrentText(newWhereClause);
  }
signals:
  void whereClauseChanged(const QString& whereClause);
private:
  void onQuickWhereClause();

  void subscribe();
  void Emit();
  QComboBox* m_nameClauseCB{nullptr};
  QuickWhereClause* m_quickWhereClause{nullptr};
};
#endif // DATABASESEARCHTOOLBAR_H
