#include "NavigationFavorites.h"
#include <QAction>
#include <QIcon>

NavigationFavorites::NavigationFavorites(QWidget* parent)
  : QWidget{parent} {
  m_searchLineEdit = new QLineEdit{this};
  m_startSearchAct = m_searchLineEdit->addAction(QIcon(":img/FILE_SYSTEM_FILTER"), QLineEdit::LeadingPosition);
  m_searchLineEdit->setClearButtonEnabled(true);

  m_favView = new FavoritesTreeView{this};

  m_layout = new QVBoxLayout{this};
  m_layout->addWidget(m_searchLineEdit);
  m_layout->addWidget(m_favView);
  m_layout->setSpacing(0);
  m_layout->setContentsMargins(0, 0, 0, 0);

  connect(m_searchLineEdit, &QLineEdit::returnPressed, this, &NavigationFavorites::onStartFilter);
  connect(m_startSearchAct, &QAction::triggered, this, &NavigationFavorites::onStartFilter);
}

void NavigationFavorites::onStartFilter() {
  m_favView->setFilter(m_searchLineEdit->text());
}
