#ifndef CONFLICTSITEMHELPER_H
#define CONFLICTSITEMHELPER_H

#include <QString>
#include <QStringList>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QSet>
#include <QPair>

class Finder{
public:
    static auto FindAllItems(const QString& l, const QStringList& lRels, bool isMove = true) -> QStringList{
        if (lRels.isEmpty()){
            return {};
        }
        if (isMove){
            return lRels;;
        }

        const int n = l.size();
        QStringList itemList;
        for (const QString& lName: lRels){
            if (QFileInfo(l, lName).isFile()){
                itemList.append(lName);
            }else{
                QStringList lst;
                QDirIterator it(l + '/' + lName, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot, QDirIterator::IteratorFlag::Subdirectories);
                while (it.hasNext()){
                    it.next();
                    lst.append(it.filePath().mid(n + 1));
                }
                itemList += QStringList(lst.crbegin(), lst.crend());
            }
        }
        return itemList;
    };

    static auto FindLLRelRCommon(const QString& l, const QStringList& lRels, const QString& r) -> QStringList{
        if (lRels.isEmpty()){
            return {};
        }
        QStringList lFileLst;
        QStringList lFolderLst;
        for (const QString& lName: lRels){
            if (QFileInfo(l, lName).isFile()){
                lFileLst.append(lName);
            }else{
                lFolderLst.append(lName);
            }
        }

        auto rFolderLst = QDir(r, "", QDir::SortFlag::NoSort, QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot).entryList();
        auto commonFoldersSet = QSet<QString>(lFolderLst.cbegin(), lFolderLst.cend()).intersect(QSet<QString>(rFolderLst.cbegin(), rFolderLst.cend()));
        QStringList commonFolders(commonFoldersSet.cbegin(), commonFoldersSet.cend());
        commonFolders.sort(Qt::CaseSensitivity::CaseInsensitive);

        QStringList commonList;
        for (const QString& folder: commonFolders) {
            const QString& newl = l + '/' + folder;
            const QString& newr = r + '/' + folder;
            const QStringList& newlRels = QDir(newl, "", QDir::SortFlag::NoSort, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot).entryList();
            const QStringList& tempDup = Finder::FindLLRelRCommon(newl, newlRels, newr);
            for(const QString& d: tempDup){
                commonList.append(folder + '/' + d);
            }
            commonList.append(folder);
        }
        const QStringList& rFileLst = QDir(r, "", QDir::SortFlag::NoSort, QDir::Filter::Files).entryList();

        QSet<QString> commonFilesSet(lFileLst.cbegin(), lFileLst.cend());
        commonFilesSet.intersect(QSet<QString>(rFileLst.cbegin(), rFileLst.cend()));
        QStringList commonFiles(commonFilesSet.cbegin(), commonFilesSet.cend());
        commonFiles.sort(Qt::CaseSensitivity::CaseInsensitive);
        commonList += commonFiles;
        return commonList;
    }
};

class ConflictsItemHelper{
public:
    const QString l;
    const QString r;
    const QStringList lRels;
    const QStringList commonList;
    explicit ConflictsItemHelper(const QString& l_, const QString& r_, const QStringList& lRels_):
        l(l_), r(r_), lRels(lRels_), commonList(Finder::FindLLRelRCommon(l_, lRels_, r_)) {
    }
    explicit ConflictsItemHelper(const QPair<QString, QStringList>& lAndRels, const QString& r_):
        l(lAndRels.first), r(r_), lRels(lAndRels.second), commonList(Finder::FindLLRelRCommon(l, lRels, r_)) {
    }

    operator bool() const{
        return not commonList.isEmpty();
    }

    explicit ConflictsItemHelper(const QString& l_, const QString& r_):
        ConflictsItemHelper(l_, r_,
                            QDir(l_, "", QDir::SortFlag::NoSort, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot).entryList()){
    }

    static auto GetLAndRels(const QStringList& lAbsPathList)->QPair<QString, QStringList>{
        if (lAbsPathList.isEmpty()){
            return {"", lAbsPathList};
        }
        if (lAbsPathList.size() == 1){
            return {QFileInfo(lAbsPathList[0]).absolutePath(), {QFileInfo(lAbsPathList[0]).fileName()}};
        }
        const QStringList& first = lAbsPathList.front().split('/');
        const QStringList& last = lAbsPathList.back().split('/');
        int comparedCharsLength = std::min(first.size(), last.size());
        QString maxCommonPrepath;
        for (int i=0; i< comparedCharsLength; ++ i){
            if (first[i] == last[i]){
                maxCommonPrepath += (first[i] + '/');  // including trailing '/';
            }
        }
        if (maxCommonPrepath.isEmpty()){
            return {"", lAbsPathList};
        }

        QStringList lRels;
        const int N = maxCommonPrepath.size();
        for (const QString& lAbsPath: lAbsPathList){
            lRels.append(lAbsPath.mid(N));
        }
        return {maxCommonPrepath.chopped(1), lRels}; // remove the trailing '/'
    }
    explicit ConflictsItemHelper(const QStringList& lAbsPathList, const QString& r_):
        ConflictsItemHelper(GetLAndRels(lAbsPathList), r_)
    {}
    auto GetLeftRelPathList(const bool isMove) const -> QStringList{
        return Finder::FindAllItems(l, lRels, isMove);
    }
};
#endif // CONFLICTSITEMHELPER_H
