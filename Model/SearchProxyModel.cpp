#include "SearchProxyModel.h"
#include "PublicVariable.h"
#include "MemoryKey.h"
#include "FileTool.h"
#include "PathTool.h"

void SearchProxyModel::setSearchMode(SearchTools::SearchModeE newSearchMode) {
  initSearchMode(newSearchMode);
  startFilterWhenTextChanged(m_nameRawString, m_contentRawText);
}

void SearchProxyModel::setNameFilterDisables(bool bGrayOrHide) {
  initNameFilterDisables(bGrayOrHide);
  startFilterWhenTextChanged(m_nameRawString, m_contentRawText);
}

void SearchProxyModel::setFileContentsCaseSensitive(Qt::CaseSensitivity sensitive) {
  initFileContentsCaseSensitive(sensitive);
  startFilterWhenTextChanged(m_nameRawString, m_contentRawText);
}

void SearchProxyModel::setFileNameFiltersCaseSensitive(Qt::CaseSensitivity sensitive) {
  if (!initFileNameFiltersCaseSensitive(sensitive)) {
    // already auto search started
    return;
  }
  startFilterWhenTextChanged(m_nameRawString, m_contentRawText);
}

void SearchProxyModel::PrintRegexDebugMessage() const {
  LOG_D("Search Mode: %d, file name raw[%s] caseSentive:%d, file content raw[%s] caseSentive:%d",  //
        (int)m_searchMode,                                                                         //
        qPrintable(m_nameRawString), (int)m_nameFiltersCaseSensitive,                              //
        qPrintable(m_contentRawText), (int)m_fileContentsCaseSensitive);                           //
  const auto& regex = filterRegularExpression();
  const QString& nameFilterPattern = regex.pattern();
  if (!regex.isValid()) {
    LOG_W("Regex[%s] is invalid", qPrintable(nameFilterPattern));
    return;
  }
  if (m_searchMode == SearchTools::SearchModeE::FILE_CONTENTS) {
    LOG_D(R"(grep -iE \"%s\" --include="%s")", qPrintable(m_contentRawText), qPrintable(nameFilterPattern));
  } else {
    LOG_D(R"(find ./ -type f -iname "%s")", qPrintable(m_nameRawString));
  }
}

void SearchProxyModel::startFilterWhenTextChanged(const QString& nameText, const QString& contentText) {
  // triggered by enter key
  if (_searchSourceModel != nullptr) {
    _searchSourceModel->clearDisables();
  }
  m_nameRawString = nameText;
  m_contentRawText.clear();
  using namespace SearchTools;
  switch (m_searchMode) {
    case SearchModeE::NORMAL:
      setFilterFixedString(nameText);
      break;
    case SearchModeE::REGEX:
      setFilterRegularExpression(nameText);
      break;
    case SearchModeE::FILE_CONTENTS:
      setContentFilter(contentText);
      setFilterRegularExpression(nameText);
      break;
    default:
      LOG_W("Search mode[%d] not support", (int)m_searchMode);
      break;
  }
#ifdef RUNNING_UNIT_TESTS
  ForceCompleteFilter();
#endif
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
  if (_searchSourceModel != nullptr) {
    _searchSourceModel->appendDisable(index);
  }
  return true;  // show and gray/disable
}

bool SearchProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const {
  // when m_nameFilterDisablesOrHidden is true,  and not pass => hidden,         pass => show
  // when m_nameFilterDisablesOrHidden is false, and not pass => grayAndDisable, pass => show
  const QModelIndex nameModelIndex = sourceModel()->index(source_row, FilePropertyHelper::Name, source_parent);
  // 1. Check if file name pass the name filter
  const bool isFileNamePass = QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
  if (m_searchMode != SearchTools::SearchModeE::FILE_CONTENTS) {
    return ReturnPostOperation(isFileNamePass, nameModelIndex);
  }
  // 2. Check if file content pass the content filter
  if (isFileNamePass) {
    const QString filePath = _searchSourceModel->filePath(nameModelIndex);
    const bool isFilePlainText{TYPE_FILTER::NON_BINARY_SET.contains('*' + PathTool::GetDotFileExtension(filePath))};
    // if search content is empty. always pass
    const bool isContentPass = isFilePlainText && (m_contentRawText.isEmpty() || CheckIfContentsContained(filePath, m_contentRawText));
    return ReturnPostOperation(isContentPass, nameModelIndex);
  }
  return ReturnPostOperation(false, nameModelIndex);
}

bool SearchProxyModel::CheckIfContentsContained(const QString& filePath, const QString& contained) const {
  if (contained.isEmpty()) {
    return true;
  }
  LOG_D("Read file [%s]", qPrintable(filePath));
  const QString& fileContents = FileTool::TextReader(filePath);
  // Todo: new feature on the way: regex match, parms text is a wildcard
  return fileContents.contains(contained, m_fileContentsCaseSensitive);
}

void SearchProxyModel::ForceCompleteFilter() {
  LOG_D("Force complete filter for rows");
  invalidateFilter();
  rowCount();
  return;
  // or below
  // CHECK_NULLPTR_RETURN_VOID(_searchSourceModel);
  // const int rc = _searchSourceModel->rowCount();
  // for (int i = 0; i < rc; ++i) {
  //   filterAcceptsRow(i, QModelIndex());
  // }
}
