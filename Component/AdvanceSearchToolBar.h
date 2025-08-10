#ifndef ADVANCESEARCHTOOLBAR_H
#define ADVANCESEARCHTOOLBAR_H

#include "FileSystemTypeFilter.h"
#include "SearchCaseMatterToolButton.h"
#include "SearchModeComboBox.h"

#include "AdvanceSearchModel.h"
#include "SearchProxyModel.h"

#include <QLineEdit>
#include <QToolBar>

class AdvanceSearchToolBar : public QToolBar {
 public:
  explicit AdvanceSearchToolBar(const QString& title = "Advance Search Toolbar", QWidget* parent = nullptr);

  void BindSearchAllModel(SearchProxyModel* searchProxyModel, AdvanceSearchModel* searchSourceModel);

  void onSearchTextChanges();
  void onSearchEnterAndApply();
  void onSearchModeChanged(const QString& newSearchModeText);

  void onGetFocus() {
    m_nameFilter->setFocus();
    m_nameFilter->selectAll();
  }

 private:
  void BindSearchProxyModel(SearchProxyModel* searchProxyModel);
  void BindSearchSourceModel(AdvanceSearchModel* searchSourceModel);

  QComboBox* m_nameFilterCB{nullptr};
  QLineEdit* m_nameFilter{nullptr};

  QComboBox* m_contentCB{nullptr};  // used for content search

  FileSystemTypeFilter* m_typeFilterButton = new FileSystemTypeFilter(this);
  SearchModeComboBox* m_searchModeComboBox = new SearchModeComboBox;
  SearchCaseMatterToolButton* m_searchCaseButton = new SearchCaseMatterToolButton(this);

  SearchProxyModel* _searchProxyModel = nullptr;
  AdvanceSearchModel* _searchSourceModel = nullptr;
};

#endif  // ADVANCESEARCHTOOLBAR_H
