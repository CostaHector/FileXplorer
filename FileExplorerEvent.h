﻿#ifndef FILEEXPLOREREVENT_H
#define FILEEXPLOREREVENT_H
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
#include "Tools/MyClipboard.h"
#include "Tools/RedundantFolderRemove.h"

#include "Model/MyQFileSystemModel.h"
class MyQFileSystemModel;
class ContentPanel;
class CustomStatusBar;
class MyClipboard;
class VideoPlayer;
class AlertSystem;
class AdvanceRenamer;
class PropertiesWindow;
class DuplicateVideosFinder;
class RedundantImageFinder;
class Archiver;

class FileExplorerEvent : public QObject {
  Q_OBJECT

 public:
  static FileExplorerEvent* GetFileExlorerEvent(MyQFileSystemModel* fsm, ContentPanel* view, CustomStatusBar* logger, QObject* parent = nullptr);

 private:
  FileExplorerEvent(MyQFileSystemModel* fsm, ContentPanel* view, CustomStatusBar* logger, QObject* parent);
  void subscribe();
  void subsribeCompress();
  void subsribeFileActions();
  void subscribeThumbnailActions();

  void onRename(AdvanceRenamer* renameWid);

  auto __CanNewItem() const -> bool;

  auto __FocusNewItem(const QString& itemPath) -> bool;

  auto on_NewTextFile(QString newTextName = "", const QString& contents = "") -> bool;
  auto on_NewJsonFile() -> bool;
  auto on_NewFolder() -> bool;
  auto on_BatchNewFilesOrFolders(const char* namePattern = "Page %03d.txt", int startIndex = 1, int endIndex = 11, bool isFolder = false) -> bool;
  auto on_BatchNewFilesOrFolders(bool isFolder = false) -> bool;

  bool on_ExtractImagesFromThumbnail(int beg, int end, bool skipIfExist = true);

  auto selectedIndexesProxyToSource() const -> QModelIndexList;
  auto selectedIndexes() const -> QModelIndexList;
  auto selectedItems() const -> QStringList {
    const auto& inds = selectedIndexes();
    QStringList filePaths;
    filePaths.reserve(inds.size());
    for (const QModelIndex& ind : inds) {
      filePaths << _fileSysModel->filePath(ind);
    }
    return filePaths;
  }

  bool on_searchKeywordInSystemDefaultExplorer() const;
  bool on_calcMD5() const;

  bool on_properties() const;

  bool on_revealInExplorer() const;
  bool on_OpenInTerminal() const;

  bool on_forceRefreshFileSystemModel();

  bool on_compress();
  bool on_deCompress();
  bool on_compressImgsByGroup();
  bool on_archivePreview();

  auto on_moveToTrashBin() -> bool;

  auto on_deletePermanently() -> bool;

  auto on_SelectAll() -> void;
  auto on_SelectNone() -> void;
  auto on_SelectInvert() -> void;

  bool on_HarView();
  auto on_PlaySelectedItemsInView() -> bool;
  auto on_PlayCurrentPathOfView() -> bool;

  auto on_PlayVideo() const -> bool;

  auto on_Merge(const bool reverse = false) -> bool;
  bool on_Copy();
  bool on_Cut();
  bool on_Paste();

  auto on_NameStandardize() -> bool;
  bool on_FileClassify();
  bool on_FileUnclassify();
  auto on_RemoveDuplicateImages() -> bool;
  auto on_RemoveRedundantItem(RedundantRmv& remover) -> bool;

  auto on_MoveCopyEventSkeleton(const CCMMode::Mode operationName, QString r) -> bool;
  auto on_MoveTo(const QString& r = "") -> bool { return this->on_MoveCopyEventSkeleton(CCMMode::CUT_OP, r); }
  auto on_CopyTo(const QString& r = "") -> bool { return this->on_MoveCopyEventSkeleton(CCMMode::COPY_OP, r); }

  MyQFileSystemModel* _fileSysModel;
  ContentPanel* _contentPane;

  CustomStatusBar* _logger;
  MyClipboard* m_clipboard;
  VideoPlayer* videoPlayer{nullptr};

  AlertSystem* m_alertSystem{nullptr};
  Archiver* m_archivePreview{nullptr};
  DuplicateVideosFinder* m_duplicateVideosFinder{nullptr};
  RedundantImageFinder* m_redundantImageFinder{nullptr};
 signals:
 private:
  CCMMode::Mode QueryCopyOrCut();
  FILE_STRUCTURE_MODE QueryKeepStructureOrFlatten(); // true: keep, false: flatten
};

#endif  // FILEEXPLOREREVENT_H
