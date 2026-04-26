#include "SearchCaseMatterToolButton.h"
#include "SearchKey.h"
#include "Configuration.h"
#include <QMenu>

SearchCaseMatterToolButton::SearchCaseMatterToolButton(QWidget* parent) : QToolButton(parent) {
  SEARCH_NAME_CASE_SENSITIVE = new (std::nothrow) QAction("Name sensitive", this);
  SEARCH_NAME_CASE_SENSITIVE->setToolTip(QString("<b>%1 (%2)</b><br/> Name Case matter.").arg(SEARCH_NAME_CASE_SENSITIVE->text(), SEARCH_NAME_CASE_SENSITIVE->shortcut().toString()));
  SEARCH_NAME_CASE_SENSITIVE->setCheckable(true);
  SEARCH_NAME_CASE_SENSITIVE->setChecked(getConfig(SearchKey::NAME_CASE_SENSITIVE).toBool());

  SEARCH_CONTENTS_CASE_SENSITIVE = new (std::nothrow) QAction("Content sensitive", this);
  SEARCH_CONTENTS_CASE_SENSITIVE->setToolTip(QString("<b>%1 (%2)</b><br/> Content Case matter.").arg(SEARCH_CONTENTS_CASE_SENSITIVE->text(), SEARCH_CONTENTS_CASE_SENSITIVE->shortcut().toString()));
  SEARCH_CONTENTS_CASE_SENSITIVE->setCheckable(true);
  SEARCH_CONTENTS_CASE_SENSITIVE->setChecked(getConfig(SearchKey::CONTENTS_CASE_SENSITIVE).toBool());

  QMenu* m_searchCaseMenu = new (std::nothrow) QMenu{"Search case-sensitive", this};
  m_searchCaseMenu->addSection("search type(name or contents)");
  m_searchCaseMenu->addAction(SEARCH_NAME_CASE_SENSITIVE);
  m_searchCaseMenu->addAction(SEARCH_CONTENTS_CASE_SENSITIVE);
  m_searchCaseMenu->setToolTipsVisible(true);

  setMenu(m_searchCaseMenu);
  setIcon(QIcon(":img/NAME_STR_CASE"));
  setPopupMode(QToolButton::ToolButtonPopupMode::InstantPopup);
  setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
  setToolTip("Name/Content Case Sensitive Switch");

  connect(SEARCH_NAME_CASE_SENSITIVE, &QAction::triggered, this, [this](const bool bNameCaseSenstive){
    emit nameCaseSensitiveChanged(bNameCaseSenstive ? Qt::CaseSensitivity::CaseSensitive : Qt::CaseSensitivity::CaseInsensitive);
  });

  connect(SEARCH_CONTENTS_CASE_SENSITIVE, &QAction::triggered, this, [this](const bool bContentCaseSenstive){
    emit contentCaseSensitiveChanged(bContentCaseSenstive ? Qt::CaseSensitivity::CaseSensitive : Qt::CaseSensitivity::CaseInsensitive);
  });
}

SearchCaseMatterToolButton::~SearchCaseMatterToolButton() {
  setConfig(SearchKey::CONTENTS_CASE_SENSITIVE, curNameCaseSensitive() == Qt::CaseSensitivity::CaseSensitive);
  setConfig(SearchKey::NAME_CASE_SENSITIVE, curContentCaseSensitive() == Qt::CaseSensitivity::CaseSensitive);
}
