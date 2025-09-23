#include "VidDupTabFields.h"
#include "PublicVariable.h"

namespace VidDupHelper {

QString GetAiDupVidDbPath() {
#ifdef RUNNING_UNIT_TESTS
    return TESTCASE_ROOT_PATH "/test/DUPLICATES_DB_TEST.db";
#else
    return SystemPath::AI_MEDIA_DUP_DATABASE();
#endif
}

}
