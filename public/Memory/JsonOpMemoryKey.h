#ifndef JSONOPMEMORYKEY_H
#define JSONOPMEMORYKEY_H

#include "KV.h"

namespace JsonOpMemoryKey {
using namespace GeneralDataType;
using namespace RawVariant;
using namespace ValueChecker;
constexpr KV STUDIO_OPERATION{"STUDIO_OPERATION", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 100>}; // [0,100]
constexpr KV CAST_OPERATION{"CAST_OPERATION", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 100>};     // [0,100]
constexpr KV TAGS_OPERATION{"TAGS_OPERATION", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 100>};     // [0,100]
} // namespace JsonOpMemoryKey

#endif // JSONOPMEMORYKEY_H
