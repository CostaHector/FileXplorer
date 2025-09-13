#include "SearchCaseMatterToolButton.h"
#include "MemoryKey.h"
#include <QMenu>

SearchCaseMatterToolButton::SearchCaseMatterToolButton(QWidget* parent) : QToolButton(parent) {
  SEARCH_NAME_CASE_SENSITIVE = new (std::nothrow) QAction("Name sensitive", this);
  SEARCH_NAME_CASE_SENSITIVE->setToolTip(QString("<b>%1 (%2)</b><br/> Name Case matter.").arg(SEARCH_NAME_CASE_SENSITIVE->text(), SEARCH_NAME_CASE_SENSITIVE->shortcut().toString()));
  SEARCH_NAME_CASE_SENSITIVE->setCheckable(true);
  SEARCH_NAME_CASE_SENSITIVE->setChecked(Configuration().value(MemoryKey::SEARCH_NAME_CASE_SENSITIVE.name, MemoryKey::SEARCH_NAME_CASE_SENSITIVE.v).toBool());

  SEARCH_CONTENTS_CASE_SENSITIVE = new (std::nothrow) QAction("Content sensitive", this);
  SEARCH_CONTENTS_CASE_SENSITIVE->setToolTip(QString("<b>%1 (%2)</b><br/> Content Case matter.").arg(SEARCH_CONTENTS_CASE_SENSITIVE->text(), SEARCH_CONTENTS_CASE_SENSITIVE->shortcut().toString()));
  SEARCH_CONTENTS_CASE_SENSITIVE->setCheckable(true);
  SEARCH_CONTENTS_CASE_SENSITIVE->setChecked(Configuration().value(MemoryKey::SEARCH_CONTENTS_CASE_SENSITIVE.name, MemoryKey::SEARCH_CONTENTS_CASE_SENSITIVE.v).toBool());

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
  Configuration().setValue(MemoryKey::SEARCH_CONTENTS_CASE_SENSITIVE.name, curNameCaseSensitive() == Qt::CaseSensitivity::CaseSensitive);
  Configuration().setValue(MemoryKey::SEARCH_NAME_CASE_SENSITIVE.name, curContentCaseSensitive() == Qt::CaseSensitivity::CaseSensitive);
}
