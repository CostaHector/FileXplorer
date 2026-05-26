#ifndef PAR2TOOLS_H
#define PAR2TOOLS_H

#include <QByteArray>
#include <QStringList>

namespace Par2Tools {
// folderPath: C:/Users/aria/Documents/testMultiPar2/rename
// par2FileName: WhenFileRenamed2.txt.par2
// volpar2FileName: [WhenFileRenamed2.txt.vol0+1.par2]
// newName: suggest from par2FileName

QString ChopPostfixVolAndPar2(QString fileName);

struct Par2Info {
  Par2Info(QByteArray&& setId_, QStringList&& oldFileNames_, int sourceFileCount_)
    : setId{std::move(setId_)}
    , oldFileNames{std::move(oldFileNames_)}
    , sourceFileCount{sourceFileCount_} {}
  QByteArray setId;         // PAR2 集合唯一标识
  QStringList oldFileNames; // 原内嵌源文件名列表
  int sourceFileCount = 0;  // Main 包中声明的源文件数量
  bool isTrustable() const;
};

std::pair<bool, QStringList> CheckInputParms(const QString& folderPath, const QString& par2FileName, const QStringList& volPar2Files);

// 解析第一个 PAR2 文件，提取 SetID 和源文件名
Par2Info ParsePar2File(const QString& par2FileAbsPath);

constexpr const char SYNC_PAR2_POSTFIX[]{"_SYNC"};
QString GetSyncPar2FileAbsPath(const QString& par2FileAbsPath);
// 遍历所有 PAR2 文件，修改 FileDesc 包并生成新文件
int ModifyPar2FilesFileDesc(const QStringList& par2FileList, const QStringList& newFileNames, const Par2Info& par2Info);

bool SyncBuiltInSrcFileListInPar2(const QString& folderPath, const QString& par2FileName, const QStringList& volPar2Files);
}; // namespace Par2Tools

#endif // PAR2TOOLS_H
