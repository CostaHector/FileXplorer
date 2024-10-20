#ifndef STACKEDTOOLBAR_H
#define STACKEDTOOLBAR_H
#include "Component/AdvanceSearchToolBar.h"
#include "Component/DatabaseSearchToolBar.h"
#include "Component/NavigationAndAddressBar.h"
#include "PublicVariable.h"

#include <QHash>
#include <QLayout>
#include <QStackedWidget>
#include <QToolBar>
#include "Tools/ViewTypeTool.h"

class NavigationViewSwitcher;

class StackedToolBar : public QToolBar {
 public:
  friend class NavigationViewSwitcher;
  explicit StackedToolBar(const QString& title = "Stacked Toolbar", QWidget* parent = nullptr)
      : QToolBar{title, parent}, m_stackedToolBar{new QStackedWidget(this)} {
    addWidget(m_stackedToolBar);

    setFixedHeight(CONTROL_TOOLBAR_HEIGHT);
    layout()->setSpacing(0);
    layout()->setContentsMargins(0, 0, 0, 0);
  }
  int AddToolBar(ViewTypeTool::ViewType vt, QToolBar* tb);

  NavigationAndAddressBar* m_addressBar{nullptr};
  DatabaseSearchToolBar* m_dbSearchBar{nullptr};
  AdvanceSearchToolBar* m_advanceSearchBar{nullptr};

 private:
  QStackedWidget* m_stackedToolBar;
  QMap<ViewTypeTool::ViewType, int> m_name2StackIndex;
};




#endif  // STACKEDTOOLBAR_H
