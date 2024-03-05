#include "SearchProxyModel.h"
#include "PublicVariable.h"

SearchProxyModel::SearchProxyModel(QObject* parent)
    : QSortFilterProxyModel{parent}, m_searchMode{PreferenceSettings().value("SEARCH_MODE_DEFAULT_VALUE", "Normal").toString()} {}

auto SearchProxyModel::onSearchModeChanged(const QString& searchMode) -> void {
  PreferenceSettings().setValue("SEARCH_MODE_DEFAULT_VALUE", searchMode);
  m_searchMode = searchMode;
  startFilter(m_lastTimeFilterStr);
}

auto SearchProxyModel::startFilter(const QString& searchText) -> void {
  m_lastTimeFilterStr = searchText;
  if (m_searchMode == "Normal") {
    setFilterFixedString(searchText);
  } else if (m_searchMode == "Wildcard") {
    setFilterWildcard(searchText);
  } else if (m_searchMode == "Regex") {
    setFilterRegExp(searchText);
  } else {
    qDebug("Error search mode[%s] not support", qPrintable(m_searchMode));
  }
}
