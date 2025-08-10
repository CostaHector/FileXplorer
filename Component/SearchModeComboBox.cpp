#include "SearchModeComboBox.h"
#include <QLayout>
#include "MemoryKey.h"
#include "SearchTools.h"
using namespace SearchTools;

SearchModeComboBox::SearchModeComboBox(QWidget* parent) : QComboBox{parent} {
  addItem(QIcon(":img/MATCH_EQUAL"), GetSearchModeStr(SEARCH_MODE::NORMAL));
  addItem(QIcon(":img/MATCH_REGEX"), GetSearchModeStr(SEARCH_MODE::REGEX));
  insertSeparator(count());
  addItem(QIcon(":img/SEARCH_SCOPE_CONTENT"), GetSearchModeStr(SEARCH_MODE::FILE_CONTENTS));

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
