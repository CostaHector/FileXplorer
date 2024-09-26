#ifndef ADVANCESEARCHTOOLBAR_H
#define ADVANCESEARCHTOOLBAR_H

#include "Component/FileSystemTypeFilter.h"
#include "Component/SearchCaseMatterToolButton.h"
#include "Component/SearchModeComboBox.h"

#include "AdvanceSearchModel.h"
#include "Tools/SearchProxyModel.h"

#include <QLineEdit>
#include <QMainWindow>
#include <QToolBar>

#include <QLayout>

class AdvanceSearchToolBar : public QToolBar {
  Q_OBJECT
 public:
  explicit AdvanceSearchToolBar(const QString& title = tr("Advance Search Toolbar"), QWidget* parent = nullptr);

  void BindSearchAllModel(SearchProxyModel* searchProxyModel, AdvanceSearchModel* searchSourceModel);

  void onSearchEnterAndApply();

  void onGetFocus() {
    m_nameFilter->setFocus();
    m_nameFilter->selectAll();
  }

 private:
  void BindSearchProxyModel(SearchProxyModel* searchProxyModel);
  void BindSearchSourceModel(AdvanceSearchModel* searchSourceModel);

  QComboBox* m_nameFilterCB{nullptr};
  QLineEdit* m_nameFilter{nullptr};
  FileSystemTypeFilter* m_typeButton = new FileSystemTypeFilter(this);
  SearchModeComboBox* m_searchModeComboBox = new SearchModeComboBox;
  SearchCaseMatterToolButton* m_searchCaseButton = new SearchCaseMatterToolButton(this);

  SearchProxyModel* _searchProxyModel = nullptr;
  AdvanceSearchModel* _searchSourceModel = nullptr;
};

#endif  // ADVANCESEARCHTOOLBAR_H
