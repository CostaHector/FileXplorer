#include "TorrDb.h"
#include "PublicMacro.h"
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
              .arg(ENUM_2_STR(Name))          //
              .arg(ENUM_2_STR(Size))          //
              .arg(ENUM_2_STR(DateModified))  //
              .arg(ENUM_2_STR(MD5))           //
              .arg(ENUM_2_STR(PrePath))       //
    };

const QString TorrDb::REPLACE_INTO_TABLE_TEMPLATE  //
    {
        "REPLACE INTO `%1` "                             //
        + QString{R"((`%1`, `%2`, `%3`, `%4`, `%5`)
VALUES(:%1, :%2, :%3, :%4, :%5);)"}                 //
              .arg(ENUM_2_STR(Name))          //
              .arg(ENUM_2_STR(Size))          //
              .arg(ENUM_2_STR(DateModified))  //
              .arg(ENUM_2_STR(MD5))           //
              .arg(ENUM_2_STR(PrePath))       //
    };

TorrDb::TorrDb(const QString& dbName, const QString& connName, QObject* parent)  //
    : DbManager{dbName, connName, parent}                                        //
{}
