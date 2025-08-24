#ifndef STACKEDADDRESSANDSEARCHTOOLBAR_H
#define STACKEDADDRESSANDSEARCHTOOLBAR_H
#include "AdvanceSearchToolBar.h"
#include "DatabaseSearchToolBar.h"
#include "NavigationAndAddressBar.h"

#include <QHash>
#include <QLayout>
#include <QStackedWidget>
#include <QToolBar>
#include "ViewTypeTool.h"

class ToolBarAndViewSwitcher;

class StackedAddressAndSearchToolBar : public QToolBar {
 public:
  friend class ToolBarAndViewSwitcher;
  explicit StackedAddressAndSearchToolBar(const QString& title = "Stacked Toolbar", QWidget* parent = nullptr);
  int AddToolBar(ViewTypeTool::ViewType vt, QWidget* tb);

  NavigationAndAddressBar* m_addressBar{nullptr};
  DatabaseSearchToolBar* m_dbSearchBar{nullptr};
  AdvanceSearchToolBar* m_advanceSearchBar{nullptr};
  QLineEdit* m_perfSearch{nullptr};

 private:
  QStackedWidget* m_stackedToolBar{nullptr};
  QMap<ViewTypeTool::ViewType, int> m_name2StackIndex;
};




#endif  // STACKEDADDRESSANDSEARCHTOOLBAR_H
