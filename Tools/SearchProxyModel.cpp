#include "SearchProxyModel.h"
#include "Component/NotificatorFrame.h"
#include "PublicVariable.h"

SearchProxyModel::SearchProxyModel(QObject* parent)
    : QSortFilterProxyModel{parent},
      m_searchMode{PreferenceSettings().value(MemoryKey::SEARCH_MODE_DEFAULT_VALUE.name, MemoryKey::SEARCH_MODE_DEFAULT_VALUE.v).toString()},
      m_isCustomSearch{m_searchMode == "Search for File Content"},
      m_fileContentsCaseSensitive{
          PreferenceSettings().value(MemoryKey::SEARCH_CONTENTS_CASE_SENSITIVE.name, MemoryKey::SEARCH_CONTENTS_CASE_SENSITIVE.v).toBool()},
      m_nameFiltersCaseSensitive{
          PreferenceSettings().value(MemoryKey::SEARCH_NAME_CASE_SENSITIVE.name, MemoryKey::SEARCH_NAME_CASE_SENSITIVE.v).toBool()},
      m_nameFilterDisableOrHide{
          PreferenceSettings().value(MemoryKey::DISABLE_ENTRIES_DONT_PASS_FILTER.name, MemoryKey::DISABLE_ENTRIES_DONT_PASS_FILTER.v).toBool()} {}

auto SearchProxyModel::initSearchMode(const QString& searchMode) -> void {
  m_searchMode = searchMode;
}

auto SearchProxyModel::setSearchMode(const QString& searchMode) -> void {
  PreferenceSettings().setValue(MemoryKey::SEARCH_MODE_DEFAULT_VALUE.name, searchMode);
  initSearchMode(searchMode);
  startFilterWhenTextChanged(m_searchSourceString);
}

auto SearchProxyModel::startFilterWhenTextChanged(const QString& searchText) -> void {
  // triggered by enter key
  Reset();
  if (_searchSourceModel)
    _searchSourceModel->clearDisables();
  m_searchSourceString = searchText;
  m_isCustomSearch = m_searchMode == "Search for File Content";
  if (m_searchMode == "Normal") {
    setFilterFixedString(searchText);
  } else if (m_searchMode == "Wildcard") {
    setFilterWildcard(searchText);
  } else if (m_searchMode == "Regex") {
    setFilterRegExp(searchText);
  } else if (m_searchMode == "Search for File Content") {
    changeCustomSearchNameAndContents(searchText);
  } else {
    qDebug("Error search mode[%s] not support", qPrintable(m_searchMode));
  }
}

auto SearchProxyModel::startFilterWhenTextChanges(const QString& searchText) -> void {
  // triggered by text changed
  m_searchSourceString = searchText;
  m_isCustomSearch = m_searchMode == "Search for File Content";
  if (m_searchMode == "Normal") {
    setFilterFixedString(searchText);
  } else if (m_searchMode == "Wildcard") {
    setFilterWildcard(searchText);
  } else if (m_searchMode == "Regex") {
    setFilterRegExp(searchText);
  } else if (m_searchMode == "Search for File Content") {
    ;
  } else {
    qDebug("Error search mode[%s] not support", qPrintable(m_searchMode));
  }
}

auto SearchProxyModel::ReturnPostOperation(const bool isPass, const QModelIndex& index) const -> bool {
  // here index is self.sourceModel().index(source_row, 0, source_parent)
  if (isPass) {
    if (_searchSourceModel)
      _searchSourceModel->removeDisable(index);
    return true;  // show and normal
  }
  if (not m_nameFilterDisableOrHide) {  // true: disable, false: hide
    return false;                       // hidden and normal
  }
  if (_searchSourceModel)
    _searchSourceModel->appendDisable(index);
  return true;  // show and gray/disable
}

auto SearchProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const -> bool {
  // expr: QRegularExpression = self.filterRegularExpression()
  // return expr.match(txt).hasMatch() hasExactMatch
  // when m_nameFilterDisablesOrHidden is true,  and not pass => hidden,         pass => show
  // when m_nameFilterDisablesOrHidden is false, and not pass => grayAndDisable, pass => show

  static constexpr int NAME_INDEX = 0;
  const QModelIndex nameModelIndex = sourceModel()->index(source_row, NAME_INDEX, source_parent);
  if (not m_isCustomSearch) {
    bool isPass = QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    return ReturnPostOperation(isPass, nameModelIndex);
  }

  if (m_fileContents.isEmpty()) {
    return ReturnPostOperation(true, nameModelIndex);
  }

  static constexpr int PRE_PATH_INDEX = 4;
  const QModelIndex prepathModelIndex = sourceModel()->index(source_row, PRE_PATH_INDEX, source_parent);
  const QString filePrePath = sourceModel()->data(prepathModelIndex, Qt::DisplayRole).toString();
  const QString fileName = sourceModel()->data(nameModelIndex, Qt::DisplayRole).toString();
  const QString filePath = filePrePath + fileName;

  for (const auto& wildCardRe : m_nameFilters) {
    if (wildCardRe.exactMatch(fileName)) {
      bool isPass = CheckIfContentsContained(filePath, m_fileContents);
      return ReturnPostOperation(isPass, nameModelIndex);
    }
  }
  return ReturnPostOperation(false, nameModelIndex);
}

void SearchProxyModel::changeCustomSearchNameAndContents(const QString& searchText) {
  // will change m_isCustomSearch, m_fileContents, m_nameFilters
  m_isCustomSearch = true;

  if (searchText.isEmpty()) {
    QRegExp wildCardRe("*");
    wildCardRe.setPatternSyntax(QRegExp::Wildcard);
    m_nameFilters = {wildCardRe};
    m_fileContents = "";
    return;
  }

  // "nameFilter|contents"
  QStringList nameSrcFilters;
  int splitIndex = searchText.lastIndexOf('|');
  if (splitIndex == -1) {
    nameSrcFilters = searchText.split(',');
    m_fileContents = "";
  } else {
    nameSrcFilters = searchText.left(splitIndex).split(',');
    m_fileContents = searchText.mid(splitIndex + 1);
  }
  foreach (const QString& nameSrc, nameSrcFilters) {
    QRegExp wildCardRe(nameSrc);
    wildCardRe.setPatternSyntax(QRegExp::Wildcard);
    if (not wildCardRe.isValid()) {
      qWarning("Invalid wildcard[%s]. Disable filter and pass everything.", qPrintable(nameSrc));
      Notificator::warning("Invalid wildcard[%1]. Disable filter and pass everything.", nameSrc);
      m_fileContents = "";
      break;
    }
    wildCardRe.setCaseSensitivity(m_nameFiltersCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
    m_nameFilters.append(wildCardRe);
  }
  invalidateFilter();
}

void SearchProxyModel::setNameFilterDisables(bool hide) {
  PreferenceSettings().setValue(MemoryKey::DISABLE_ENTRIES_DONT_PASS_FILTER.name, hide);
  initNameFilterDisables(hide);
  startFilterWhenTextChanged(m_searchSourceString);
}

void SearchProxyModel::setFileContentsCaseSensitive(bool sensitive) {
  PreferenceSettings().setValue(MemoryKey::SEARCH_CONTENTS_CASE_SENSITIVE.name, sensitive);
  initFileContentsCaseSensitive(sensitive);
  startFilterWhenTextChanged(m_searchSourceString);
}

void SearchProxyModel::setFileNameFiltersCaseSensitive(bool sensitive) {
  PreferenceSettings().setValue(MemoryKey::SEARCH_NAME_CASE_SENSITIVE.name, sensitive);
  initFileNameFiltersCaseSensitive(sensitive);
  startFilterWhenTextChanged(m_searchSourceString);
}

auto SearchProxyModel::CheckIfContentsContained(const QString& filePath, const QString& contained) const -> bool {
  qDebug("Read file [%s]", qPrintable(filePath));
  if (contained.isEmpty()) {
    return true;
  }
  QFile fi(filePath);
  if (not fi.open(QIODevice::Text | QIODevice::ReadOnly))
    return false;
  QTextStream ts(&fi);
  ts.setCodec("UTF-8");
  const QString& fileContents = ts.readAll();
  fi.close();
  return fileContents.contains(contained, m_fileContentsCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
  // Todo: new feature on the way: regex match, parms text is a wildcard
};
