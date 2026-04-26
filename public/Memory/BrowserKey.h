#ifndef BROWSERKEY_H
#define BROWSERKEY_H

#include "KV.h"

namespace BrowserKey {
using namespace GeneralDataType;
using namespace RawVariant;
using namespace ValueChecker;
constexpr KV CAST_PREVIEW_BROWSER_SHOW_RELATED_IMAGES{"BrowserKey/CAST_PREVIEW_BROWSER_SHOW_RELATED_IMAGES", Var{true}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV CAST_PREVIEW_BROWSER_SHOW_RELATED_VIDEOS{"BrowserKey/CAST_PREVIEW_BROWSER_SHOW_RELATED_VIDEOS", Var{true}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV FLOATING_IMAGE_VIEW_SHOW{"BrowserKey/FLOATING_IMAGE_VIEW_SHOW", Var{true}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV FLOATING_VIDEO_VIEW_SHOW{"BrowserKey/FLOATING_VIDEO_VIEW_SHOW", Var{true}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV FLOATING_OTHER_VIEW_SHOW{"BrowserKey/FLOATING_OTHER_VIEW_SHOW", Var{true}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV FLOATING_MEDIA_TYPE_SEQ{"BrowserKey/FLOATING_MEDIA_TYPE_SEQ", Var{"012"}, GeneralDataType::Type::PLAIN_STR, GeneralSequenceChecker};
} // namespace BrowserKey

#endif // BROWSERKEY_H
