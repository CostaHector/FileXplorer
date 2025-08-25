#ifndef DATABASESEARCHTOOLBAR_H
#define DATABASESEARCHTOOLBAR_H

#include <QComboBox>
#include <QLineEdit>
#include <QWidget>
#include <QHBoxLayout>

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

#include <QToolBar>
class MovieDBSearchToolBar : public QToolBar {
public:
  explicit MovieDBSearchToolBar(const QString& title = "Movie Database Search Toolbar", //
                                 QWidget* parent = nullptr);
  QComboBox* m_searchCB{nullptr};
  Guid2RootPathComboxBox* m_tablesCB{nullptr};
};

class CastDatabaseSearchToolBar : public QToolBar {
  Q_OBJECT
public:
  explicit CastDatabaseSearchToolBar(const QString& title = "Cast Database Search Toolbar", //
                                 QWidget* parent = nullptr);
  QComboBox* m_nameClauseCB{nullptr};
  QComboBox* m_otherClauseCB{nullptr};
signals:
  void whereClauseChanged(const QString& whereClause);
private:
  void subscribe();
  void Emit();
};
#endif // DATABASESEARCHTOOLBAR_H
