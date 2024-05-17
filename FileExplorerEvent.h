#ifndef FILEEXPLOREREVENT_H
#define FILEEXPLOREREVENT_H
#include "Component/CustomStatusBar.h"
#include "Component/JsonEditor.h"
#include "Component/MD5Window.h"

#include "Component/PropertiesWindow.h"
#include "Component/VideoPlayer.h"
#include "Component/AlertSystem.h"
#include "View/LogView.h"

#include "Tools/MyClipboard.h"
#include "Tools/RedundantFolderRemove.h"
#include "Component/RenameWidgets/AdvanceRenamer.h"

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

class FileExplorerEvent : public QObject {
  Q_OBJECT

 public:
  static FileExplorerEvent* GetFileExlorerEvent(MyQFileSystemModel* fsm,
                                               ContentPanel* view,
                                               CustomStatusBar* logger,
                                               QObject* parent = nullptr);

 private:
  FileExplorerEvent(MyQFileSystemModel* fsm, ContentPanel* view, CustomStatusBar* logger, QObject* parent);
  void subscribe();

  void onRename(AdvanceRenamer* renameWid);
  auto onRenamePre() const -> std::pair<QString, QStringList>;

  auto __CanNewItem() const -> bool;

  auto __FocusNewItem(const QString& itemPath) -> bool;

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
    auto* pW = new PropertiesWindow(this->_contentPane);
    pW->show();
    pW->operator()(items);
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
  auto on_MoveTo(const QString& r = "") -> bool { return this->on_MoveCopyEventSkeleton(CCMMode::CUT_OP, r); }
  auto on_CopyTo(const QString& r = "") -> bool { return this->on_MoveCopyEventSkeleton(CCMMode::COPY_OP, r); }

  MyQFileSystemModel* _fileSysModel;
  ContentPanel* _contentPane;

  CustomStatusBar* _logger;
  MyClipboard* m_clipboard;
  JsonEditor* jsonEditor{nullptr};
  VideoPlayer* videoPlayer{nullptr};

  AlertSystem* m_alertSystem{nullptr};
  LogView* m_logView{nullptr};
 signals:
 private:
  auto QueryCopyOrCut() -> CCMMode;
};

#endif  // FILEEXPLOREREVENT_H
