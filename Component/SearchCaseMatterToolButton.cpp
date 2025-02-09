#include "SearchCaseMatterToolButton.h"
#include "PublicVariable.h"

SearchCaseMatterToolButton::SearchCaseMatterToolButton(QWidget* parent) : QToolButton(parent) {
  m_searchCaseMenu->addSection(tr("search type(name or contents)"));
  m_searchCaseMenu->addAction(SEARCH_NAME_CASE_SENSITIVE);
  m_searchCaseMenu->addAction(SEARCH_CONTENTS_CASE_SENSITIVE);

  m_searchCaseMenu->setToolTipsVisible(true);

  for (auto* act : m_searchCaseMenu->actions()) {
    if (act->isSeparator()) {
      continue;
    }
    act->setCheckable(true);
  }

  SEARCH_NAME_CASE_SENSITIVE->setChecked(
      PreferenceSettings().value(MemoryKey::SEARCH_NAME_CASE_SENSITIVE.name, MemoryKey::SEARCH_NAME_CASE_SENSITIVE.v).toBool());
  SEARCH_CONTENTS_CASE_SENSITIVE->setChecked(
      PreferenceSettings().value(MemoryKey::SEARCH_CONTENTS_CASE_SENSITIVE.name, MemoryKey::SEARCH_CONTENTS_CASE_SENSITIVE.v).toBool());

  SEARCH_NAME_CASE_SENSITIVE->setToolTip(
      QString("<b>%1 (%2)</b><br/> Name Case matter.").arg(SEARCH_NAME_CASE_SENSITIVE->text(), SEARCH_NAME_CASE_SENSITIVE->shortcut().toString()));
  SEARCH_CONTENTS_CASE_SENSITIVE->setToolTip(QString("<b>%1 (%2)</b><br/> Content Case matter.")
                                                 .arg(SEARCH_CONTENTS_CASE_SENSITIVE->text(), SEARCH_CONTENTS_CASE_SENSITIVE->shortcut().toString()));

  setIcon(QIcon(":img/NAME_STR_CASE"));
  setPopupMode(QToolButton::ToolButtonPopupMode::InstantPopup);
  setMenu(m_searchCaseMenu);
  setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
}

void SearchCaseMatterToolButton::BindSearchModel(SearchProxyModel* searchProxyModel) {
  if (searchProxyModel == nullptr) {
    qDebug("Don't bind nullptr to _searchProxyModel");
  }
  if (_searchProxyModel != nullptr) {
    qDebug("Don't rebind to _searchProxyModel");
  }
  _searchProxyModel = searchProxyModel;
  connect(SEARCH_NAME_CASE_SENSITIVE, &QAction::triggered, _searchProxyModel, &SearchProxyModel::setFileNameFiltersCaseSensitive);
  connect(SEARCH_CONTENTS_CASE_SENSITIVE, &QAction::triggered, _searchProxyModel, &SearchProxyModel::setFileContentsCaseSensitive);
}
