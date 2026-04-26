#ifndef REDUNIMGFINDERKEY_H
#define REDUNIMGFINDERKEY_H

#include "KV.h"

namespace RedunImgFinderKey {
using namespace GeneralDataType;
using namespace RawVariant;
using namespace ValueChecker;
constexpr KV ALSO_RECYCLE_EMPTY_IMAGE{"RedunImgFinderKey/ALSO_RECYCLE_EMPTY_IMAGE", Var{true}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV RUND_IMG_PATH{"RedunImgFinderKey/RUND_IMG_PATH", Var{"."}, GeneralDataType::Type::FOLDER_PATH, GeneralFolderPathChecker};
constexpr KV SHOW_IMG_PIXMAP{"RedunImgFinderKey/SHOW_IMG_PIXMAP", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
} // namespace RedunImgFinderKey


#endif // REDUNIMGFINDERKEY_H
