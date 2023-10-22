#ifndef FILEEXPLOREREVENT_H
#define FILEEXPLOREREVENT_H
#include "Component/CustomStatusBar.h"
#include "Component/RenameConflicts.h"
#include "Tools/Categorizer.h"
#include "Tools/FilesNameBatchStandardizer.h"
#include "Tools/MimeDataCX.h"
#include "Tools/PlayVideo.h"
#include "UndoRedo.h"

#include <QAbstractButton>
#include <QAbstractItemModel>
#include <QClipboard>
#include <QDateTime>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QGuiApplication>
#include <QItemSelectionModel>
#include <QListView>
#include <QMessageBox>
#include <QObject>
#include <QProcess>
#include <QTableView>
#include <QTextStream>

class FileExplorerEvent : public QObject {
  Q_OBJECT
 public:
  FileExplorerEvent(QObject* parent = nullptr,
                    QFileSystemModel* fileSysModel_ = nullptr,
                    QTableView* view_ = nullptr,
                    CustomStatusBar* _statusBar = nullptr);
  void subscribe();

  auto onRenamePre() -> QPair<QString, QStringList> {
    QStringList preNames;
    for (QModelIndex ind : selectedIndexes()) {
      preNames.append(fileSysModel->fileName(ind));
    }
    return {fileSysModel->rootPath(), preNames};
  }

  auto __CanNewItem() const -> bool {
    if (fileSysModel->rootPath().isEmpty()) {
      qDebug("New item only available on non-empty path[%s]", fileSysModel->rootPath().toStdString().c_str());
      return false;
    }
    return true;
  }

  auto __FocusNewItem(const QString& itemPath) -> bool {
    const QModelIndex ind = fileSysModel->index(itemPath);
    if (not ind.isValid()) {
      qDebug("Target Lose");
      return false;
    }
    view->clearSelection();
    view->setCurrentIndex(ind);
    return true;
  }

  auto on_NewTextFile(QString newTextName, const QString& contents) -> bool;
  auto on_NewJsonFile() -> bool;
  auto on_NewFolder() -> bool;

  auto selectedIndexes() const -> QModelIndexList {
    // ignore other column, keep the first column
    if (view == nullptr) {
      return {};
    }
    if (qobject_cast<QListView*>(view) != nullptr) {
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

  auto on_Merge(const bool reverse = false) -> bool;
  auto on_Copy() -> bool;
  auto on_Cut() -> bool;
  auto on_Paste() -> bool;

  auto FillMimeDataIntoClipboard(const CCMMode cutCopy = CCMMode::ERROR, const QString& fromPath = "") -> int {
    QStringList lRels;
    QList<QUrl> urls;
    for (const QModelIndex ind : selectedIndexes()) {
      lRels.append(fileSysModel->fileName(ind));
      urls.append(QUrl::fromLocalFile(lRels.back()));
    }
    if (lRels.isEmpty()) {
      return 0;
    }
    MimeDataCX* mimedata = new MimeDataCX(fromPath, lRels, cutCopy);
    mimedata->setUrls(urls);

    mimedata->setText(lRels.join('\n'));
    this->clipboard->setMimeData(mimedata);
    return urls.size();
  }
  auto on_NameStandardize() -> bool;
  auto on_FileClassify() -> bool;

  auto on_MoveCopyEventSkeleton(const CCMMode operationName, QString r) -> bool;
  auto on_MoveTo(const QString& r = "") -> bool { return this->on_MoveCopyEventSkeleton(CCMMode::CUT, r); }
  auto on_CopyTo(const QString& r = "") -> bool { return this->on_MoveCopyEventSkeleton(CCMMode::COPY, r); }

  QFileSystemModel* fileSysModel;
  QTableView* view;
  CustomStatusBar* logger;
  QClipboard* clipboard;
 signals:
 private:
  auto QueryCopyOrCut() -> CCMMode;
};

#endif  // FILEEXPLOREREVENT_H
