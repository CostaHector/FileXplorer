#ifndef ROWHEIGHTREGISTRY_H
#define ROWHEIGHTREGISTRY_H
#include <unordered_set>

// SetFontableObject can be CustomTableView, CustomTreeView. (at least setRowHeight(), rowHeight() needed)

template <typename SetRowHeightableObject>
struct RowHeightRegistry {
  using TWidgetSet = std::unordered_set<SetRowHeightableObject*>;
  static TWidgetSet* GetWidgetsSet();

  static bool registerWidgetForAdjust(SetRowHeightableObject* pWid, bool isAlwaysAlive = true);
  static bool unregisterWidgetForAdjust(SetRowHeightableObject* pWid);
  static int updateRegisteredWidgetsForAdjust(int newRowHeight);
};

#endif  // ROWHEIGHTREGISTRY_H
