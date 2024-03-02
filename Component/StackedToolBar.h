#ifndef STACKEDTOOLBAR_H
#define STACKEDTOOLBAR_H
#include "Component/DatabaseSearchToolBar.h"
#include "Component/NavigationAndAddressBar.h"
#include "PublicVariable.h"

#include <QHash>
#include <QLayout>
#include <QStackedWidget>
#include <QToolBar>

class NavigationViewSwitcher;

class StackedToolBar : public QToolBar {
 public:
  friend class NavigationViewSwitcher;
  explicit StackedToolBar(const QString& title = "Stacked Toolbar", QWidget* parent = nullptr)
      : QToolBar{title, parent}, m_addressBar{nullptr}, m_dbSearchBar{nullptr}, m_stackedToolBar{new QStackedWidget(this)} {
    addWidget(m_stackedToolBar);

    setFixedHeight(CONTROL_TOOLBAR_HEIGHT);
    layout()->setSpacing(0);
    layout()->setContentsMargins(0, 0, 0, 0);
  }
  int AddToolBar(const QString& name, QToolBar* tb);

  NavigationAndAddressBar* m_addressBar;
  DatabaseSearchToolBar* m_dbSearchBar;
private:
  QStackedWidget* m_stackedToolBar;
  QHash<QString, int> m_name2StackIndex;
};

#endif  // STACKEDTOOLBAR_H
