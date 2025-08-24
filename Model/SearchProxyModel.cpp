#include "SearchProxyModel.h"
#include "PublicVariable.h"
#include "MemoryKey.h"
#include "PublicTool.h"
#include "PathTool.h"

SearchProxyModel::SearchProxyModel(QObject* parent)
  : QSortFilterProxyModel{parent}  //
{                                    //
  m_fileContentsCaseSensitive = Configuration().value(MemoryKey::SEARCH_CONTENTS_CASE_SENSITIVE.name, MemoryKey::SEARCH_CONTENTS_CASE_SENSITIVE.v).toBool();
  m_nameFiltersCaseSensitive = Configuration().value(MemoryKey::SEARCH_NAME_CASE_SENSITIVE.name, MemoryKey::SEARCH_NAME_CASE_SENSITIVE.v).toBool();
  m_nameFilterDisableOrHide = Configuration().value(MemoryKey::DISABLE_ENTRIES_DONT_PASS_FILTER.name, MemoryKey::DISABLE_ENTRIES_DONT_PASS_FILTER.v).toBool();
  const QString searchModeStr = Configuration().value(MemoryKey::ADVANCE_SEARCH_MODE.name, MemoryKey::ADVANCE_SEARCH_MODE.v).toString();
  initSearchMode(searchModeStr);

  static const Qt::CaseSensitivity CASE_SENSITIVE_BOOL_2_ENUM[2] = {Qt::CaseInsensitive, Qt::CaseSensitive};
  const Qt::CaseSensitivity nameCaseSensitive = CASE_SENSITIVE_BOOL_2_ENUM[(int)m_nameFiltersCaseSensitive];
  if (filterCaseSensitivity() != nameCaseSensitive) {
    setFilterCaseSensitivity(nameCaseSensitive);
  }
}

void SearchProxyModel::initSearchMode(const QString& searchMode) {
  using namespace SearchTools;
  m_searchMode = GetSearchModeEnum(searchMode);
}

void SearchProxyModel::setSearchMode(const QString& searchMode) {
  Configuration().setValue(MemoryKey::ADVANCE_SEARCH_MODE.name, searchMode);
  initSearchMode(searchMode);
  startFilterWhenTextChanged(m_nameRawString, m_contentRawText);
}

void SearchProxyModel::PrintRegexDebugMessage() const {
  qDebug("Search Mode: %d, file name raw[%s] caseSentive:%d, file content raw[%s] caseSentive:%d",  //
         (int)m_searchMode,                                                                         //
         qPrintable(m_nameRawString), (int)m_nameFiltersCaseSensitive,                              //
         qPrintable(m_contentRawText), (int)m_fileContentsCaseSensitive);                           //
  const auto& regex = filterRegularExpression();
  const QString& nameFilterPattern = regex.pattern();
  if (!regex.isValid()) {
    qWarning("Regex[%s] is invalid", qPrintable(nameFilterPattern));
    return;
  }
  if (m_searchMode == SearchTools::SEARCH_MODE::FILE_CONTENTS) {
    qDebug(R"(grep -iE \"%s\" --include="%s")", qPrintable(m_contentRawText), qPrintable(nameFilterPattern));
  } else {
    qDebug(R"(find ./ -type f -iname "%s")", qPrintable(m_nameRawString));
  }
}

void SearchProxyModel::startFilterWhenTextChanged(const QString& nameText, const QString& contentText) {
  // triggered by enter key
  if (_searchSourceModel) {
    _searchSourceModel->clearDisables();
  }
  m_nameRawString = nameText;
  m_contentRawText.clear();
  using namespace SearchTools;
  switch (m_searchMode) {
    case SEARCH_MODE::NORMAL:
      setFilterFixedString(nameText);
      break;
    case SEARCH_MODE::REGEX:
      setFilterRegularExpression(nameText);
      break;
    case SEARCH_MODE::FILE_CONTENTS:
      setContentFilter(contentText);
      setFilterRegularExpression(nameText);
      break;
    default:
      qWarning("Search mode[%d] not support", (int)m_searchMode);
      break;
  }
}

void SearchProxyModel::startFilterWhenTextChanges(const QString& nameText, const QString& contentText) {
  // triggered by text changed
  m_nameRawString = nameText;
  m_contentRawText = contentText;
  using namespace SearchTools;
  switch (m_searchMode) {
    case SEARCH_MODE::NORMAL:
      setFilterFixedString(nameText);
      break;
    case SEARCH_MODE::REGEX:
      setFilterRegularExpression(nameText);
      break;
    case SEARCH_MODE::FILE_CONTENTS:
      return;
    default:
      qWarning("Search mode[%d] not support", (int)m_searchMode);
      break;
  }
}

void SearchProxyModel::setContentFilter(const QString& contentText) {
  m_contentRawText = contentText;  // file contents full match m_fileContFilter
}

bool SearchProxyModel::ReturnPostOperation(const bool isPass, const QModelIndex& index) const {
  // here index is self.sourceModel().index(source_row, 0, source_parent)
  if (isPass) {
    if (_searchSourceModel)
      _searchSourceModel->removeDisable(index);
    return true;  // show and normal
  }
  if (!m_nameFilterDisableOrHide) {  // true: disable, false: hide
    return false;                    // hidden and normal
  }
  if (_searchSourceModel) {
    _searchSourceModel->appendDisable(index);
  }
  return true;  // show and gray/disable
}

bool SearchProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const {
  // when m_nameFilterDisablesOrHidden is true,  and not pass => hidden,         pass => show
  // when m_nameFilterDisablesOrHidden is false, and not pass => grayAndDisable, pass => show
  static constexpr int NAME_INDEX = 0;
  const QModelIndex nameModelIndex = sourceModel()->index(source_row, NAME_INDEX, source_parent);
  // 1. Check if file name pass the name filter
  const bool isFileNamePass = QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
  if (m_searchMode != SearchTools::SEARCH_MODE::FILE_CONTENTS) {
    return ReturnPostOperation(isFileNamePass, nameModelIndex);
  }
  // 2. Check if file content pass the content filter
  if (isFileNamePass) {
    const QString filePath = _searchSourceModel->filePath(nameModelIndex);
    const bool isFilePlainText{TYPE_FILTER::TEXT_TYPE_SET.contains('*' + PathTool::GetFileExtension(filePath))};
    // if search content is empty. always pass
    const bool isContentPass = isFilePlainText && (m_contentRawText.isEmpty() || CheckIfContentsContained(filePath, m_contentRawText));
    return ReturnPostOperation(isContentPass, nameModelIndex);
  }
  return ReturnPostOperation(false, nameModelIndex);
}

void SearchProxyModel::setNameFilterDisables(bool hide) {
  Configuration().setValue(MemoryKey::DISABLE_ENTRIES_DONT_PASS_FILTER.name, hide);
  initNameFilterDisables(hide);
  startFilterWhenTextChanged(m_nameRawString, m_contentRawText);
}

void SearchProxyModel::setFileContentsCaseSensitive(bool sensitive) {
  Configuration().setValue(MemoryKey::SEARCH_CONTENTS_CASE_SENSITIVE.name, sensitive);
  initFileContentsCaseSensitive(sensitive);
  startFilterWhenTextChanged(m_nameRawString, m_contentRawText);
}

void SearchProxyModel::setFileNameFiltersCaseSensitive(bool sensitive) {
  Configuration().setValue(MemoryKey::SEARCH_NAME_CASE_SENSITIVE.name, sensitive);
  initFileNameFiltersCaseSensitive(sensitive);
  const auto nameCaseSensitive = sensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
  if (filterCaseSensitivity() != nameCaseSensitive) {
    setFilterCaseSensitivity(nameCaseSensitive);
    return;
  }
  startFilterWhenTextChanged(m_nameRawString, m_contentRawText);
}

bool SearchProxyModel::CheckIfContentsContained(const QString& filePath, const QString& contained) const {
  if (contained.isEmpty()) {
    return true;
  }
  qDebug("Read file [%s]", qPrintable(filePath));
  const QString& fileContents = TextReader(filePath);
  // Todo: new feature on the way: regex match, parms text is a wildcard
  return fileContents.contains(contained, m_fileContentsCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
}
