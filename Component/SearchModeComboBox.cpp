#include "SearchModeComboBox.h"

#include "PublicVariable.h"
SearchModeComboBox::SearchModeComboBox(QWidget* parent) : QComboBox(parent) {
  addItems(MemoryKey::SEARCH_MODE_DEFAULT_VALUE.candidatePool);
  setCurrentText(PreferenceSettings().value(MemoryKey::SEARCH_MODE_DEFAULT_VALUE.name, MemoryKey::SEARCH_MODE_DEFAULT_VALUE.v).toString());
}

void SearchModeComboBox::BindSearchModel(SearchProxyModel* searchProxyModel) {
  if (searchProxyModel == nullptr) {
    qDebug("Don't try to bind nullptr searchProxyModel to SearchModeComboBox");
    return;
  }
  if (_searchProxyModel != nullptr) {
    qDebug("Don't try to rebind searchProxyModel to SearchModeComboBox");
    return;
  }
  _searchProxyModel = searchProxyModel;
  connect(this, &QComboBox::currentTextChanged, _searchProxyModel, &SearchProxyModel::setSearchMode);
}
