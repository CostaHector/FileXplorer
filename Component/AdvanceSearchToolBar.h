#ifndef ADVANCESEARCHTOOLBAR_H
#define ADVANCESEARCHTOOLBAR_H



#include "Component/SearchModeComboBox.h"
#include "Component/ToolButtonFileSystemTypeFilter.h"
#include "MySearchModel.h"
#include "Tools/SearchProxyModel.h"

#include <QLineEdit>
#include <QMainWindow>
#include <QToolBar>

#include <QLayout>

class AdvanceSearchToolBar : public QToolBar {
  Q_OBJECT
 public:
  explicit AdvanceSearchToolBar(const QString& title = tr("Advance Search Toolbar"), QWidget* parent = nullptr);

  void BindProxyModel(SearchProxyModel* proxyModel);

  void BindSourceModel(MySearchModel* model);

 private:
  QLineEdit* m_nameFilter = new QLineEdit(this);
  ToolButtonFileSystemTypeFilter* m_typeButton = new ToolButtonFileSystemTypeFilter(this);
  SearchModeComboBox* m_searchModeComboBox = new SearchModeComboBox;

  SearchProxyModel* _proxyModel = nullptr;
  MySearchModel* _model = nullptr;
};

#endif // ADVANCESEARCHTOOLBAR_H
