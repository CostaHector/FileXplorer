#include "ViewActions.h"

ViewActions::ViewActions(QObject* parent) : QObject{parent}, PANES_RIBBONS(Get_NAVIGATION_PANE_Actions()) {}

ViewActions& g_viewActions() {
  static ViewActions ins;
  return ins;
}
