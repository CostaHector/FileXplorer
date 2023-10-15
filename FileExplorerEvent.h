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

    auto selectedIndexes() -> QModelIndexList {
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

    auto on_PlayVideo() -> bool;

    QFileSystemModel *fileSysModel;
    QTableView *view;
signals:
};

#endif // FILEEXPLOREREVENT_H
