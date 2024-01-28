#include "ViewActions.h"

ViewActions& g_viewActions() {
  static ViewActions ins;
  return ins;
}
