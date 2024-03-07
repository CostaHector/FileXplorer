#include "AdvanceSearchToolBar.h"
#include "PublicVariable.h"

#include <QDebug>

AdvanceSearchToolBar::AdvanceSearchToolBar(const QString& title, QWidget* parent) : QToolBar(title, parent) {
  m_nameFilter->addAction(QIcon(":/themes/SEARCH"), QLineEdit::LeadingPosition);
  m_nameFilter->setClearButtonEnabled(true);

  addWidget(m_nameFilter);
  addWidget(m_typeButton);
  addWidget(m_searchModeComboBox);

  m_nameFilter->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
  m_nameFilter->setFixedHeight(CONTROL_TOOLBAR_HEIGHT);
  m_typeButton->setFixedHeight(CONTROL_TOOLBAR_HEIGHT);
  m_searchModeComboBox->setFixedHeight(CONTROL_TOOLBAR_HEIGHT);
  setFixedHeight(CONTROL_TOOLBAR_HEIGHT);

  layout()->setSpacing(0);
  layout()->setContentsMargins(0, 0, 0, 0);

  m_nameFilter->setText(PreferenceSettings().value("ADVANCE_SEARCH_LINEEDIT_VALUE", "").toString());
  connect(m_searchModeComboBox, &QComboBox::currentTextChanged, this, &AdvanceSearchToolBar::UpdateLineEditReplaceHolder);
}

void AdvanceSearchToolBar::UpdateLineEditReplaceHolder(const QString& searchModeText) {
  m_nameFilter->setPlaceholderText("");
  if (searchModeText == "Normal") {
    m_nameFilter->setPlaceholderText("abc");
  } else if (searchModeText == "Wildcard") {
    m_nameFilter->setPlaceholderText("do?x");
  } else if (searchModeText == "Regex") {
    m_nameFilter->setPlaceholderText("\\d{4}");
  } else if (searchModeText == "Search for File Content") {
    m_nameFilter->setPlaceholderText("name|*.html,*.txt");
  }
}

void AdvanceSearchToolBar::BindSearchAllModel(SearchProxyModel* searchProxyModel, AdvanceSearchModel* searchSourceModel){
  // independant bind
  BindSearchProxyModel(searchProxyModel);
  BindSearchSourceModel(searchSourceModel);
  // interative bind
  m_typeButton->BindFileSystemModel(_searchSourceModel, _searchProxyModel);
}

void AdvanceSearchToolBar::BindSearchProxyModel(SearchProxyModel* searchProxyModel) {
  if (searchProxyModel == nullptr) {
    qDebug("don't try bind nullptr SearchProxyModel*");
    return;
  }
  if (_searchProxyModel != nullptr) {
    qDebug("don't try rebind SearchProxyModel*");
    return;
  }
  _searchProxyModel = searchProxyModel;
  connect(m_nameFilter, &QLineEdit::textChanged, _searchProxyModel, &SearchProxyModel::startFilterWhenTextChanges);
  connect(m_nameFilter, &QLineEdit::returnPressed, this, &AdvanceSearchToolBar::onSearchEnterAndApply);
  m_searchModeComboBox->BindSearchModel(_searchProxyModel);
}

void AdvanceSearchToolBar::BindSearchSourceModel(AdvanceSearchModel* searchSourceModel) {
  if (searchSourceModel == nullptr) {
    qDebug("don't try bind nullptr MySearchModel*");
    return;
  }
  if (_searchSourceModel != nullptr) {
    qDebug("don't try rebind MySearchModel*");
    return;
  }
  _searchSourceModel = searchSourceModel;
}

void AdvanceSearchToolBar::onSearchEnterAndApply() {
  // hint message placeholder
  PreferenceSettings().setValue("ADVANCE_SEARCH_LINEEDIT_VALUE", m_nameFilter->text());
  if (_searchProxyModel) {
    _searchProxyModel->startFilterWhenTextChanged(m_nameFilter->text());
  }
}
