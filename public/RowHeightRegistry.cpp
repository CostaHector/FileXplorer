#include "RowHeightRegistry.h"

#include <QObject>

template <typename SetRowHeightableObject>
typename RowHeightRegistry<SetRowHeightableObject>::TWidgetSet* RowHeightRegistry<SetRowHeightableObject>::GetWidgetsSet() {
  static TWidgetSet multiRowWidget;
  return &multiRowWidget;
}

template <typename SetRowHeightableObject>
bool RowHeightRegistry<SetRowHeightableObject>::registerWidgetForAdjust(SetRowHeightableObject* pWid, bool isAlwaysAlive) {
  if (pWid == nullptr) {
    return false;
  }
  TWidgetSet* pSet{GetWidgetsSet()};
  const auto& pr = pSet->insert(pWid);
  bool bRegisterResult = pr.second;
  if (bRegisterResult && !isAlwaysAlive) {
    QObject::connect(pWid, &QObject::destroyed, [pSet, pWid]() {
      auto it = pSet->find(pWid);
      if (it != pSet->end()) {
        pSet->erase(it);
      }
    });
  }
  return bRegisterResult;
}

template <typename SetRowHeightableObject>
bool RowHeightRegistry<SetRowHeightableObject>::unregisterWidgetForAdjust(SetRowHeightableObject* pWid) {
  if (pWid == nullptr) {
    return false;
  }
  TWidgetSet* pSet{GetWidgetsSet()};
  auto it = pSet->find(pWid);
  if (it == pSet->end()) {
    return false;
  }
  pSet->erase(it);
  return true;
}

template <typename SetRowHeightableObject>
int RowHeightRegistry<SetRowHeightableObject>::updateRegisteredWidgetsForAdjust(int newRowHeight) {
  int changedCnt = 0;
  TWidgetSet* pSet{GetWidgetsSet()};
  for (auto* widget : *pSet) {
    if (widget == nullptr || widget->rowHeight() == newRowHeight) {
      continue;
    }
    widget->setRowHeight(newRowHeight);
    ++changedCnt;
  }
  return changedCnt;
}

#include "CustomTableView.h"
#include "CustomTreeView.h"
template struct RowHeightRegistry<CustomTableView>;
template struct RowHeightRegistry<CustomTreeView>;
