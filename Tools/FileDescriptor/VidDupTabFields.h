#ifndef VIDDUPTABFIELDS_H
#define VIDDUPTABFIELDS_H

#include "PublicMacro.h"
#include <QString>

#define VID_DUP_TABLE_FIELD_MAPPING    \
    VID_DUP_TABLE_KEY_ITEM(EFFECTIVE_NAME, 0)      \
    VID_DUP_TABLE_KEY_ITEM(SIZE, 1)      \
    VID_DUP_TABLE_KEY_ITEM(DURATION, 2)       \
    VID_DUP_TABLE_KEY_ITEM(DATE, 3)      \
    VID_DUP_TABLE_KEY_ITEM(ABSOLUTE_PATH, 4)       \
    VID_DUP_TABLE_KEY_ITEM(FIRST_1024_HASH, 5)      \
    VID_DUP_TABLE_KEY_ITEM(FIRST_8192_HASH, 6)      \
    VID_DUP_TABLE_KEY_ITEM(FULL_SIZE_HASH, 7)    \

namespace VidDupHelper {
enum FIELD_E {
#define VID_DUP_TABLE_KEY_ITEM(enu, val) enu = val,
    VID_DUP_TABLE_FIELD_MAPPING
#undef VID_DUP_TABLE_KEY_ITEM
};

inline QString GetAiDupVidDbPath() {
#ifdef RUNNING_UNIT_TESTS
    return TESTCASE_ROOT_PATH "/test/DUPLICATES_DB_TEST.db";
#else
    return SystemPath::AI_MEDIA_DUP_DATABASE;
#endif
}
constexpr char VID_DUP_CONNECTION_NAME[]{"AI_MEDIA_DUP_CONNECT"};
}


#endif // VIDDUPTABFIELDS_H
