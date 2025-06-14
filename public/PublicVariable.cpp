#include "PublicVariable.h"
#include <QDir>

const QString SystemPath::HOME_PATH{QDir::homePath()};
const QString SystemPath::STARRED_PATH{HOME_PATH + "/Documents"};
const QString SystemPath::VIDS_DATABASE{HOME_PATH + "/FileExplorerReadOnly/VIDS_DATABASE.db"};
const QString SystemPath::DEVICES_AND_DRIVES_DATABASE{HOME_PATH + "/FileExplorerReadOnly/DEVICES_AND_DRIVES.db"};
const QString SystemPath::AI_MEDIA_DUP_DATABASE{HOME_PATH + "/FileExplorerReadOnly/DUPLICATES_DB.db"};
const QString SystemPath::RECYCLE_BIN_DATABASE{HOME_PATH + "/FileExplorerReadOnly/RECYCLE_BIN_DATABASE.db"};
const QString SystemPath::PEFORMERS_DATABASE{HOME_PATH + "/FileExplorerReadOnly/PERFORMERS_DATABASE.db"};
const QString SystemPath::TORRENTS_DATABASE{HOME_PATH + "/FileExplorerReadOnly/TORRENTS_DATABASE.db"};
const QString SystemPath::PRODUCTION_STUDIOS_DATABASE{HOME_PATH + "/FileExplorerReadOnly/PRODUCTION_STUDIOS_DATABASE.db"};
