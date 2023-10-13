#include "PublicTool.h"
#include <QDir>

PublicTool::PublicTool()
{

}

bool PublicTool::copyDirectoryFiles(const QString &fromDir, const QString &toDir, bool coverFileIfExist)
{
    QDir sourceDir(fromDir);
    QDir targetDir(toDir);
    if(!targetDir.exists()){    /* if directory don't exists, build it */
        if(!targetDir.mkdir(targetDir.absolutePath()))
            return false;
    }

    QFileInfoList fileInfoList = sourceDir.entryInfoList();
    for(const QFileInfo& fileInfo:fileInfoList){
        if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        if(fileInfo.isDir()){    /* if it is directory, copy recursively*/
            if(copyDirectoryFiles(fileInfo.filePath(), targetDir.filePath(fileInfo.fileName()), coverFileIfExist)){
                continue;
            }
            return false;
        }
        /* if coverFileIfExist == true, remove old file first */

        if (targetDir.exists(fileInfo.fileName())){
            if(coverFileIfExist){
                targetDir.remove(fileInfo.fileName());
                qDebug("%s/%s is covered by file under [%s]", targetDir.absolutePath().toStdString().c_str(), fileInfo.fileName().toStdString().c_str(), fromDir.toStdString().c_str());
            }else{
                qDebug("%s/[%s] was kept", targetDir.absolutePath().toStdString().c_str(), fileInfo.fileName().toStdString().c_str());
            }
        }
        // files copy
        if(!QFile::copy(fileInfo.filePath(), targetDir.filePath(fileInfo.fileName()))){
            return false;
        }
    }
    return true;

}
