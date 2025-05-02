#include "DatabaseSearchToolBar.h"
#include "public/PublicMacro.h"
#include "Tools/FileDescriptor/TableFields.h"
#include <QLayout>

DatabaseSearchToolBar::DatabaseSearchToolBar(const QString& title, QWidget* parent) : QToolBar(title, parent), m_tables(new QComboBox), m_searchLE(new QLineEdit), m_searchCB(new QComboBox) {
  m_tables->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);

  m_searchLE->addAction(QIcon(":img/SEARCH"), QLineEdit::LeadingPosition);
  m_searchLE->setClearButtonEnabled(true);

  m_searchCB->setLineEdit(m_searchLE);
  using namespace DB_HEADER_KEY;
  m_searchCB->addItem(QString("%1 like \"%\"").arg(VOLUME_ENUM_TO_STRING(ForSearch)));
  m_searchCB->addItem(QString("%1 in (\"ts\", \"avi\")").arg(VOLUME_ENUM_TO_STRING(Type)));
  m_searchCB->addItem(QString("%1 between 0 AND 1000000").arg(VOLUME_ENUM_TO_STRING(Size)));
  m_searchCB->addItem(QString("%1 = \"E:/\"").arg(VOLUME_ENUM_TO_STRING(Driver)));
  m_searchCB->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);

  addWidget(m_tables);
  addWidget(m_searchCB);

  layout()->setSpacing(0);
  layout()->setContentsMargins(0, 0, 0, 0);
}
