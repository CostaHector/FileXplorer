#include "SearchModeComboBox.h"
#include <QLayout>
#include "PublicVariable.h"
SearchModeComboBox::SearchModeComboBox(QWidget* parent) : QComboBox{parent} {
  addItems({"Normal", "Wildcard", "Regex", "Search for File Content"});
  setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);

  setCurrentText(PreferenceSettings().value("ADVANCE_SEARCH_MODE", "Wildcard").toString());
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
