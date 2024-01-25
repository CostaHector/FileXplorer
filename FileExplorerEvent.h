#ifndef FILEEXPLOREREVENT_H
#define FILEEXPLOREREVENT_H
#include "Component/CustomStatusBar.h"
#include "Component/JsonEditor.h"
#include "Component/MD5Window.h"
#include "Component/NotificatorFrame.h"
#include "Component/PropertiesWindow.h"
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
      qDebug("Reject. don't create item here[%s]", qPrintable(fileSysModel->rootPath()));
      Notificator::warning("Reject", QString("Don't create item here[%s]").arg(fileSysModel->rootPath()));
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
  auto selectedItems() const -> QStringList {
    const auto& inds = selectedIndexes();
    QStringList filePaths;
    filePaths.reserve(inds.size());
    for (const QModelIndex ind : inds) {
      filePaths << fileSysModel->filePath(ind);
    }
    return filePaths;
  }

  bool on_calcMD5() const {
    const QStringList& items = selectedItems();
    auto* md5W = new MD5Window(fileSysModel->rootPath(), items, this->view);
    md5W->show();
    return true;
  }

  bool on_properties() const {
    const QStringList& items = selectedItems();
    auto* pW = new PropertiesWindow(items, this->view);
    pW->show();
    return true;
  }

  bool on_revealInExplorer() const;

  auto on_moveToTrashBin() -> bool;

  auto on_deletePermanently() -> bool;

  auto on_SelectAll() -> void;
  auto on_SelectNone() -> void;
  auto on_SelectInvert() -> void;

  auto on_PlaySelectedItemsInView() const -> bool;
  auto on_PlayCurrentPathOfView() const -> bool;

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
