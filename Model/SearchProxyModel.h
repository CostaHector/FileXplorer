#ifndef SEARCHPROXYMODEL_H
#define SEARCHPROXYMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include "AdvanceSearchModel.h"
#include "SearchTools.h"

class SearchProxyModel : public QSortFilterProxyModel {
public:
  using QSortFilterProxyModel::QSortFilterProxyModel;
  // only set value, no trigger filter
  void initSearchMode(SearchTools::SearchModeE newSearchMode) { m_searchMode = newSearchMode; }
  void initNameFilterDisables(bool hide) { m_nameFilterDisableOrHide = hide; }
  inline bool initFileNameFiltersCaseSensitive(Qt::CaseSensitivity sensitive) {
    m_nameFiltersCaseSensitive = sensitive;
    if (filterCaseSensitivity() != sensitive) {
      setFilterCaseSensitivity(sensitive);
      return false; // auto start
    }
    return true; // need manual start search
  }
  inline void initFileContentsCaseSensitive(Qt::CaseSensitivity sensitive) { m_fileContentsCaseSensitive = sensitive; }

  void setSearchMode(SearchTools::SearchModeE newSearchMode);
  void setNameFilterDisables(bool hide);
  void setFileContentsCaseSensitive(Qt::CaseSensitivity sensitive);
  void setFileNameFiltersCaseSensitive(Qt::CaseSensitivity sensitive);

  void setSourceModel(QAbstractItemModel* sourceModel) override {
    _searchSourceModel = dynamic_cast<AdvanceSearchModel*>(sourceModel);
    if (_searchSourceModel == nullptr) {
      LOG_W("Error. setNameFilterDisables will not work.");
      return;
    }
    QSortFilterProxyModel::setSourceModel(sourceModel);
  }

  QVariant headerData(int section, Qt::Orientation orientation, int role) const override { return sourceModel()->headerData(section, orientation, role); }

  void setContentFilter(const QString& contentText);
  void PrintRegexDebugMessage() const;
  void startFilterWhenTextChanged(const QString& nameText, const QString& contentText);
  void startFilterWhenTextChanges(const QString& nameText, const QString& contentText);
  bool ReturnPostOperation(const bool isPass, const QModelIndex& index) const;

  bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
  bool CheckIfContentsContained(const QString& filePath, const QString& contained) const;
  AdvanceSearchModel* _searchSourceModel{nullptr};

  QString m_nameRawString;
  QString m_contentRawText;

  SearchTools::SearchModeE m_searchMode{SearchTools::SearchModeE::NORMAL};
  Qt::CaseSensitivity m_nameFiltersCaseSensitive{Qt::CaseSensitivity::CaseInsensitive};
  Qt::CaseSensitivity m_fileContentsCaseSensitive{Qt::CaseSensitivity::CaseInsensitive};
  bool m_nameFilterDisableOrHide{false};  // true: disable, false: hide
};
#endif  // SEARCHPROXYMODEL_H
