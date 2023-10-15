#include "FramelessWindowActions.h"

FramelessWindowActions::FramelessWindowActions(QObject *parent)
    : QObject{parent},
    _EXPAND_RIBBONS(nullptr),
    FRAMELESS_MENU_RIBBONS(Get_FRAMELESS_MENU_Actions())
{

}

FramelessWindowActions& g_framelessWindowAg(){
    static FramelessWindowActions framelessWindowsActIns;
    return framelessWindowsActIns;
}
