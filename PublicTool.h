#ifndef PUBLICTOOL_H
#define PUBLICTOOL_H

#include <QString>
#include <QFileInfo>


class PublicTool
{
public:
    PublicTool();

    static bool copyDirectoryFiles(const QString &fromDir, const QString &toDir, bool coverFileIfExist=false);

};

#endif // PUBLICTOOL_H
