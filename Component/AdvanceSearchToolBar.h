#ifndef ADVANCESEARCHTOOLBAR_H
#define ADVANCESEARCHTOOLBAR_H

#include "Component/SearchModeComboBox.h"
#include "Component/FileSystemTypeFilter.h"
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

  void UpdateLineEditReplaceHolder(const QString& searchModeText);

 private:
  void BindSearchProxyModel(SearchProxyModel* searchProxyModel);
  void BindSearchSourceModel(AdvanceSearchModel* searchSourceModel);

  QLineEdit* m_nameFilter = new QLineEdit(this);
  FileSystemTypeFilter* m_typeButton = new FileSystemTypeFilter(this);
  SearchModeComboBox* m_searchModeComboBox = new SearchModeComboBox;

  SearchProxyModel* _searchProxyModel = nullptr;
  AdvanceSearchModel* _searchSourceModel = nullptr;
};

#endif // ADVANCESEARCHTOOLBAR_H
