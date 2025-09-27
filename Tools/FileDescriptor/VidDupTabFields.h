#ifndef VIDDUPTABFIELDS_H
#define VIDDUPTABFIELDS_H

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
#ifdef RUNNING_UNIT_TESTS
QString& DupVidDbAbsFilePathInTestCase(const QString& aNewPrePath="");
QString& DupVidDbConnectionNameInTestCase(const QString& aNewConnectionName="");
#endif
QString GetAiDupVidDbPath();
QString GetAiDupVidDbConnectionName();
}

#endif // VIDDUPTABFIELDS_H
