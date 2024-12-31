#include "AdvanceSearchToolBar.h"
#include "PublicVariable.h"

#include <QDebug>

AdvanceSearchToolBar::AdvanceSearchToolBar(const QString& title, QWidget* parent) : QToolBar(title, parent) {
  m_nameFilter = new QLineEdit{""};
  m_nameFilter->addAction(QIcon(":img/SEARCH"), QLineEdit::LeadingPosition);
  m_nameFilter->setClearButtonEnabled(true);

  m_nameFilterCB = new QComboBox{this};
  m_nameFilterCB->setLineEdit(m_nameFilter);

  addWidget(m_nameFilterCB);
  addWidget(m_typeButton);
  addWidget(m_searchModeComboBox);
  addWidget(m_searchCaseButton);

  m_nameFilterCB->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
  m_nameFilterCB->addItem(PreferenceSettings().value(MemoryKey::ADVANCE_SEARCH_LINEEDIT_VALUE.name, MemoryKey::ADVANCE_SEARCH_LINEEDIT_VALUE.v).toString());
  m_nameFilterCB->addItem("nonporn|.*?html"); // grep -E \"contents\" --include="*.html"
  m_nameFilterCB->addItem("*.torrent");
  m_nameFilter->setPlaceholderText("Normal[abc], Wildcard[do?x], Regex[\\d{4}], Search for File Content[nonporn|.*?html]");
  layout()->setSpacing(0);
  layout()->setContentsMargins(0, 0, 0, 0);
}

void AdvanceSearchToolBar::BindSearchAllModel(SearchProxyModel* searchProxyModel, AdvanceSearchModel* searchSourceModel) {
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
  m_searchCaseButton->BindSearchModel(_searchProxyModel);
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
  PreferenceSettings().setValue(MemoryKey::ADVANCE_SEARCH_LINEEDIT_VALUE.name, m_nameFilter->text());
  if (_searchProxyModel) {
    _searchProxyModel->startFilterWhenTextChanged(m_nameFilter->text());
  }
}
