#include "LogViewActions.h"


LogViewActions& g_logAg() {
  static LogViewActions instance;
  return instance;
}
