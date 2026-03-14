#include "AddableMenu.h"

void AddableMenu::push_front(QMenu& rhs) {
  if (this == &rhs) {
    return;
  }
  const QList<QAction*> curActs{actions()};
  QAction* firstAct{curActs.isEmpty() ? nullptr : curActs.front()};
  QList<QAction*> oldActs{rhs.actions()};
  insertActions(firstAct, oldActs);
  for (QAction* needRmvAction : oldActs) {
    rhs.removeAction(needRmvAction);
  }
}

void AddableMenu::push_front(const QList<QAction*>& acts) {
  if (acts.isEmpty()) {
    return;
  }
  const QList<QAction*> curActs{actions()};
  QAction* firstAct{curActs.isEmpty() ? nullptr : curActs.front()};
  insertActions(firstAct, acts);
}
