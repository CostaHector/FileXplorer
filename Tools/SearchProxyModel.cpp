#include "SearchProxyModel.h"
#include "PublicVariable.h"

SearchProxyModel::SearchProxyModel(QObject* parent)
    : QSortFilterProxyModel{parent},
      m_searchMode{PreferenceSettings().value("SEARCH_MODE_DEFAULT_VALUE", "Normal").toString()} {}

auto SearchProxyModel::onSearchModeChanged(const QString& searchMode) -> void {
  PreferenceSettings().setValue("SEARCH_MODE_DEFAULT_VALUE", searchMode);
  m_searchMode = searchMode;
  startFilterWhenTextChanged(m_lastTimeFilterStr);
}

auto SearchProxyModel::startFilterWhenTextChanged(const QString& searchText) -> void {
  Reset();
  m_lastTimeFilterStr = searchText;
  if (m_searchMode == "Normal") {
    setFilterFixedString(searchText);
  } else if (m_searchMode == "Wildcard") {
    setFilterWildcard(searchText);
  } else if (m_searchMode == "Regex") {
    setFilterRegExp(searchText);
  } else if (m_searchMode == "Search for File Content") {
    // "contents|nameFilter"
    int splitIndex = searchText.lastIndexOf('|');
    if (splitIndex == -1) {
      qDebug("invalid search statement. no '|' find");
      return;
    }
    QString searchContent = searchText.left(splitIndex);
    QStringList nameSrcFilters = searchText.mid(splitIndex + 1).split(',');
    setSearchInFileContentsString(searchContent, nameSrcFilters);
  } else {
    qDebug("Error search mode[%s] not support", qPrintable(m_searchMode));
  }
}

auto SearchProxyModel::startFilterWhenTextChanges(const QString& searchText) -> void {
  m_lastTimeFilterStr = searchText;
  if (m_searchMode == "Normal") {
    setFilterFixedString(searchText);
  } else if (m_searchMode == "Wildcard") {
    setFilterWildcard(searchText);
  } else if (m_searchMode == "Regex") {
    setFilterRegExp(searchText);
  } else if (m_searchMode == "Search for File Content") {
    qDebug("Skip. press Enter/Return key to start Search for File Content");
  } else {
    qDebug("Error search mode[%s] not support", qPrintable(m_searchMode));
  }
}

auto SearchProxyModel::ReturnPostOperation(const bool isPass, const QModelIndex& index) const -> bool {
  static const QColor grayAndDisable = Qt::GlobalColor::gray;
  static const QColor normalBrush = Qt::GlobalColor::transparent;
  if (isPass) {
    sourceModel()->setData(index, normalBrush, Qt::BackgroundRole);
    return true;  // show and normal
  }
  if (m_nameFilterDisablesOrHidden) {
    return false;  // hidden and normal
  }
  sourceModel()->setData(index, grayAndDisable, Qt::BackgroundRole);
  return true;  // show and gray/disable
}

auto SearchProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const -> bool {
  // expr: QRegularExpression = self.filterRegularExpression()
  // txt = self.sourceModel()->data(self.sourceModel()->index(source_row, 0, source_parent), Qt.DisplayRole)
  // return expr.match(txt).hasMatch()

  // when m_nameFilterDisablesOrHidden is true,  and not pass => hidden,         pass => show
  // when m_nameFilterDisablesOrHidden is false, and not pass => grayAndDisable, pass => show

  static constexpr int PRE_PATH_INDEX = 4;
  static constexpr int NAME_INDEX = 0;

  const QModelIndex prepathModelIndex = sourceModel()->index(source_row, PRE_PATH_INDEX, source_parent);
  const QModelIndex nameModelIndex = sourceModel()->index(source_row, NAME_INDEX, source_parent);

  if (not m_isCustomSearch) {
    bool isPass = QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    return ReturnPostOperation(isPass, nameModelIndex);
  }

  // read a file
  static const auto readContentsAndCheckContains = [](const QString& filePath, const QString& text) -> bool {
    qDebug("Read file [%s]", qPrintable(filePath));
    QFile fi(filePath);
    if (not fi.open(QIODevice::Text | QIODevice::ReadOnly))
      return false;
    QTextStream ts(&fi);
    ts.setCodec("UTF-8");
    const QString& fileContents = ts.readAll();
    fi.close();
    // Todo: case-insensitive
    return fileContents.contains(text);  // New feature on the way: regex match, parms text is a wildcard
  };

  const QString filePrePath = sourceModel()->data(prepathModelIndex, Qt::DisplayRole).toString();
  const QString fileName = sourceModel()->data(nameModelIndex, Qt::DisplayRole).toString();
  const QString filePath = filePrePath + fileName;

  for (const auto& wildCardRe : m_nameFilters) {
    if (wildCardRe.exactMatch(fileName)) {
      bool isPass = readContentsAndCheckContains(filePath, m_searchFileContent);
      return ReturnPostOperation(isPass, nameModelIndex);
    }
  }
  return ReturnPostOperation(false, nameModelIndex);
}

void SearchProxyModel::setSearchInFileContentsString(const QString& searchContent, const QStringList& nameSrcFilters) {
  m_searchFileContent = searchContent;
  for (const auto& nameSrc : nameSrcFilters) {
    QRegExp wildCardRe(nameSrc);
    wildCardRe.setPatternSyntax(QRegExp::Wildcard);
    m_nameFilters.append(wildCardRe);
  }
  m_isCustomSearch = true;
  invalidateFilter();
}
