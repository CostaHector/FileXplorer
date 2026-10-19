#ifndef IMAGEVIEWERKEY_H
#define IMAGEVIEWERKEY_H

#include "KV.h"

namespace ImageViewerKey {
using namespace GeneralDataType;
using namespace RawVariant;
using namespace ValueChecker;
constexpr KV NAVIGATE_INTO_SUBFOLDERS_WHEN_ITERATE{"ImageViewerKey/NAVIGATE_INTO_SUBFOLDERS_WHEN_ITERATE", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};

constexpr KV RECOGNIZE_FACE_ON_SELECTION_COMPLETE{"ImageViewerKey/RECOGNIZE_FACE_ON_SELECTION_COMPLETE", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV AUTO_SAVE_ON_SELECTION_COMPLETE{"ImageViewerKey/POPUP_SAVE_DIALOG_ON_SELECTION_COMPLETE", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV SILENT_SAVE_ON_AUTO_SAVE{"ImageViewerKey/SAVE_DIRECTLY_ON_SELECTION_COMPLETE", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
} // namespace PathKey


#endif // IMAGEVIEWERKEY_H
