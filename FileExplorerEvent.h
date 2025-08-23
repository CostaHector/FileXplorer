#ifndef FILEEXPLOREREVENT_H
#define FILEEXPLOREREVENT_H
#include <QClipboard>
#include "ComplexOperation.h"
#include "RedundantFolderRemove.h"
#include "PublicVariable.h"
#include "FileSystemModel.h"

class ContentPanel;
class CustomStatusBar;
class AlertSystem;
class AdvanceRenamer;
class PropertiesWindow;
class DuplicateVideosFinder;
class RedundantImageFinder;
class Archiver;

class FileExplorerEvent : public QObject {
  Q_OBJECT

 public:
  static FileExplorerEvent* GetFileExlorerEvent(FileSystemModel* fsm, ContentPanel* view, CustomStatusBar* logger, QObject* parent = nullptr);

 private:
  FileExplorerEvent(FileSystemModel* fsm, ContentPanel* view, CustomStatusBar* logger, QObject* parent);
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

  bool on_CreateThumbnailImages(int dimensionX, int dimensionY, int widthPx);
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

  bool on_MoveCopyEventSkeleton(const Qt::DropAction& dropAct, QString r);
  bool on_MoveTo(const QString& r = "") { return on_MoveCopyEventSkeleton(Qt::DropAction::MoveAction, r); }
  bool on_CopyTo(const QString& r = "") { return on_MoveCopyEventSkeleton(Qt::DropAction::CopyAction, r); }

  FileSystemModel* _fileSysModel{nullptr};
  ContentPanel* _contentPane{nullptr};

  CustomStatusBar* _logger{nullptr};
  QClipboard* m_clipboard{nullptr};

  AlertSystem* m_alertSystem{nullptr};
  Archiver* m_archivePreview{nullptr};
  DuplicateVideosFinder* m_duplicateVideosFinder{nullptr};
  RedundantImageFinder* m_redundantImageFinder{nullptr};

 private:
  bool QueryKeepStructureOrFlatten(ComplexOperation::FILE_STRUCTURE_MODE& mode); // true: keep, false: flatten
  static bool SetMimeDataCutCopy(QMimeData& mimeData, const Qt::DropAction dropAction);
};

#endif  // FILEEXPLOREREVENT_H
