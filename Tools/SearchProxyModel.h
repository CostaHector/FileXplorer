#ifndef SEARCHPROXYMODEL_H
#define SEARCHPROXYMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include "AdvanceSearchModel.h"

class SearchProxyModel : public QSortFilterProxyModel {
 public:
  SearchProxyModel(QObject* parent = nullptr);
  void setSourceModel(QAbstractItemModel* sourceModel) override {
    _searchSourceModel = dynamic_cast<AdvanceSearchModel*>(sourceModel);
    if (_searchSourceModel == nullptr) {
      qWarning("Error. setNameFilterDisables will not work.");
    }
    QSortFilterProxyModel::setSourceModel(sourceModel);
  }

  auto headerData(int section, Qt::Orientation orientation, int role) const -> QVariant override {
    return sourceModel()->headerData(section, orientation, role);
  }

  auto initSearchMode(const QString& searchMode) -> void;
  auto setSearchMode(const QString& searchMode) -> void;

  auto startFilterWhenTextChanged(const QString& searchText) -> void;
  auto startFilterWhenTextChanges(const QString& searchText) -> void;

  auto ReturnPostOperation(const bool isPass, const QModelIndex& index) const -> bool;

  auto filterAcceptsRow(int source_row, const QModelIndex& source_parent) const -> bool override;

  void Reset() {
    m_fileContents.clear();
    m_nameFilters.clear();
  }

  void changeCustomSearchNameAndContents(const QString& searchText);

  void initNameFilterDisables(bool hide) { m_nameFilterDisableOrHide = hide; }
  void setNameFilterDisables(bool hide);

  inline void initFileContentsCaseSensitive(bool sensitive) { m_fileContentsCaseSensitive = sensitive; }
  void setFileContentsCaseSensitive(bool sensitive);

  inline void initFileNameFiltersCaseSensitive(bool sensitive) {
    m_nameFiltersCaseSensitive = sensitive;
  }
  void setFileNameFiltersCaseSensitive(bool sensitive);

 private:
  auto CheckIfContentsContained(const QString& filePath, const QString& contained) const -> bool;
  AdvanceSearchModel* _searchSourceModel{nullptr};

  QString m_searchSourceString;

  QString m_searchMode;
  bool m_isCustomSearch;

  QString m_fileContents;
  QList<QRegExp> m_nameFilters;

  bool m_fileContentsCaseSensitive;
  bool m_nameFiltersCaseSensitive;

  bool m_nameFilterDisableOrHide;
};
#endif  // SEARCHPROXYMODEL_H
