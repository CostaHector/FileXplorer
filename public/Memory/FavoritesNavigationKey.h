#ifndef FAVORITESNAVIGATIONKEY_H
#define FAVORITESNAVIGATIONKEY_H

#include "KV.h"

namespace FavoritesNavigationKey {
using namespace GeneralDataType;
using namespace RawVariant;
using namespace ValueChecker;
constexpr KV SORT_BY_ROLE{"FavoritesNavigationKey/SORT_BY_ROLE", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 16>};
constexpr KV SORT_ORDER_REVERSE{"FavoritesNavigationKey/SORT_ORDER_REVERSE", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
} // namespace FavoritesNavigationKey

#endif // FAVORITESNAVIGATIONKEY_H
