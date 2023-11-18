#include "DataBaseActions.h"


DataBaseActions& g_dbAct(){
    static DataBaseActions ins;
    return ins;
}
