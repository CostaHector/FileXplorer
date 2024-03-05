#include "SearchModeComboBox.h"

#include "PublicVariable.h"
SearchModeComboBox::SearchModeComboBox(QWidget* parent):QComboBox(parent){
  addItems({"Normal", "Wildcard", "Regex"});
  setCurrentText(PreferenceSettings().value("SEARCH_MODE_DEFAULT_VALUE", "Normal").toString());
}

void SearchModeComboBox::BindSearchModel(SearchProxyModel* searchProxyModel) {
  if (searchProxyModel == nullptr){
    qDebug("Don't try to bind nullptr searchProxyModel to SearchModeComboBox");
    return;
  }
  if (_searchProxyModel != nullptr){
    qDebug("Don't try to rebind searchProxyModel to SearchModeComboBox");
    return;
  }
  _searchProxyModel = searchProxyModel;
  connect(this, &QComboBox::currentTextChanged, _searchProxyModel, &SearchProxyModel::onSearchModeChanged);
}
