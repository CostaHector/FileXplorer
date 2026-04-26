#ifndef SEARCHKEY_H
#define SEARCHKEY_H

#include "KV.h"

namespace SearchKey {
using namespace GeneralDataType;
using namespace RawVariant;
using namespace ValueChecker;
constexpr KV INCLUDING_SUBDIRECTORIES{"SearchKey/INCLUDING_SUBDIRECTORIES", Var{true}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV NAME_CASE_SENSITIVE{"SearchKey/NAME_CASE_SENSITIVE", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV CONTENTS_CASE_SENSITIVE{"SearchKey/CONTENTS_CASE_SENSITIVE", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV GRAY_ENTRIES_DONT_PASS_FILTER{"SearchKey/GRAY_ENTRIES_DONT_PASS_FILTER", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV ADVANCE_LINEEDIT_VALUE{"SearchKey/ADVANCE_LINEEDIT_VALUE", Var{""}, GeneralDataType::Type::PLAIN_STR, GeneralCStrChecker};
constexpr KV ADVANCE_CONTENTS_LINEEDIT_VALUE{"SearchKey/ADVANCE_CONTENTS_LINEEDIT_VALUE", Var{""}, GeneralDataType::Type::PLAIN_STR, GeneralCStrChecker};
constexpr KV ADVANCE_MODE{"SearchKey/ADVANCE_MODE", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 16>};
} // namespace SearchKey

#endif // SEARCHKEY_H
