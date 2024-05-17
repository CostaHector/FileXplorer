#include "PropertiesWindowActions.h"

PropertiesWindowActions& g_propertiesWindowAct(){
  static PropertiesWindowActions ins;
  return ins;
}
