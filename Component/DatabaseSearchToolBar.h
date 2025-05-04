#ifndef DATABASESEARCHTOOLBAR_H
#define DATABASESEARCHTOOLBAR_H

#include <QComboBox>
#include <QToolBar>
#include <QLineEdit>

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

class DatabaseSearchToolBar : public QToolBar {
  Q_OBJECT
 public:
  explicit DatabaseSearchToolBar(const QString& title = tr("Database Search Toolbar"),  //
                                 QWidget* parent = nullptr);
  Guid2RootPathComboxBox* m_tables{nullptr};
  QLineEdit* m_searchLE{nullptr};
  QComboBox* m_searchCB{nullptr};
};
#endif  // DATABASESEARCHTOOLBAR_H
