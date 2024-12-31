#include "SearchProxyModel.h"
#include "PublicVariable.h"

SearchProxyModel::SearchProxyModel(QObject* parent)
    : QSortFilterProxyModel{parent},
      m_fileContentsCaseSensitive{PreferenceSettings().value(MemoryKey::SEARCH_CONTENTS_CASE_SENSITIVE.name, MemoryKey::SEARCH_CONTENTS_CASE_SENSITIVE.v).toBool()},
      m_nameFiltersCaseSensitive{PreferenceSettings().value(MemoryKey::SEARCH_NAME_CASE_SENSITIVE.name, MemoryKey::SEARCH_NAME_CASE_SENSITIVE.v).toBool()},
      m_nameFilterDisableOrHide{PreferenceSettings().value(MemoryKey::DISABLE_ENTRIES_DONT_PASS_FILTER.name, MemoryKey::DISABLE_ENTRIES_DONT_PASS_FILTER.v).toBool()} {
  initSearchMode(PreferenceSettings().value("ADVANCE_SEARCH_MODE", "Wildcard").toString());
  const auto nameCaseSensitive = m_nameFiltersCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
  if (filterCaseSensitivity() != nameCaseSensitive) {
    setFilterCaseSensitivity(nameCaseSensitive);
    return;
  }
}

auto SearchProxyModel::initSearchMode(const QString& searchMode) -> void {
  using namespace SearchTools;
  m_searchMode = GetSearchModeEnum(searchMode);
}

auto SearchProxyModel::setSearchMode(const QString& searchMode) -> void {
  PreferenceSettings().setValue("ADVANCE_SEARCH_MODE", searchMode);
  initSearchMode(searchMode);
  startFilterWhenTextChanged(m_searchSourceString);
}

auto SearchProxyModel::startFilterWhenTextChanged(const QString& searchText) -> void {
  // triggered by enter key
  using namespace SearchTools;
  Reset();
  if (_searchSourceModel) {
    _searchSourceModel->clearDisables();
  }
  m_searchSourceString = searchText;
  switch (m_searchMode) {
    case SEARCH_MODE::NORMAL:
      setFilterFixedString(searchText);
      break;
    case SEARCH_MODE::REGEX:
      setFilterRegularExpression(searchText);
      break;
    case SEARCH_MODE::WILDCARD:
      setFilterWildcard(searchText);
      break;
    case SEARCH_MODE::SEARCH_FOR_FILE_CONTENTS:
      changeCustomSearchNameAndContents(searchText);
      break;
    default:
      qWarning("Search mode[%d] not support", (int)m_searchMode);
      break;
  }
}

auto SearchProxyModel::startFilterWhenTextChanges(const QString& searchText) -> void {
  // triggered by text changed
  m_searchSourceString = searchText;
  using namespace SearchTools;
  switch (m_searchMode) {
    case SEARCH_MODE::NORMAL:
      setFilterFixedString(searchText);
      break;
    case SEARCH_MODE::REGEX:
      setFilterRegularExpression(searchText);
      break;
    case SEARCH_MODE::WILDCARD:
      setFilterWildcard(searchText);
      break;
    case SEARCH_MODE::SEARCH_FOR_FILE_CONTENTS:
      break;
    default:
      qWarning("Search mode[%d] not support", (int)m_searchMode);
      break;
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
  if (_searchSourceModel) {
    _searchSourceModel->appendDisable(index);
  }
  return true;  // show and gray/disable
}

auto SearchProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const -> bool {
  // expr: QRegularExpression = self.filterRegularExpression()
  // return expr.match(txt).hasMatch() hasExactMatch
  // when m_nameFilterDisablesOrHidden is true,  and not pass => hidden,         pass => show
  // when m_nameFilterDisablesOrHidden is false, and not pass => grayAndDisable, pass => show

  static constexpr int NAME_INDEX = 0;
  const QModelIndex nameModelIndex = sourceModel()->index(source_row, NAME_INDEX, source_parent);
  if (m_searchMode != SearchTools::SEARCH_MODE::SEARCH_FOR_FILE_CONTENTS) {
    const bool isPass = QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    return ReturnPostOperation(isPass, nameModelIndex);
  }

  static constexpr int PRE_PATH_INDEX = 4;
  const QModelIndex prepathModelIndex = sourceModel()->index(source_row, PRE_PATH_INDEX, source_parent);
  const QString filePrePath = sourceModel()->data(prepathModelIndex, Qt::DisplayRole).toString();
  const QString fileName = sourceModel()->data(nameModelIndex, Qt::DisplayRole).toString();
  const QString filePath = filePrePath + fileName;

  if (m_nameFilters.match(fileName).hasMatch()) {
    // if search content is empty, i.e., skip contents check
    const bool isPass = m_fileContentFilter.isEmpty() || CheckIfContentsContained(filePath, m_fileContentFilter);
    return ReturnPostOperation(isPass, nameModelIndex);
  }
  return ReturnPostOperation(false, nameModelIndex);
}

void SearchProxyModel::changeCustomSearchNameAndContents(const QString& searchText) {
  // will change m_fileContFilter, m_nameFilters
  if (searchText.isEmpty()) {
    m_fileContentFilter = "";  // file contents full match m_fileContFilter
    return;
  }

  // grep -E "contents" --include="*.txt"
  // contents can be omit
  QString fileNameStrFilter;
  int splitIndex = searchText.lastIndexOf('|');
  if (splitIndex == -1) {
    m_fileContentFilter = "";
    fileNameStrFilter = searchText;
  } else {
    m_fileContentFilter = searchText.left(splitIndex);
    fileNameStrFilter = searchText.mid(splitIndex + 1);
  }
  qDebug("grep -E \"%s\" --include=\"%s\"", qPrintable(m_fileContentFilter), qPrintable(fileNameStrFilter));
  m_nameFilters.setPattern(fileNameStrFilter);
  m_nameFilters.setPatternOptions(m_nameFiltersCaseSensitive ? QRegularExpression::PatternOption::NoPatternOption : QRegularExpression::PatternOption::CaseInsensitiveOption);
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
  if (!fi.open(QIODevice::Text | QIODevice::ReadOnly)) {
    return false;
  }
  QTextStream ts(&fi);
  ts.setCodec("UTF-8");
  const QString& fileContents = ts.readAll();
  fi.close();
  // Todo: new feature on the way: regex match, parms text is a wildcard
  return fileContents.contains(contained, m_fileContentsCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
}
