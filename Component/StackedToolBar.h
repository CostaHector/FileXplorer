#ifndef STACKEDTOOLBAR_H
#define STACKEDTOOLBAR_H
#include "AdvanceSearchToolBar.h"
#include "DatabaseSearchToolBar.h"
#include "NavigationAndAddressBar.h"

#include <QHash>
#include <QLayout>
#include <QStackedWidget>
#include <QToolBar>
#include "ViewTypeTool.h"

class NavigationViewSwitcher;

class StackedToolBar : public QToolBar {
 public:
  friend class NavigationViewSwitcher;
  explicit StackedToolBar(const QString& title = "Stacked Toolbar", QWidget* parent = nullptr);
  int AddToolBar(ViewTypeTool::ViewType vt, QWidget* tb);

  NavigationAndAddressBar* m_addressBar{nullptr};
  DatabaseSearchToolBar* m_dbSearchBar{nullptr};
  AdvanceSearchToolBar* m_advanceSearchBar{nullptr};
  QLineEdit* m_perfSearch{nullptr};

 private:
  QStackedWidget* m_stackedToolBar;
  QMap<ViewTypeTool::ViewType, int> m_name2StackIndex;
};




#endif  // STACKEDTOOLBAR_H
