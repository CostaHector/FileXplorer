#include "SearchProxyModel.h"
#include "Component/NotificatorFrame.h"
#include "PublicVariable.h"

SearchProxyModel::SearchProxyModel(QObject* parent)
  : QSortFilterProxyModel{parent},
    m_searchMode{"Normal"},
    m_isCustomSearch{m_searchMode == "Search for File Content"},
    m_fileContentsCaseSensitive{
      PreferenceSettings().value(MemoryKey::SEARCH_CONTENTS_CASE_SENSITIVE.name, MemoryKey::SEARCH_CONTENTS_CASE_SENSITIVE.v).toBool()},
    m_nameFiltersCaseSensitive{
      PreferenceSettings().value(MemoryKey::SEARCH_NAME_CASE_SENSITIVE.name, MemoryKey::SEARCH_NAME_CASE_SENSITIVE.v).toBool()},
    m_nameFilterDisableOrHide{
      PreferenceSettings().value(MemoryKey::DISABLE_ENTRIES_DONT_PASS_FILTER.name, MemoryKey::DISABLE_ENTRIES_DONT_PASS_FILTER.v).toBool()} {
  const auto nameCaseSensitive = m_nameFiltersCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
  if (filterCaseSensitivity() != nameCaseSensitive) {
    setFilterCaseSensitivity(nameCaseSensitive);
    return;
  }
}

auto SearchProxyModel::initSearchMode(const QString& searchMode) -> void {
  m_searchMode = searchMode;
}

auto SearchProxyModel::setSearchMode(const QString& searchMode) -> void {
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
    setFilterRegularExpression(searchText);
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
    setFilterRegularExpression(searchText);
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

  if (m_fileContFilter.isEmpty()) {
    return ReturnPostOperation(true, nameModelIndex);
  }

  static constexpr int PRE_PATH_INDEX = 4;
  const QModelIndex prepathModelIndex = sourceModel()->index(source_row, PRE_PATH_INDEX, source_parent);
  const QString filePrePath = sourceModel()->data(prepathModelIndex, Qt::DisplayRole).toString();
  const QString fileName = sourceModel()->data(nameModelIndex, Qt::DisplayRole).toString();
  const QString filePath = filePrePath + fileName;

  for (const auto& wildCardRe : m_nameFilters) {
    if (wildCardRe.match(fileName).hasMatch()) {
      bool isPass = CheckIfContentsContained(filePath, m_fileContFilter);
      return ReturnPostOperation(isPass, nameModelIndex);
    }
  }
  return ReturnPostOperation(false, nameModelIndex);
}

void SearchProxyModel::changeCustomSearchNameAndContents(const QString& searchText) {
  // will change m_isCustomSearch, m_fileContFilter, m_nameFilters
  m_isCustomSearch = true;
  m_nameFilters.clear();
  if (searchText.isEmpty()) {
    m_fileContFilter = ""; // file contents full match m_fileContFilter
    return;
  }

  // "nameFilter|contents"
  QStringList fileNameStrFilters;
  int splitIndex = searchText.lastIndexOf('|');
  if (splitIndex == -1) {
    fileNameStrFilters = searchText.split(',');
    m_fileContFilter = "";
  } else {
    fileNameStrFilters = searchText.left(splitIndex).split(',');
    m_fileContFilter = searchText.mid(splitIndex + 1);
  }
  foreach (const QString& fileNameWildCard, fileNameStrFilters) {
    const QString fileNameReg = QRegularExpression::wildcardToRegularExpression(fileNameWildCard);
    m_nameFilters.append(QRegularExpression{fileNameReg, m_nameFiltersCaseSensitive ? QRegularExpression::PatternOption::NoPatternOption : QRegularExpression::PatternOption::CaseInsensitiveOption});
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
  const auto nameCaseSensitive = sensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
  if (filterCaseSensitivity() != nameCaseSensitive) {
    setFilterCaseSensitivity(nameCaseSensitive);
    return;
  }
  startFilterWhenTextChanged(m_searchSourceString);
}

bool SearchProxyModel::CheckIfContentsContained(const QString& filePath, const QString& contained) const {
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
  // Todo: new feature on the way: regex match, parms text is a wildcard
  return fileContents.contains(contained, m_fileContentsCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
}
