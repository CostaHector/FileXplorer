#ifndef ADVANCESEARCHTOOLBAR_H
#define ADVANCESEARCHTOOLBAR_H

#include "TypeFilterButton.h"
#include "SearchCaseMatterToolButton.h"
#include "SearchModeToolButton.h"

#include "AdvanceSearchModel.h"
#include "SearchProxyModel.h"

#include <QLineEdit>
#include <QToolBar>
#include <QComboBox>

class AdvanceSearchToolBar : public QToolBar {
 public:
  explicit AdvanceSearchToolBar(const QString& title, QWidget* parent);

  void BindSearchAllModel(SearchProxyModel* searchProxyModel, AdvanceSearchModel* searchSourceModel);

  void onSearchTextChanges();
  void onSearchEnterAndApply();
  void onSearchModeChanged(SearchTools::SEARCH_MODE searchMode);

  void onGetFocus() {
    m_nameFilterCB->setFocus();
    m_nameFilterCB->lineEdit()->selectAll();
  }


 private:
  void BindSearchProxyModel(SearchProxyModel* searchProxyModel);
  void BindSearchSourceModel(AdvanceSearchModel* searchSourceModel);

  QComboBox* m_nameFilterCB{nullptr};

  QComboBox* m_contentCB{nullptr};  // used for content search

  TypeFilterButton* m_searchFilterButton {nullptr};
  SearchModeToolButton* m_searchModeBtn {nullptr};
  SearchCaseMatterToolButton* m_searchCaseButton {nullptr};

  SearchProxyModel* _searchProxyModel = nullptr;
  AdvanceSearchModel* _searchSourceModel = nullptr;
};

#endif  // ADVANCESEARCHTOOLBAR_H
