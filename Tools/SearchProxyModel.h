#ifndef SEARCHPROXYMODEL_H
#define SEARCHPROXYMODEL_H

#include <QAbstractItemModel>
#include <QComboBox>
#include <QDebug>
#include "AdvanceSearchModel.h"

class SearchProxyModel : public QSortFilterProxyModel {
 public:
  SearchProxyModel(QObject* parent = nullptr);
  void setSourceModel(QAbstractItemModel* sourceModel) override {
    _searchSourceModel = dynamic_cast<AdvanceSearchModel*>(sourceModel);
    QSortFilterProxyModel::setSourceModel(sourceModel);
    if (_searchSourceModel == nullptr) {
      qWarning("Error. setNameFilterDisables will not work.");
    }
  }

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
  }

  void setSearchInFileContentsString(const QString& searchContent, const QStringList& nameSrcFilters);

  void initNameFilterDisables(bool hide) { m_nameFilterHideOrDisable = hide; }
  void setNameFilterDisables(bool hide) { initNameFilterDisables(hide); }

 private:
  AdvanceSearchModel* _searchSourceModel{nullptr};

  QString m_lastTimeFilterStr;
  QString m_searchMode;

  QString m_searchFileContent;
  QList<QRegExp> m_nameFilters;
  bool m_nameFilterHideOrDisable;
  bool m_isCustomSearch;
};
#endif  // SEARCHPROXYMODEL_H
