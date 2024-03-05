#ifndef ADVANCESEARCHWINDOW_H
#define ADVANCESEARCHWINDOW_H

#include "Component/SearchModeComboBox.h"
#include "Component/ToolButtonFileSystemTypeFilter.h"
#include "CustomTableView.h"
#include "MySearchModel.h"
#include "Tools/SearchProxyModel.h"

#include <QDebug>
#include <QLineEdit>
#include <QMainWindow>
#include <QToolBar>


class AdvanceSearchTableView : public CustomTableView{
 public:
  AdvanceSearchTableView(MySearchModel* _sourceModel, SearchProxyModel* _searchProxyModel, QWidget* parent = nullptr);

 private:
};

#endif  // ADVANCESEARCHWINDOW_H
