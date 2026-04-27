#ifndef PATHKEY_H
#define PATHKEY_H

#include "KV.h"

namespace PathKey {
using namespace GeneralDataType;
using namespace RawVariant;
using namespace ValueChecker;
constexpr KV STARTUP_PATH{"PathKey/STARTUP_PATH", Var{"./"}, GeneralDataType::Type::FOLDER_PATH, GeneralFolderPathChecker};
constexpr KV LAST_TIME_COPY_TO{"PathKey/LAST_TIME_COPY_TO", Var{""}, GeneralDataType::Type::FOLDER_PATH, GeneralFolderPathChecker};
constexpr KV VIDEO_PLAYER_OPEN_PATH{"PathKey/VIDEO_PLAYER_OPEN_PATH", Var{"./"}, GeneralDataType::Type::FOLDER_PATH, GeneralFolderPathChecker};
constexpr KV PERFORMER_IMAGEHOST_LOCATE{"PathKey/PERFORMER_IMAGEHOST_LOCATE", Var{"./"}, GeneralDataType::Type::FOLDER_PATH, GeneralFolderPathChecker};
constexpr KV DB_INSERT_VIDS_FROM{"PathKey/DB_INSERT_VIDS_FROM", Var{"./"}, GeneralDataType::Type::FOLDER_PATH, GeneralFolderPathChecker};
constexpr KV DB_INSERT_TORRENTS_FROM{"PathKey/DB_INSERT_TORRENTS_FROM", Var{"./"}, GeneralDataType::Type::FOLDER_PATH, GeneralFolderPathChecker};
} // namespace PathKey


#endif // PATHKEY_H
