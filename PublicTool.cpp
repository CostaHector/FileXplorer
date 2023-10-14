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

#include <QDirIterator>
#include <QDir>

OSWalker_RETURN OSWalker(const QString& pre, const QStringList& rels, const bool includingSub, const bool includingSuffix){
    // Reverse the return value, One can get bottom To Top result like os.walk
    QDirIterator::IteratorFlag dirIterFlag = includingSub? QDirIterator::IteratorFlag::Subdirectories:QDirIterator::IteratorFlag::NoIteratorFlags;
    const int n1 = pre.size() + 1;
    QDir preDir(pre);

    QStringList relToNames;
    QStringList completeNames;
    QStringList suffixs;
    QList<bool> isFiles;

    for (const QString& rel: rels){
        QFileInfo fileInfo(preDir.absoluteFilePath(rel));
        isFiles.append(fileInfo.isFile());
        relToNames.append(fileInfo.absolutePath().mid(n1));
        if (includingSuffix){
            completeNames.append(fileInfo.fileName());
            suffixs.append("");
        }
        else{
            completeNames.append(fileInfo.completeBaseName());
            suffixs.append(fileInfo.suffix());
        }

        if (includingSub and fileInfo.isDir()){  // folders
            QDirIterator it(fileInfo.absoluteFilePath(), {},
                            QDir::Filter::NoDotAndDotDot | QDir::Filter::AllEntries, dirIterFlag);
            while (it.hasNext()){
                it.next();
                auto fi = it.fileInfo();
                isFiles.append(fi.isFile());
                relToNames.append(fi.absolutePath().mid(n1));
                if (includingSuffix){
                    completeNames.append(fi.fileName());
                    suffixs.append("");
                }else{
                    completeNames.append(fi.completeBaseName());
                    suffixs.append(fi.suffix());
                }
            }
        }
    }
    return {relToNames, completeNames, suffixs, isFiles};
}
