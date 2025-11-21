#include "AddableMenu.h"

AddableMenu& AddableMenu::operator+=(QMenu& rhs) {
  if (this == &rhs) {
    return *this;
  }
  for (QAction* action : rhs.actions()) {
    rhs.removeAction(action);
    addAction(action);
  }
  return *this;
}
