#ifndef FILEEXPLOREREVENT_H
#define FILEEXPLOREREVENT_H
#include "Component/CustomStatusBar.h"
#include "Component/JsonEditor.h"
#include "Component/MD5Window.h"
#include "Component/NotificatorFrame.h"
#include "Component/PropertiesWindow.h"
#include "Component/VideoPlayer.h"

#include "Tools/MyClipboard.h"
#include "Tools/MimeDataCX.h"
#include "Tools/RedundantFolderRemove.h"

#include "ContentPanel.h"

#include <QAbstractButton>
#include <QAbstractTableModel>
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
#include <QTextStream>

#include "View/AdvanceSearchTableView.h"

class FileExplorerEvent : public QObject {
  Q_OBJECT
 public:
  FileExplorerEvent(QObject* parent = nullptr,
                    MyQFileSystemModel* fsm = nullptr,
                    ContentPanel* view = nullptr,
                    CustomStatusBar* logger = nullptr,
                    T_UpdateComponentVisibility hotUpdate_ = T_UpdateComponentVisibility());
  void subscribe();

  auto onRenamePre() const -> QPair<QString, QStringList> {
    QStringList preNames;
    for (QModelIndex ind : selectedIndexes()) {
      preNames.append(_fileSysModel->fileName(ind));
    }
    return {_fileSysModel->rootPath(), preNames};
  }

  auto __CanNewItem() const -> bool {
    if (_fileSysModel->rootPath().isEmpty()) {
      qDebug("Reject. don't create item here[%s]", qPrintable(_fileSysModel->rootPath()));
      Notificator::warning("Reject", QString("Don't create item here[%s]").arg(_fileSysModel->rootPath()));
      return false;
    }
    return true;
  }

  auto __FocusNewItem(const QString& itemPath) -> bool {
    if (_contentPane->isFSView()){
      return false;
    }
    auto* view = _contentPane->GetCurView();
    const QModelIndex ind = _fileSysModel->index(itemPath);
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

  auto selectedIndexesProxyToSource() const -> QModelIndexList;
  auto selectedIndexes() const -> QModelIndexList;
  auto selectedItems() const -> QStringList {
    const auto& inds = selectedIndexes();
    QStringList filePaths;
    filePaths.reserve(inds.size());
    for (const QModelIndex ind : inds) {
      filePaths << _fileSysModel->filePath(ind);
    }
    return filePaths;
  }

  bool on_calcMD5() const {
    const QStringList& items = selectedItems();
    auto* md5W = new MD5Window(_fileSysModel->rootPath(), items, this->_contentPane);
    md5W->show();
    return true;
  }

  bool on_properties() const {
    const QStringList& items = selectedItems();
    auto* pW = new PropertiesWindow(items, this->_contentPane);
    pW->show();
    return true;
  }

  bool on_revealInExplorer() const;
  bool on_OpenInTerminal() const;

  auto on_moveToTrashBin() -> bool;

  auto on_deletePermanently() -> bool;

  auto on_SelectAll() -> void;
  auto on_SelectNone() -> void;
  auto on_SelectInvert() -> void;

  auto on_PlaySelectedItemsInView() -> bool;
  auto on_PlayCurrentPathOfView() -> bool;

  auto on_PlayVideo() const -> bool;

  auto on_Merge(const bool reverse = false) -> bool;
  auto on_Copy() -> bool;
  auto on_Cut() -> bool;
  auto on_Paste() -> bool;

  auto on_NameStandardize() -> bool;
  auto on_FileClassify() -> bool;
  auto on_RemoveDuplicateImages() -> bool;
  auto on_RemoveRedundantItem(RedundantRmv* remover) -> bool;

  auto on_MoveCopyEventSkeleton(const CCMMode operationName, QString r) -> bool;
  auto on_MoveTo(const QString& r = "") -> bool { return this->on_MoveCopyEventSkeleton(CCMMode::CUT, r); }
  auto on_CopyTo(const QString& r = "") -> bool { return this->on_MoveCopyEventSkeleton(CCMMode::COPY, r); }

  MyQFileSystemModel* _fileSysModel;
  ContentPanel* _contentPane;

  CustomStatusBar* _logger;
  MyClipboard* m_clipboard;
  JsonEditor* jsonEditor;
  VideoPlayer* videoPlayer;
  T_UpdateComponentVisibility updateComponentVisibility;
 signals:
 private:
  auto QueryCopyOrCut() -> CCMMode;
};

#endif  // FILEEXPLOREREVENT_H
