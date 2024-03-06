#include "AdvanceSearchToolBar.h"
#include "PublicVariable.h"

#include <QDebug>

AdvanceSearchToolBar::AdvanceSearchToolBar(const QString& title, QWidget* parent) : QToolBar(title, parent) {
  m_nameFilter->setFixedHeight(CONTROL_TOOLBAR_HEIGHT);
  m_typeButton->setFixedHeight(CONTROL_TOOLBAR_HEIGHT);
  m_searchModeComboBox->setFixedHeight(CONTROL_TOOLBAR_HEIGHT);

  addWidget(m_nameFilter);
  addWidget(m_typeButton);
  addWidget(m_searchModeComboBox);

  setFixedHeight(CONTROL_TOOLBAR_HEIGHT);
  layout()->setSpacing(0);
  layout()->setContentsMargins(0, 0, 0, 0);

  m_nameFilter->setText(PreferenceSettings().value("ADVANCE_SEARCH_LINEEDIT_VALUE", "").toString());
}

void AdvanceSearchToolBar::BindProxyModel(SearchProxyModel* proxyModel) {
  if (proxyModel == nullptr) {
    qDebug("don't try bind nullptr SearchProxyModel*");
    return;
  }
  if (_proxyModel != nullptr) {
    qDebug("don't try rebind SearchProxyModel*");
    return;
  }
  _proxyModel = proxyModel;
  connect(m_nameFilter, &QLineEdit::textChanged, _proxyModel, &SearchProxyModel::startFilterWhenTextChanges);
  connect(m_nameFilter, &QLineEdit::returnPressed, this, &AdvanceSearchToolBar::onSearchEnterAndApply);
  m_searchModeComboBox->BindSearchModel(_proxyModel);
  //    _model->BindProxyModel(m_proxyModel);
}

void AdvanceSearchToolBar::onSearchEnterAndApply() {
  PreferenceSettings().setValue("ADVANCE_SEARCH_LINEEDIT_VALUE", m_nameFilter->text());
  if (_proxyModel) {
    _proxyModel->startFilterWhenTextChanged(m_nameFilter->text());
  }
}

void AdvanceSearchToolBar::BindSourceModel(MySearchModel* model) {
  if (model == nullptr) {
    qDebug("don't try bind nullptr MySearchModel*");
    return;
  }
  if (_model != nullptr) {
    qDebug("don't try rebind MySearchModel*");
    return;
  }
  _model = model;
  m_typeButton->BindFileSystemModel(_model);
}
