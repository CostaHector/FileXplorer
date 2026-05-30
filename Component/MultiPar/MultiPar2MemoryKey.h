#ifndef MULTIPAR2MEMORYKEY_H
#define MULTIPAR2MEMORYKEY_H

#include "KV.h"

namespace MultiPar2MemoryKey {
using namespace GeneralDataType;
using namespace RawVariant;
using namespace ValueChecker;
constexpr KV CUSTOM_RATE_OF_REDUNDANCY{"CUSTOM_RATE_OF_REDUNDANCY", Var{10}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 100>}; // [0,100]
constexpr KV LAST_TIME_CREATE_PAR2_TERIGGERED_ACTION{"LAST_TIME_CREATE_PAR2_TERIGGERED_ACTION", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 16>};     // [0,16]
} // namespace JsonOpMemoryKey

#endif // MULTIPAR2MEMORYKEY_H
