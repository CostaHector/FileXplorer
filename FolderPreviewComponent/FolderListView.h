#ifndef FOLDERLISTVIEW_H
#define FOLDERLISTVIEW_H

#include <QListView>
#include "MyQFileSystemModel.h"

class FolderListView : public QListView
{
public:
    using QListView::contextMenuEvent;

    explicit FolderListView(MyQFileSystemModel* fileSystemModel_=nullptr, const QString& viewName_="");
    virtual auto InitViewSettings()->void = 0;
    bool operator()(const QString& path);
    void subscribe();;
    void contextMenuEvent(QContextMenuEvent* event) override;
    QAction* hideWidget;

protected:
    MyQFileSystemModel* m_fileSystemPreview;
    QMenu* m_listViewMenu;
    QString m_viewName;
};

#endif // FOLDERLISTVIEW_H
