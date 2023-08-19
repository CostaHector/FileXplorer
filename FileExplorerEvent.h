#ifndef FILEEXPLOREREVENT_H
#define FILEEXPLOREREVENT_H
#include <QObject>
#include <QFileSystemModel>
#include <QTableView>
#include <QAbstractItemModel>
#include <QProcess>
#include <QFileInfo>

class FileExplorerEvent: public QObject
{
Q_OBJECT
public:
    FileExplorerEvent(QObject *parent=nullptr, QFileSystemModel* fileSysModel_=nullptr, QTableView* view_=nullptr);
    void subscribe();

    bool on_revealInExplorer() const;
    QFileSystemModel* fileSysModel;
    QTableView* view;
signals:
};

#endif // FILEEXPLOREREVENT_H
