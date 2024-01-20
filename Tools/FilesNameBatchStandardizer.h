#ifndef FILESNAMEBATCHSTANDARDIZER_H
#define FILESNAMEBATCHSTANDARDIZER_H

#include <QSet>
#include <QString>
#include <QDirIterator>
#include "Tools/NameStandardizer.h"
#include "FileOperation/FileOperation.h"
#include "UndoRedo.h"

class FilesNameBatchStandardizer
{
public:
    FilesNameBatchStandardizer() = default;
    bool operator()(const QString& rootPath){
        QStringList pathList;
        QStringList nmList;
        QStringList newNmList;
        QDirIterator it(rootPath,
                        QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot,
                        QDirIterator::IteratorFlag::Subdirectories);
        QSet<QString> uniqueAbsPathSet;
        while (it.hasNext()) {
            it.next();
            const QString& oldName = it.fileName();
            nmList.append(oldName);
            newNmList.append(ns(oldName));
            pathList.append(it.filePath().chopped(oldName.size()+1));
            const QString& absPath = it.filePath();
            if (uniqueAbsPathSet.contains(absPath)){
                qDebug("%s will duplicate", qPrintable(oldName));
                continue;
            }
            uniqueAbsPathSet.insert(it.filePath());
        }
        if (uniqueAbsPathSet.size() < newNmList.size()){
            qDebug("%d/%d nms is duplicate. solve now first",
                   newNmList.size()-uniqueAbsPathSet.size(), newNmList.size());
            return false;
        }
        FileOperation::BATCH_COMMAND_LIST_TYPE cmds;
        for (auto i = nmList.size() - 1 ; i > -1 ; --i){
            const QString& oldNm = nmList[i];
            const QString& newNm = newNmList[i];
            if (oldNm == newNm){
                continue;
            }
            cmds.append({"rename", pathList[i], oldNm, pathList[i], newNm});
        }
        auto isAllSuccess = g_undoRedo.Do(cmds);
        return isAllSuccess;
    }

    NameStandardizer ns;
};

#endif // FILESNAMEBATCHSTANDARDIZER_H
