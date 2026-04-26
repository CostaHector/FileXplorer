#ifndef BEHAVIORKEY_H
#define BEHAVIORKEY_H

#include "KV.h"

namespace BehaviorKey {
using namespace GeneralDataType;
using namespace RawVariant;
using namespace ValueChecker;
constexpr KV NEW_CHOICE{"BehaviorKey/NEW_CHOICE", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 16>};
constexpr KV COPY_CHOICE{"BehaviorKey/COPY_CHOICE", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 16>};
constexpr KV RENAME_CHOICE{"BehaviorKey/RENAME_CHOICE", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 16>};
constexpr KV THUMBNAILS_DIMENSION{"BehaviorKey/THUMBNAILS_DIMENSION", Var{"Create 2x2 thumbnail"}, GeneralDataType::Type::PLAIN_STR, GeneralCStrChecker};
constexpr KV EXTRACT_CHOICE{"BehaviorKey/EXTRACT_CHOICE", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 16>};
constexpr KV MOVE_TO_HISTORY{"BehaviorKey/MOVE_TO_HISTORY", Var{".\n..\n\\"}, GeneralDataType::Type::MULTI_LINE_STR, GeneralCStrChecker};
constexpr KV COPY_TO_HISTORY{"BehaviorKey/COPY_TO_HISTORY", Var{".\n..\n\\"}, GeneralDataType::Type::MULTI_LINE_STR, GeneralCStrChecker};
constexpr KV FILESYSTEM_STRUCTURE{"BehaviorKey/FILESYSTEM_STRUCTURE", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 16>};

constexpr KV VIDS_LAST_TABLE_NAME{"BehaviorKey/VIDS_LAST_TABLE_NAME", Var{""}, GeneralDataType::Type::PLAIN_STR, GeneralCStrChecker};
constexpr KV WHERE_CLAUSE_HISTORY{"BehaviorKey/WHERE_CLAUSE_HISTORY", Var{"A\nA&B\nA|B"}, GeneralDataType::Type::MULTI_LINE_STR, GeneralCStrChecker};

constexpr KV BATCH_FILES_NAME_PATTERN{"BehaviorKey/BATCH_FILES_NAME_PATTERN", Var{"Page %03d%1$1$11.html"}, GeneralDataType::Type::PLAIN_STR, GeneralCStrChecker};
constexpr KV BATCH_FOLDERS_NAME_PATTERN{"BehaviorKey/BATCH_FOLDERS_NAME_PATTERN", Var{"Page %03d%1$1$11"}, GeneralDataType::Type::PLAIN_STR, GeneralCStrChecker};

constexpr KV DIR_FILTER_ON_SWITCH_ENABLE{"BehaviorKey/DIR_FILTER_ON_SWITCH_ENABLE", Var{-1}, GeneralDataType::Type::PLAIN_INT, GeneralIntChecker};

constexpr KV ALL_LOG_LEVEL_AUTO_FFLUSH{"BehaviorKey/ALL_LOG_LEVEL_AUTO_FFLUSH", Var{(int) QtMsgType::QtWarningMsg}, GeneralDataType::Type::PLAIN_INT, GeneralIntRangeChecker<0, 16>};
constexpr KV RETURN_ERRORCODE_UPON_ANY_FAILURE{"BehaviorKey/RETURN_ERRORCODE_UPON_ANY_FAILURE", Var{true}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
} // namespace BehaviorKey

#endif // BEHAVIORKEY_H
