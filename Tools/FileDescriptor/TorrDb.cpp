#include "TorrDb.h"
#include "public/PublicMacro.h"
#include "TableFields.h"

using namespace TORRENTS_DB_HEADER_KEY;
const QString TorrDb::CREATE_TABLE_TEMPLATE  //
    {
        "CREATE TABLE IF NOT EXISTS `%1`( "  //
        + QString{"   `%1` NCHAR(256) NOT NULL,"
                  "   `%2` INT DEFAULT 0,"
                  "   `%3` INT,"
                  "   `%4` CHAR(64) default '',"
                  "   `%5` NCHAR(256) NOT NULL,"
                  "    PRIMARY KEY (%1, %5)"
                  "    );"}
              .arg(ENUM_TO_STRING(Name))          //
              .arg(ENUM_TO_STRING(Size))          //
              .arg(ENUM_TO_STRING(DateModified))  //
              .arg(ENUM_TO_STRING(MD5))           //
              .arg(ENUM_TO_STRING(PrePath))       //
    };

const QString TorrDb::REPLACE_INTO_TABLE_TEMPLATE  //
    {
        "REPLACE INTO `%1` "                             //
        + QString{R"((`%1`, `%2`, `%3`, `%4`, `%5`)
VALUES(:%1, :%2, :%3, :%4, :%5);)"}                 //
              .arg(ENUM_TO_STRING(Name))          //
              .arg(ENUM_TO_STRING(Size))          //
              .arg(ENUM_TO_STRING(DateModified))  //
              .arg(ENUM_TO_STRING(MD5))           //
              .arg(ENUM_TO_STRING(PrePath))       //
    };

TorrDb::TorrDb(const QString& dbName, const QString& connName, QObject* parent)  //
    : DbManager{dbName, connName, parent}                                        //
{}
