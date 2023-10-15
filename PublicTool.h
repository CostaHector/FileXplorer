#ifndef PUBLICTOOL_H
#define PUBLICTOOL_H

#include <QString>
#include <QFileInfo>


class OSWalker_RETURN{
public:
    QStringList relToNames;
    QStringList completeNames;
    QStringList suffixs;
    QList<bool> isFiles;
};
OSWalker_RETURN OSWalker(const QString& pre, const QStringList& rels, const bool includingSub = false, const bool includingSuffix = false);

class PublicTool
{
public:
    PublicTool();

    static bool copyDirectoryFiles(const QString &fromDir, const QString &toDir, bool coverFileIfExist=false);
};

#include <QAction>
#include <QActionGroup>
#include <QLayout>

auto FindQActionFromQActionGroupByActionName(const QString& actionName, QActionGroup* ag) -> QAction*;
void SetLayoutAlightment(QLayout* lay, const Qt::AlignmentFlag align);
#endif // PUBLICTOOL_H
