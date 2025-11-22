#ifndef ADDABLEMENU_H
#define ADDABLEMENU_H

#include <QMenu>
class AddableMenu : public QMenu {
 public:
  using QMenu::QMenu;

  AddableMenu& operator+=(QMenu& rhs);
};

#endif  // ADDABLEMENU_H
