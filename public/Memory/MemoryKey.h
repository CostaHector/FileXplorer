#ifndef MEMORYKEY_H
#define MEMORYKEY_H

#include "KV.h"
#include "SizeTool.h"

namespace MemoryKey {
using namespace GeneralDataType;
using namespace RawVariant;
using namespace ValueChecker;
constexpr KV LANGUAGE_ZH_CN{"LANGUAGE_ZH_CN", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV ROW_HEIGHT{"ROW_HEIGHT", Var{SizeTool::TABLE_DEFAULT_ROW_SECTION_SIZE}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, INT_MAX>};
} // namespace MemoryKey

#endif // MEMORYKEY_H
