#ifndef CASTKEY_H
#define CASTKEY_H

#include "KV.h"

namespace CastKey {
using namespace GeneralDataType;
using namespace RawVariant;
using namespace ValueChecker;
constexpr KV SHOW_CASTS_IN_CARD_VIEW{"CastKey/SHOW_CASTS_IN_CARD_VIEW", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
} // namespace SceneKey

#endif // CASTKEY_H
