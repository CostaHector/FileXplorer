#include "PublicVariable.h"
#include "PublicMacro.h"

constexpr int IMAGE_SIZE::TABS_ICON_IN_MENU_16;
constexpr int IMAGE_SIZE::TABS_ICON_IN_MENU_24;
constexpr int IMAGE_SIZE::TABS_ICON_IN_MENU_48;
int IMAGE_SIZE::IMG_WIDTH = 420;
int IMAGE_SIZE::IMG_HEIGHT = 320;

constexpr int CCMMode::MCCL2STR_MAX_LEN;
const char CCMMode::MCCL2STR[BUTT][MCCL2STR_MAX_LEN]  //
    {
        ENUM_2_STR(MERGE_OP),  //
        ENUM_2_STR(COPY_OP),   //
        ENUM_2_STR(CUT_OP),    //
        ENUM_2_STR(LINK_OP),   //
        ENUM_2_STR(ERROR_OP)   //
    };
