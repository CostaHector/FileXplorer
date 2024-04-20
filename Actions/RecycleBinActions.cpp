#include "RecycleBinActions.h"

RecycleBinActions& g_recycleBinAg() {
  static RecycleBinActions ins;
  return ins;
}
