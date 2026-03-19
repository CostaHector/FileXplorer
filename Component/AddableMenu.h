#ifndef ADDABLEMENU_H
#define ADDABLEMENU_H

#include <QMenu>
class AddableMenu : public QMenu {
 public:
  using QMenu::QMenu;
  void push_front(const QList<QAction*>& acts);
};

QAction* NewSeperatorAction(QWidget* parent);

#endif  // ADDABLEMENU_H
