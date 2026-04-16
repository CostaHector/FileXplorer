#include "StyleSheetMgr.h"
#include <QAction>
#include <QIcon>

StyleSheetMgr::StyleSheetMgr(QWidget* parent)
  : QWidget{parent} {
  m_searchLineEdit = new QLineEdit{this};
  m_startSearchAct = m_searchLineEdit->addAction(QIcon(":img/FILE_SYSTEM_FILTER"), QLineEdit::LeadingPosition);
  m_searchLineEdit->setClearButtonEnabled(true);

  m_styleSheetView = new StyleSheetTreeView{this};

  m_layout = new QVBoxLayout{this};
  m_layout->addWidget(m_searchLineEdit);
  m_layout->addWidget(m_styleSheetView);
  m_layout->setSpacing(0);
  m_layout->setContentsMargins(0, 0, 0, 0);

  connect(m_searchLineEdit, &QLineEdit::returnPressed, this, &StyleSheetMgr::onStartFilter);
  connect(m_startSearchAct, &QAction::triggered, this, &StyleSheetMgr::onStartFilter);

  setWindowTitle("StyleSheet Manager");
  setWindowIcon(QIcon{":styles/STYLESHEET_MGR"});
}

void StyleSheetMgr::onStartFilter() {
  m_styleSheetView->setFilter(m_searchLineEdit->text());
}
