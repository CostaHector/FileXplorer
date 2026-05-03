#ifndef COMPOVISKEY_H
#define COMPOVISKEY_H

#include "KV.h"

namespace CompoVisKey {
using namespace GeneralDataType;
using namespace RawVariant;
using namespace ValueChecker;
constexpr KV MENU_RIBBON_CURRENT_TAB_INDEX{"CompoVisKey/MENU_RIBBON_CURRENT_TAB_INDEX", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 16>};
constexpr KV FOLDER_PREVIEW_TYPE{"CompoVisKey/FOLDER_PREVIEW_TYPE", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 16>};
constexpr KV SHOW_PREVIEW_DOCKER{"CompoVisKey/SHOW_PREVIEW_DOCKER", Var{true}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV SHOW_NAVIGATION_SIDEBAR{"CompoVisKey/SHOW_NAVIGATION_SIDEBAR", Var{true}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV EXPAND_NAVIGATION_SIDEBAR{"CompoVisKey/EXPAND_NAVIGATION_SIDEBAR", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV EXPAND_OFFICE_STYLE_MENUBAR{"CompoVisKey/EXPAND_OFFICE_STYLE_MENUBAR", Var{true}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
} // namespace CompoVisKey

#endif // COMPOVISKEY_H
