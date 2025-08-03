#include "PublicVariable.h"
#include <QDir>

const QString SystemPath::HOME_PATH{QDir::homePath()};
const QString SystemPath::WORK_PATH{HOME_PATH + "/" + PROJECT_NAME};
const QString SystemPath::STARRED_PATH{HOME_PATH + "/Documents"};
const QString SystemPath::VIDS_DATABASE{WORK_PATH + "/VIDS_DATABASE.db"};
const QString SystemPath::DEVICES_AND_DRIVES_DATABASE{WORK_PATH + "/DEVICES_AND_DRIVES.db"};
const QString SystemPath::AI_MEDIA_DUP_DATABASE{WORK_PATH + "/DUPLICATES_DB.db"};
const QString SystemPath::RECYCLE_BIN_DATABASE{WORK_PATH + "/RECYCLE_BIN_DATABASE.db"};
const QString SystemPath::PEFORMERS_DATABASE{WORK_PATH + "/PERFORMERS_DATABASE.db"};
const QString SystemPath::TORRENTS_DATABASE{WORK_PATH + "/TORRENTS_DATABASE.db"};
const QString SystemPath::PRODUCTION_STUDIOS_DATABASE{WORK_PATH + "/PRODUCTION_STUDIOS_DATABASE.db"};
