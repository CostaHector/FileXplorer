#include "AddableMenu.h"

void AddableMenu::push_front(const QList<QAction*>& acts) {
  if (acts.isEmpty()) {
    return;
  }
  const QList<QAction*> curActs{actions()};
  QAction* firstAct{curActs.isEmpty() ? nullptr : curActs.front()};
  insertActions(firstAct, acts);
}
