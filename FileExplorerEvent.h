#ifndef FILEEXPLOREREVENT_H
#define FILEEXPLOREREVENT_H
#include <QClipboard>
#include "ComplexOperation.h"
#include "RedundantFolderRemove.h"
#include "PublicVariable.h"
#include "FileSystemModel.h"

class ViewsStackedWidget;
class CustomStatusBar;
class AlertSystem;
class AdvanceRenamer;
class PropertiesWindow;
class DuplicateVideosFinder;
class RedundantImageFinder;
class Archiver;
class HarTableView;

class FileExplorerEvent : public QObject {
  Q_OBJECT

 public:
  static FileExplorerEvent* GetFileExlorerEvent(FileSystemModel* fsm, ViewsStackedWidget* view, CustomStatusBar* logger, QObject* parent = nullptr);

 private:
  FileExplorerEvent(FileSystemModel* fsm, ViewsStackedWidget* view, CustomStatusBar* logger, QObject* parent);
  void subscribe();
  void subsribeCompress();
  void subsribeFileActions();
  void subscribeThumbnailActions();

  void onRename(AdvanceRenamer& renameWid);

  bool __CanNewItem() const;

  bool __FocusNewItem(const QString& itemPath);

  bool on_NewTextFile();
  bool on_NewJsonFile();
  bool on_NewFolder();

  bool on_BatchNewFilesOrFolders(bool isFolder = false);

  bool on_CreateThumbnailImages(int dimensionX, int dimensionY, int widthPx);
  bool on_ExtractImagesFromThumbnail(int beg, int end, bool skipIfExist = true);

  QModelIndexList selectedIndexesProxyToSource() const;
  QModelIndexList selectedIndexes() const;
  QStringList selectedItems() const {
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
  void on_settings(const bool checked);

  bool on_revealInExplorer() const;
  bool on_OpenInTerminal() const;

  bool on_forceRefreshFileSystemModel();

  bool on_compress();
  bool on_deCompress();
  bool on_compressImgsByGroup();
  bool on_archivePreview();

  bool on_moveToTrashBin();

  bool on_deletePermanently();

  void on_SelectAll();
  void on_SelectNone();
  void on_SelectInvert();

  bool on_HarView();

  bool on_PlayVideo() const;

  bool on_Merge(const bool reverse = false);
  bool on_Copy();
  bool on_Cut();
  bool on_Paste();

  bool on_NameStandardize();
  bool on_FileClassify();
  bool on_FileUnclassify();
  bool on_RemoveDuplicateImages();
  bool on_RemoveRedundantItem(RedundantRmv& remover);
  void on_DUPLICATE_VIDEOS_FINDER(const bool checked);
  void on_DUPLICATE_IMAGES_FINDER(const bool checked);

  bool on_MoveCopyEventSkeleton(const Qt::DropAction& dropAct, QString r);
  bool on_MoveTo(const QString& r = "") { return on_MoveCopyEventSkeleton(Qt::DropAction::MoveAction, r); }
  bool on_CopyTo(const QString& r = "") { return on_MoveCopyEventSkeleton(Qt::DropAction::CopyAction, r); }

  void on_RMV_FOLDER_BY_KEYWORD();

  FileSystemModel* _fileSysModel{nullptr};
  ViewsStackedWidget* _contentPane{nullptr};

  CustomStatusBar* _logger{nullptr};
  QClipboard* m_clipboard{nullptr};

  AlertSystem* m_alertSystem{nullptr};
  Archiver* m_archivePreview{nullptr};
  DuplicateVideosFinder* m_duplicateVideosFinder{nullptr};
  RedundantImageFinder* m_redundantImageFinder{nullptr};

  HarTableView* m_harTableview{nullptr};

 private:
  bool QueryKeepStructureOrFlatten(ComplexOperation::FILE_STRUCTURE_MODE& mode); // true: keep, false: flatten
  static bool SetMimeDataCutCopy(QMimeData& mimeData, const Qt::DropAction dropAction);
};

#endif  // FILEEXPLOREREVENT_H
