#ifndef DATABASESEARCHTOOLBAR_H
#define DATABASESEARCHTOOLBAR_H

#include <QComboBox>
#include <QToolBar>
#include <QLineEdit>

class DatabaseSearchToolBar : public QToolBar {
  Q_OBJECT
 public:
  explicit DatabaseSearchToolBar(const QString& title = tr("Database Search Toolbar"), QWidget* parent = nullptr);
  QComboBox* m_tables;
  QLineEdit* m_searchLE;
  QComboBox* m_searchCB;
};
#endif // DATABASESEARCHTOOLBAR_H
