#ifndef FILEEXPLOREREVENT_H
#define FILEEXPLOREREVENT_H
#include "UndoRedo.h"
#include <QAbstractItemModel>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QListView>
#include <QObject>
#include <QProcess>
#include <QTableView>
#include <QItemSelectionModel>
#include "Tools/PlayVideo.h"

class FileExplorerEvent : public QObject {
    Q_OBJECT
public:
    FileExplorerEvent(QObject *parent = nullptr, QFileSystemModel *fileSysModel_ = nullptr, QTableView *view_ = nullptr);
    void subscribe();

    auto onRenamePre() -> QPair<QString, QStringList>{
        QStringList preNames;
        for (QModelIndex ind: selectedIndexes()){
            preNames.append(fileSysModel->fileName(ind));
        }
        return {fileSysModel->rootPath(), preNames};
    }

    auto selectedIndexes() const -> QModelIndexList {
        // ignore other column, keep the first column
        if (view == nullptr) {
            return {};
        }
        if (qobject_cast<QListView *>(view) != nullptr) {
            return view->selectionModel()->selectedIndexes();
        }
        return view->selectionModel()->selectedRows();
    }

    bool on_revealInExplorer() const;

    auto on_moveToTrashBin() -> bool;

    auto on_deletePermanently() -> bool;

    bool on_Undo() const {
        if (not g_undoRedo.undoAvailable()) {
            qDebug("[skip] Nothing to undo");
            return true;
        }
        const bool isAllSucceed = g_undoRedo.Undo().first;
        qDebug(isAllSucceed ? "All undo succeed" : "Some undo failed.");
        return isAllSucceed;
    }

    bool on_Redo() const {
        if (not g_undoRedo.redoAvailable()) {
            qDebug("[skip] Nothing to redo");
            return true;
        }
        const bool isAllSucceed = g_undoRedo.Redo().first;
        qDebug(isAllSucceed ? "All redo succeed" : "Some redo failed.");
        return isAllSucceed;
    }

    auto on_SelectAll() -> void;
    auto on_SelectNone() -> void;
    auto on_SelectInvert() -> void;

    auto on_PlayVideo() const -> bool;


    auto on_copyFullPath() const -> bool;
    auto on_copyFullPathFolderNameAndAppendImageSuffix() const -> bool;
    auto on_copyDirPath() const -> bool;
    auto on_copyName() const -> bool;
    auto PathCopyTriple(const QStringList lst, const QString& opName) const -> bool;



    QFileSystemModel *fileSysModel;
    QTableView *view;
signals:
};

#endif // FILEEXPLOREREVENT_H
