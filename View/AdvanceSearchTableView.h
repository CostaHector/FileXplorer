#ifndef ADVANCESEARCHTABLEVIEW_H
#define ADVANCESEARCHTABLEVIEW_H

#include "Model/AdvanceSearchModel.h"
#include "Component/AdvanceSearchMenu.h"
#include "Component/CustomStatusBar.h"
#include "CustomTableView.h"
#include "Tools/SearchProxyModel.h"

class AdvanceSearchTableView : public CustomTableView {
 public:
  AdvanceSearchTableView(AdvanceSearchModel* sourceModel, SearchProxyModel* searchProxyModel, QWidget* parent = nullptr);

  void subscribe();

  void onCopyNamesStr() const;
  void onCopyFiles();
  void onCutFiles();

  void BindLogger(CustomStatusBar* logger);

  auto keyPressEvent(QKeyEvent* e) -> void override;

 private:
  AdvanceSearchMenu* m_searchMenu;
  AdvanceSearchModel* _sourceModel;
  SearchProxyModel* _searchProxyModel;
  CustomStatusBar* _logger{nullptr};
};

#endif  // ADVANCESEARCHTABLEVIEW_H
