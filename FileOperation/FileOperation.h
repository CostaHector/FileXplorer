#ifndef FILEOPERATION_H
#define FILEOPERATION_H

#include <QStringList>

namespace ErrorCode {
constexpr int OK = 0;
constexpr int SRC_PRE_DIR_INEXIST = 1;
constexpr int SRC_FILE_INEXIST = 2;
constexpr int SRC_DIR_INEXIST = 3;
constexpr int SRC_INEXIST = 4;
constexpr int DST_DIR_INEXIST = 5;
constexpr int DST_PRE_DIR_CANNOT_MAKE = 6;
constexpr int DST_FOLDER_ALREADY_EXIST = 7;
constexpr int DST_FILE_ALREADY_EXIST = 8;
constexpr int DST_FILE_OR_PATH_ALREADY_EXIST = 9;
constexpr int CANNOT_REMOVE_FILE = 10;
constexpr int CANNOT_REMOVE_DIR = 11;
constexpr int CANNOT_MAKE_LINK = 12;
constexpr int DST_LINK_INEXIST = 13;
constexpr int CANNOT_REMOVE_LINK = 14;
constexpr int UNKNOWN_ERROR = -1;
};

#include <QPair>

class FileOperation {
public:
    using BATCH_COMMAND_LIST_TYPE = QList<QStringList>;
    using RETURN_TYPE = QPair<int, BATCH_COMMAND_LIST_TYPE>;
    static QPair<QString, QString> SplitDirName(const QString& fullPath){
        auto ind = fullPath.lastIndexOf('/');
        if (ind == -1){
            return {fullPath, ""};
        }
        if (fullPath[ind - 1] == ':'){
            return {fullPath.left(ind+1), fullPath.mid(ind+1)};
        }
        return {fullPath.left(ind), fullPath.mid(ind+1)};
    }

};

#endif // FILEOPERATION_H
