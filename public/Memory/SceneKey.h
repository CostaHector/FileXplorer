#ifndef SCENEKEY_H
#define SCENEKEY_H

#include "KV.h"

namespace SceneKey {
using namespace GeneralDataType;
using namespace RawVariant;
using namespace ValueChecker;
constexpr KV SORT_ORDER_REVERSE{"SceneKey/SORT_ORDER_REVERSE", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV SORT_BY_ROLE{"SceneKey/SORT_BY_ROLE", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 16>};
constexpr KV DISABLE_IMAGE_DECORATION{"SceneKey/DISABLE_IMAGE_DECORATION", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV CNT_EACH_PAGE{"SceneKey/CNT_EACH_PAGE", Var{40}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, INT_MAX>};
} // namespace SceneKey

#endif // SCENEKEY_H
