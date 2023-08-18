#ifndef FOLDERPREVIEWHTML_H
#define FOLDERPREVIEWHTML_H

#include <QTextEdit>

class FolderPreviewHTML : public QTextEdit
{
public:
    FolderPreviewHTML();
    static auto HtmlContentRelativePath2AbsPath(QString relPathSrc, const QString& prePath)->QString;
    auto operator()(const QString& path)->bool;
};

#endif // FOLDERPREVIEWHTML_H
