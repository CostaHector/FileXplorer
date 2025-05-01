#ifndef NAVIGATIONTOOLBAR_H
#define NAVIGATIONTOOLBAR_H

#include "NavigationExToolBar.h"

class NavigationToolBar : public QToolBar {
 public:
  explicit NavigationToolBar(const QString& title = "NavigationToolBar", bool isShow_ = true);
 private:
  NavigationExToolBar* m_extraAppendTB{nullptr};
};

#endif  // NAVIGATIONTOOLBAR_H
