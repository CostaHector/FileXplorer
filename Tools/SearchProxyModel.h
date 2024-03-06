#ifndef SEARCHPROXYMODEL_H
#define SEARCHPROXYMODEL_H

#include <QComboBox>
#include <QDebug>
#include <QSortFilterProxyModel>

class SearchProxyModel : public QSortFilterProxyModel {
 public:
  SearchProxyModel(QObject* parent = nullptr);

  auto headerData(int section, Qt::Orientation orientation, int role) const -> QVariant override {
    return sourceModel()->headerData(section, orientation, role);
  }

  auto onSearchModeChanged(const QString& searchMode) -> void;

  auto startFilterWhenTextChanged(const QString& searchText) -> void;
  auto startFilterWhenTextChanges(const QString& searchText) -> void;

  auto ReturnPostOperation(const bool isPass, const QModelIndex& index) const -> bool;

  auto filterAcceptsRow(int source_row, const QModelIndex& source_parent) const -> bool override;

  void Reset() {
    m_searchFileContent.clear();
    m_nameFilters.clear();
    m_isCustomSearch = false;
  }

  void setSearchInFileContentsString(const QString& searchContent, const QStringList& nameSrcFilters);

  void initNameFilterDisable(bool hide) { m_nameFilterDisablesOrHidden = hide; }
  void setNameFilterDisable(bool hide) {
    initNameFilterDisable(hide);
    //    m_nameFilterDisablesOrHidden = hide;
  }

 private:
  QString m_lastTimeFilterStr;
  QString m_searchMode;

  QString m_searchFileContent;
  QList<QRegExp> m_nameFilters;
  bool m_isCustomSearch = false;
  bool m_nameFilterDisablesOrHidden = true;
  //  m_nameFilterDisablesOrHidden{PreferenceSettings().value("HIDE_ENTRIES_DONT_PASS_FILTER", true).toBool()}
};
#endif  // SEARCHPROXYMODEL_H
