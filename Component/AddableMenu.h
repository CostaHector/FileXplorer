#ifndef ADDABLEMENU_H
#define ADDABLEMENU_H

#include <QMenu>
class AddableMenu : public QMenu {
 public:
  using QMenu::QMenu;

  void push_front(QMenu& rhs);
  void push_front(const QList<QAction*>& acts);
};

#endif  // ADDABLEMENU_H
