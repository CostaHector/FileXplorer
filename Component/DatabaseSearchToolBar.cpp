#include "DatabaseSearchToolBar.h"
#include "PublicVariable.h"
#include <QLayout>

DatabaseSearchToolBar::DatabaseSearchToolBar(const QString& title, QWidget* parent)
    : QToolBar(title, parent), m_tables(new QComboBox), m_searchLE(new QLineEdit), m_searchCB(new QComboBox) {
  m_tables->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);

  m_searchLE->addAction(QIcon(":img/SEARCH"), QLineEdit::LeadingPosition);
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
