#ifndef FILEEXPLOREREVENT_H
#define FILEEXPLOREREVENT_H
#include "Actions/RightClickMenuActions.h"
#include "Component/CustomStatusBar.h"
#include "Component/JsonEditor.h"
#include "Component/VideoPlayer.h"
#include "Tools/MimeDataCX.h"
#include "UndoRedo.h"

#include <QAbstractButton>
#include <QAbstractItemModel>
#include <QClipboard>
#include <QCryptographicHash>
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
                    CustomStatusBar* _statusBar = nullptr,
                    JsonEditor* jsonEditor_ = nullptr,
                    VideoPlayer* videoPlayer_ = nullptr,
                    T_UpdateComponentVisibility hotUpdate_ = T_UpdateComponentVisibility());
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

  auto on_NewTextFile(QString newTextName = "", const QString& contents = "") -> bool;
  auto on_NewJsonFile() -> bool;
  auto on_NewFolder() -> bool;
  auto on_BatchNewFilesOrFolders(const char* namePattern = "Page %03d.txt", int startIndex = 1, int endIndex = 11, bool isFolder = false) -> bool;
  auto on_BatchNewFilesOrFolders(bool isFolder = false) -> bool;

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

  static QString getMd5(const QString& filepath) {
    QFile file(filepath);
    file.open(QIODevice::ReadOnly);
    QCryptographicHash md5(QCryptographicHash::Md5);
    while (!file.atEnd()) {
      md5.addData(file.read(8192));
    }
    QString Md5Str = md5.result().toHex();
    file.close();
    return Md5Str;
  }

  bool on_calcMD5() const {
    const auto& inds = selectedIndexes();
    QStringList md5NameLst;
    md5NameLst.reserve(inds.size());
    for (const QModelIndex ind : inds) {
      QFileInfo fi(fileSysModel->fileInfo(ind));
      if (not fi.isFile()) {
        continue;
      }
      md5NameLst.append(getMd5(fi.absoluteFilePath()) + "\t" + fi.fileName());
    }
    QMessageBox msgBox(this->view);
    msgBox.setWindowIcon(g_rightClickActions()._CALC_MD5_ACT->icon());
    msgBox.setWindowTitle(QString("MD5 of selected %1 item(s)").arg(inds.size()));
    msgBox.setText("Click Show Detail to see md5 lists\n" + QString(256, '_'));
    msgBox.setDetailedText(md5NameLst.join('\n'));
    msgBox.exec();
    return true;
  }

  bool on_properties() const {
    const auto& inds = selectedIndexes();
    int fileCnt = 0;
    qint64 total = 0;
    for (const QModelIndex ind : inds) {
      QFileInfo fi(fileSysModel->fileInfo(ind));
      if (fi.isFile()) {
        total += fi.size();
        ++fileCnt;
        continue;
      }
      if (fi.isDir()) {
        QDirIterator it(fi.absoluteFilePath(), QDir::Filter::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
          it.next();
          total += it.fileInfo().size();
          ++fileCnt;
        }
      }
    }
    const qint64 xGiB = total / (1 << 30);
    const qint64 xMiB = total % (1 << 30) / (1 << 20);
    const qint64 xkiB = total % (1 << 30) % (1 << 20) / (1 << 10);
    const qint64 xB = total % (1 << 30) % (1 << 20) % (1 << 10);
    const QString sizeMsg = QString("%1GiB+%2MiB+%3KiB+%4Byte\t=\t%5B").arg(xGiB).arg(xMiB).arg(xkiB).arg(xB).arg(total);
    const QString cntMsg = QString("%1 File(s)").arg(fileCnt);

    QMessageBox msgBox(this->view);
    msgBox.setWindowIcon(g_rightClickActions()._PROPERTIES->icon());
    msgBox.setWindowTitle(QString("Size of selected %1 item(s)").arg(inds.size()));

    msgBox.setText(sizeMsg + "\n" + cntMsg);
    msgBox.exec();
    return true;
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

  auto on_OpenInVideoPlayer() const -> bool;
  auto on_PlayVideo() const -> bool;

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
  auto on_RemoveDuplicateImages() -> bool;

  auto on_MoveCopyEventSkeleton(const CCMMode operationName, QString r) -> bool;
  auto on_MoveTo(const QString& r = "") -> bool { return this->on_MoveCopyEventSkeleton(CCMMode::CUT, r); }
  auto on_CopyTo(const QString& r = "") -> bool { return this->on_MoveCopyEventSkeleton(CCMMode::COPY, r); }

  QFileSystemModel* fileSysModel;
  QTableView* view;
  CustomStatusBar* logger;
  QClipboard* clipboard;
  JsonEditor* jsonEditor;
  VideoPlayer* videoPlayer;
  T_UpdateComponentVisibility updateComponentVisibility;
 signals:
 private:
  auto QueryCopyOrCut() -> CCMMode;
};

#endif  // FILEEXPLOREREVENT_H
