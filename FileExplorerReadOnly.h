#ifndef FILEEXPLORERREADONLY_H
#define FILEEXPLORERREADONLY_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QSettings>
#include <QRect>

#include "ContentPane.h"
#include "NavigationToolBar.h"
#include "OfficeStyleMenu.h"
class FileExplorerReadOnly : public QMainWindow
{
    Q_OBJECT

public:
    FileExplorerReadOnly(QWidget *parent = nullptr, const QString& initialPath="");
    ~FileExplorerReadOnly();
    virtual void closeEvent(QCloseEvent *event);


    auto ReadSettings(const QString& initialPath)->QString;
    bool subscribe();

    const static QString DEFAULT_PATH;
    QDockWidget* previewHtmlDock;
    FolderPreviewHTML* previewHtml;
    FolderPreviewWidget* previewWidget;
    ContentPane* explorerCentralWidget;
    NavigationToolBar* _navigationToolBar;
    OfficeStyleMenu* osm;
};
#endif // FILEEXPLORERREADONLY_H
