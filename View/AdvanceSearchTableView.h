#ifndef ADVANCESEARCHTABLEVIEW_H
#define ADVANCESEARCHTABLEVIEW_H

#include "AdvanceSearchModel.h"
#include "CustomStatusBar.h"
#include "CustomTableView.h"
#include "SearchProxyModel.h"

class AdvanceSearchTableView : public CustomTableView {
 public:
  AdvanceSearchTableView(AdvanceSearchModel* sourceModel, SearchProxyModel* searchProxyModel, QWidget* parent = nullptr);

  void subscribe();

  void onCopyNamesStr() const;
  void onCopyFiles();
  void onCutFiles();

  void keyPressEvent(QKeyEvent* e) override;

 private:
  AdvanceSearchModel* _sourceModel;
  SearchProxyModel* _searchProxyModel;
};

#endif  // ADVANCESEARCHTABLEVIEW_H
