#ifndef DATABASESEARCHTOOLBAR_H
#define DATABASESEARCHTOOLBAR_H

#include "PublicVariable.h"
#include <QComboBox>
#include <QTableView>
#include <QLayout>
#include <QToolBar>
#include <QLineEdit>

class DatabaseSearchToolBar : public QToolBar {
  Q_OBJECT
 public:
  explicit DatabaseSearchToolBar(const QString& title = tr("Database Search Toolbar"), QWidget* parent = nullptr)
      : m_tables(new QComboBox), m_searchLE(new QLineEdit), m_searchCB(new QComboBox) {
    m_tables->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);

    m_searchLE->addAction(QIcon(":/themes/SEARCH"), QLineEdit::LeadingPosition);
    m_searchLE->setClearButtonEnabled(true);

    m_searchCB->setLineEdit(m_searchLE);
    m_searchCB->addItem(QString("%1 like \"%\"").arg(DB_HEADER_KEY::ForSearch));
    m_searchCB->addItem(QString("%1 in (\"ts\", \"avi\")").arg(DB_HEADER_KEY::Type));
    m_searchCB->addItem(QString("%1 between 0 AND 1000000").arg(DB_HEADER_KEY::Size));
    m_searchCB->addItem(QString("%1 = \"E:/\"").arg(DB_HEADER_KEY::Driver));
    m_searchCB->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);

    addWidget(m_tables);
    addWidget(m_searchCB);

    m_tables->setFixedHeight(CONTROL_TOOLBAR_HEIGHT);
    m_searchCB->setFixedHeight(CONTROL_TOOLBAR_HEIGHT);
    setFixedHeight(CONTROL_TOOLBAR_HEIGHT);
    layout()->setSpacing(0);
    layout()->setContentsMargins(0, 0, 0, 0);
  }
  QComboBox* m_tables;
  QLineEdit* m_searchLE;
  QComboBox* m_searchCB;
};
#endif // DATABASESEARCHTOOLBAR_H
