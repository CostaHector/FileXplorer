#ifndef ADVANCESEARCHTOOLBAR_H
#define ADVANCESEARCHTOOLBAR_H

#include "TypeFilterButton.h"
#include "SearchCaseMatterToolButton.h"
#include "SearchModeComboBox.h"

#include "AdvanceSearchModel.h"
#include "SearchProxyModel.h"

#include <QLineEdit>
#include <QToolBar>

class AdvanceSearchToolBar : public QToolBar {
 public:
  explicit AdvanceSearchToolBar(const QString& title, QWidget* parent);

  void BindSearchAllModel(SearchProxyModel* searchProxyModel, AdvanceSearchModel* searchSourceModel);

  void onSearchTextChanges();
  void onSearchEnterAndApply();
  void onSearchModeChanged(const QString& newSearchModeText);

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
  SearchModeComboBox* m_searchModeComboBox {nullptr};
  SearchCaseMatterToolButton* m_searchCaseButton {nullptr};

  SearchProxyModel* _searchProxyModel = nullptr;
  AdvanceSearchModel* _searchSourceModel = nullptr;
};

#endif  // ADVANCESEARCHTOOLBAR_H
