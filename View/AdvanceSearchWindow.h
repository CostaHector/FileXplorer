#ifndef ADVANCESEARCHWINDOW_H
#define ADVANCESEARCHWINDOW_H

#include "CustomTableView.h"
#include "AdvanceSearchModel.h"
#include "Tools/SearchProxyModel.h"

#include <QDebug>
#include <QLineEdit>
#include <QMainWindow>
#include <QToolBar>


class AdvanceSearchTableView : public CustomTableView{
 public:
  AdvanceSearchTableView(AdvanceSearchModel* _sourceModel, SearchProxyModel* _searchProxyModel, QWidget* parent = nullptr);

 private:
};

#endif  // ADVANCESEARCHWINDOW_H
