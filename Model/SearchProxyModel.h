#ifndef SEARCHPROXYMODEL_H
#define SEARCHPROXYMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include "AdvanceSearchModel.h"
#include "SearchTools.h"

class SearchProxyModel : public QSortFilterProxyModel {
 public:
  SearchProxyModel(QObject* parent = nullptr);
  void setSourceModel(QAbstractItemModel* sourceModel) override {
    _searchSourceModel = dynamic_cast<AdvanceSearchModel*>(sourceModel);
    if (_searchSourceModel == nullptr) {
      LOG_W("Error. setNameFilterDisables will not work.");
      return;
    }
    QSortFilterProxyModel::setSourceModel(sourceModel);
  }

  QVariant headerData(int section, Qt::Orientation orientation, int role) const override { return sourceModel()->headerData(section, orientation, role); }

  void initSearchMode(const QString& searchMode);
  void setSearchMode(const QString& searchMode);

  void startFilterWhenTextChanged(const QString& nameText, const QString& contentText);
  void startFilterWhenTextChanges(const QString& nameText, const QString& contentText);
  void setContentFilter(const QString& contentText);

  bool ReturnPostOperation(const bool isPass, const QModelIndex& index) const;

  bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

  void initNameFilterDisables(bool hide) { m_nameFilterDisableOrHide = hide; }
  void setNameFilterDisables(bool hide);

  inline void initFileContentsCaseSensitive(bool sensitive) { m_fileContentsCaseSensitive = sensitive; }
  void setFileContentsCaseSensitive(bool sensitive);

  inline void initFileNameFiltersCaseSensitive(bool sensitive) { m_nameFiltersCaseSensitive = sensitive; }
  void setFileNameFiltersCaseSensitive(bool sensitive);
  void PrintRegexDebugMessage() const;

 private:
  bool CheckIfContentsContained(const QString& filePath, const QString& contained) const;
  AdvanceSearchModel* _searchSourceModel{nullptr};

  SearchTools::SEARCH_MODE m_searchMode;
  QString m_nameRawString;
  QString m_contentRawText;

  bool m_nameFiltersCaseSensitive{false};
  bool m_fileContentsCaseSensitive{false};
  bool m_nameFilterDisableOrHide{false};  // true: disable, false: hide
};
#endif  // SEARCHPROXYMODEL_H
