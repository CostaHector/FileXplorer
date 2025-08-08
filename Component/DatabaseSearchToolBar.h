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

class DatabaseSearchToolBar : public QWidget {
public:
  explicit DatabaseSearchToolBar(const QString& title = "Database Search Toolbar", //
                                 QWidget* parent = nullptr);
  QComboBox* m_searchCB{nullptr};
  Guid2RootPathComboxBox* m_tablesCB{nullptr};
  QHBoxLayout* mLo{nullptr};
};
#endif // DATABASESEARCHTOOLBAR_H
