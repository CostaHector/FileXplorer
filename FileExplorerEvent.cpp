#include "FileExplorerEvent.h"

#include "ArchiveFilesActions.h"
#include "FileRenameRulerActions.h"
#include "FileBasicOperationsActions.h"
#include "RenameActions.h"
#include "RightClickMenuActions.h"
#include "ViewActions.h"
#include "ThumbnailProcessActions.h"
#include "TSFilesMerger.h"

#include "ConfigsTable.h"
#include "Archiver.h"
#include "CustomStatusBar.h"
#include "MD5Window.h"
#include "NotificatorMacro.h"
#include "PropertiesWindow.h"
#include "RedundantImageFinder.h"
#include "ViewsStackedWidget.h"
#include "RenameWidget_LongPath.h"
#include "RenameWidget_ArrangeSection.h"
#include "RenameWidget_ConsecutiveFileNo.h"
#include "RenameWidget_ConvertBoldUnicodeCharset2Ascii.h"
#include "RenameWidget_Case.h"
#include "RenameWidget_Insert.h"
#include "RenameWidget_Numerize.h"
#include "RenameWidget_Replace.h"
#include "RenameWidget_ReverseNames.h"
#include "RenameWidget_PrependParentFolderName.h"
#include "DuplicateVideosFinder.h"
#include "HarTableView.h"

#include "ItemsUnpacker.h"
#include "ArchiveFiles.h"
#include "ItemsPacker.h"
#include "CopyItemPropertiesToClipboardIF.h"
#include "LowResImgsRemover.h"
#include "FilesNameBatchStandardizer.h"
#include "JsonHelper.h"
#include "PlayVideo.h"
#include "SysTerminal.h"
#include "ViewTypeTool.h"
#include "ThumbnailProcesser.h"

#include "OnCheckedPopupOrHideAWidget.h"
#include "PublicTool.h"
#include "MemoryKey.h"
#include "StyleSheet.h"
#include "UndoRedo.h"
#include "ComplexOperation.h"
#include "CreateFileFolderHelper.h"
#include "JsonRenameRegex.h"

#include <QApplication>
#include <QInputDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QProcess>
#include <QUrl>
#include <QDesktopServices>

using namespace ViewTypeTool;

FileExplorerEvent::FileExplorerEvent(FileSystemModel* fsm, ViewsStackedWidget* view, CustomStatusBar* logger)
  : QObject{view} {  //

  CHECK_NULLPTR_RETURN_VOID(fsm)
  CHECK_NULLPTR_RETURN_VOID(view)
  CHECK_NULLPTR_RETURN_VOID(logger)

  _fileSysModel = fsm;
  _contentPane = view;
  _logger = logger;
  m_clipboard = QApplication::clipboard();
}

bool FileExplorerEvent::on_NewTextFile() {  // not effect by selection;
  if (!__CanNewItem()) {
    return false;
  }
  QString createIn{_contentPane->getRootPath()};
  QString textAbsPath;
  bool ret = CreateFileFolderHelper::NewPlainTextFile(createIn, &textAbsPath);
  if (!ret) {
    return false;
  }
  __FocusNewItem(textAbsPath);
  return ret;
}

bool FileExplorerEvent::on_NewJsonFile() {
  if (!__CanNewItem()) {
    return false;
  }
  const QString& createIn = _fileSysModel->rootPath();
  const QStringList& basedOnFileNames = selectedItems();
  return CreateFileFolderHelper::NewJsonFile(createIn, basedOnFileNames) >= 0;
}

bool FileExplorerEvent::on_NewFolder() {  // not effect by selection;
  if (!__CanNewItem()) {
    return false;
  }
  const QString createIn{_fileSysModel->rootPath()};
  QString folderAbsPath;
  bool ret = CreateFileFolderHelper::NewFolder(createIn, &folderAbsPath);
  if (!ret) {
    return false;
  }
  __FocusNewItem(folderAbsPath);
  return true;
}

bool FileExplorerEvent::on_BatchNewFilesOrFolders(bool isFolder) {
  if (!__CanNewItem()) {
    return false;
  }

  QString title{"Create Batch "};
  title += (isFolder ? "Folders" : "Files");
  const QString defNamePattern { isFolder ? //
                                   Configuration().value(MemoryKey::NAME_PATTERN_USED_CREATE_BATCH_FOLDERS.name, MemoryKey::NAME_PATTERN_USED_CREATE_BATCH_FOLDERS.v).toString()
                                        : Configuration().value(MemoryKey::NAME_PATTERN_USED_CREATE_BATCH_FILES.name, MemoryKey::NAME_PATTERN_USED_CREATE_BATCH_FILES.v).toString()};
  const QString userInputRule = QInputDialog::getText(_contentPane, title,
                                                      "Rule Pattern: C-style Format String$StartIndex$EndIndex",
                                                      QLineEdit::Normal, defNamePattern);
  const QStringList& userInputLst = userInputRule.split('$');
  if (userInputLst.size() != 3) {
    LOG_WARN_P("[Error] Invalid Rule pattern",
               "Rule pattern[%s] should contains 3 parts seperated by '$', but there are %d parts",
               qPrintable(userInputRule), userInputLst.size());
    return false;
  }
  const QString& namePattern = userInputLst[0];
  const int startIndex = userInputLst[1].toInt();
  const int endIndex = userInputLst[2].toInt();
  Configuration().setValue(isFolder ? MemoryKey::NAME_PATTERN_USED_CREATE_BATCH_FOLDERS.name//
                                    : MemoryKey::NAME_PATTERN_USED_CREATE_BATCH_FILES.name,//
                           userInputRule);
  const QString createIn {_fileSysModel->rootPath()};
  return CreateFileFolderHelper::NewItems(createIn, namePattern, startIndex, endIndex, isFolder);
}

bool FileExplorerEvent::on_CreateThumbnailImages(int dimensionX, int dimensionY, int widthPx) {
  if (!__CanNewItem()) {
    return false;
  }
  const QStringList& selectedFiles = selectedItems();
  if (selectedFiles.isEmpty()) {
    LOG_INFO_NP("Skip nothing selected", "selected some video(s) first");
    return true;
  }
  if (!ThumbnailProcesser::CheckParameters(dimensionX, dimensionY, widthPx)) {
    return false;
  }
  const int cnt = ThumbnailProcesser::CreateThumbnailImages(selectedFiles, dimensionX, dimensionY, widthPx, true);
  if (cnt <= 0) {
    LOG_ERR_NP("Create thumbnail failed", "see details in log");
    return false;
  }
  LOG_OK_P("Create thumbnail(s) for video(s) succeed", "count: %d", cnt);
  return true;
}

bool FileExplorerEvent::on_ExtractImagesFromThumbnail(int beg, int end, bool skipIfExist) {
  if (!__CanNewItem()) {
    return false;
  }
  const QString currentPath = _fileSysModel->rootPath();
  if (currentPath.count('/') < 3) {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Icon::Question);
    msgBox.setWindowIcon(QIcon(":img/THUMBNAIL_EXTRACTOR_B_E"));
    msgBox.setWindowTitle("Extract images out of thumbnail Confirm (lag may cause)?");
    msgBox.setText(QString("All images(s) under [%1] will be extract out!").arg(currentPath));
    msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    if (msgBox.exec() != QMessageBox::Ok) {
      LOG_INFO_NP("[Skip] User cancel", "will not extract images out of thumbnail");
      return true;
    }
  }
  ThumbnailProcesser tp{skipIfExist};
  int extractedOutCnt = tp(currentPath, beg, end);
  if (!tp.mErrImg.isEmpty()) {
    LOG_INFO_P("Extract Failed", "%d images, as follows:\n%s", extractedOutCnt, qPrintable(tp.mErrImg.join('\n')));
    return false;
  }
  if (extractedOutCnt == 0) {
    LOG_OK_NP("Nothing need extracted out", currentPath);
  } else {
    LOG_OK_P("[Ok] Extract", "%d image(s) from path[%s]", extractedOutCnt, qPrintable(currentPath));
  }
  return true;
}

QStringList FileExplorerEvent::selectedItems() const {
  const QModelIndexList& inds = _contentPane->GetCurView()->selectionModel()->selectedRows();
  QStringList filePaths;
  filePaths.reserve(inds.size());
  for (const QModelIndex& ind : inds) {
    filePaths.push_back(_fileSysModel->filePath(ind));
  }
  return filePaths;
}

bool FileExplorerEvent::on_searchKeywordInSystemDefaultExplorer() const {
  ViewTypeTool::ViewType vt = _contentPane->GetVt();
  if (!ViewTypeTool::IsChromeSearchAvail(vt)) {
    LOG_WARN_NP("Current View type not support search keyword", ViewTypeTool::c_str(vt));
    return false;
  }
  QString absFilePath = _contentPane->getCurFilePath();
  QString noExtAbsFilePath = PathTool::GetFileNameExtRemoved(absFilePath);
  QString imgFileAbsPathGuess = QDir::toNativeSeparators(noExtAbsFilePath) + ' ';
  m_clipboard->setText(imgFileAbsPathGuess, QClipboard::Mode::Clipboard);
  QString fileBaseName = PathTool::GetBaseName(absFilePath);
  QString forSearch = fileBaseName.replace(JSON_RENAME_REGEX::INVALID_GOOGLE_SEARCH_LETTER, " ");

  QString url;
  if (!forSearch.isEmpty()) {
    static const QString HTML_URL_TEMPLATE{"https://www.google.com/search?&q=%1&udm=2"};
    url = HTML_URL_TEMPLATE.arg(forSearch);
  } else {
    url = "https://www.google.com/";
  }

  if (!QDesktopServices::openUrl(QUrl{url})) {
    LOG_WARN_NP("[Failed] Cannot open URL in default browser:", url);
    return false;
  }
  LOG_OK_NP("[Ok] opened URL in default browser:", url);
  return true;
}

bool FileExplorerEvent::on_calcMD5() const {
  if (!_contentPane->IsCurFSView()) {
    LOG_INFO_NP("Current View type not support MD5 Calculator", _contentPane->GetCurViewName());
    return true;
  }
  auto* md5W = new MD5Window{this->_contentPane};
  const QStringList& items = selectedItems();
  const int filesCnt = md5W->operator()(items);
  md5W->show();
  LOG_D("%d md5(s) calculate.", filesCnt);
  return true;
}

bool FileExplorerEvent::on_properties() const {
  PropertiesWindow* pW = nullptr;
  auto vt = _contentPane->GetVt();
  if (ViewTypeTool::isFSView(vt)) {
    const QStringList& items = selectedItems();
    pW = new PropertiesWindow(this->_contentPane);
    pW->show();
    pW->operator()(items);
    return true;
  } else if (ViewTypeTool::IsMatch(vt, (int)ViewTypeMask::MOVIE)) {
    pW = new PropertiesWindow(this->_contentPane);
    pW->show();
    pW->operator()(_contentPane->m_movieDbModel, _contentPane->m_movieView);
    return true;
  }
  LOG_INFO_NP("Current View type not support Properties", ViewTypeTool::c_str(vt));
  return false;
}

void FileExplorerEvent::subsribeCompress() {
  connect(g_AchiveFilesActions().COMPRESSED_HERE, &QAction::triggered, this, &FileExplorerEvent::on_compress);
  connect(g_AchiveFilesActions().DECOMPRESSED_HERE, &QAction::triggered, this, &FileExplorerEvent::on_deCompress);
  connect(g_AchiveFilesActions().COMPRESSED_IMAGES, &QAction::triggered, this, &FileExplorerEvent::on_compressImgsByGroup);
  connect(g_AchiveFilesActions().ARCHIVE_PREVIEW, &QAction::triggered, this, &FileExplorerEvent::on_archivePreview);
}

void FileExplorerEvent::subscribeThumbnailActions() {
  auto& ins = g_ThumbnailProcessActions();
  connect(ins._CREATE_THUMBNAIL_AG, &QActionGroup::triggered, this, [this, &ins](QAction* createThumbnailAct) {
    auto it = ins.mCreateThumbnailDimension.find(createThumbnailAct);
    if (it == ins.mCreateThumbnailDimension.cend()) {
      LOG_W("create thumbnail action[%p] not support", createThumbnailAct);
      return;
    }
    int dimensionX = it->x, dimensionY = it->y, widthPixel = it->width;
    on_CreateThumbnailImages(dimensionX, dimensionY, widthPixel);
  });

  connect(ins._THUMBNAIL_SAMPLE_PERIOD, &QAction::triggered, this, [this]() {
    bool bok = false;
    const int curSamplePeriod = Configuration().value(MemoryKey::DEFAULT_THUMBNAIL_SAMPLE_PERIOD.name, MemoryKey::DEFAULT_THUMBNAIL_SAMPLE_PERIOD.v).toInt();
    const int newSamplePeriod = QInputDialog::getInt(this->_contentPane, "Thumbnail Sample Period(seconds)",                              //
                                                     "Set thumbnail image sample period to:", curSamplePeriod,             //
                                                     ThumbnailProcesser::SAMPLE_PERIOD_MIN, ThumbnailProcesser::SAMPLE_PERIOD_MAX, 1,  //
                                                     &bok);
    if (!bok) {
      LOG_INFO_NP("[Skip] User cancel set sample period", "return");
      return;
    }
    if (newSamplePeriod == curSamplePeriod) {
      return;
    }
    Configuration().setValue(MemoryKey::DEFAULT_THUMBNAIL_SAMPLE_PERIOD.name, newSamplePeriod);
    LOG_OK_P("ThumbnailSamplePeriod", "T=%d", newSamplePeriod);
  });

  connect(ins._EXTRACT_THUMBNAIL_AG, &QActionGroup::triggered, this, [this, &ins](QAction* extractThumbnailAct) {
    auto it = ins.mExtractThumbnailRange.find(extractThumbnailAct);
    if (it == ins.mExtractThumbnailRange.cend()) {
      LOG_W("extract thumbnail action[%p] not support", extractThumbnailAct);
      return;
    }
    const bool bSkipExist = ins._SKIP_IF_ALREADY_EXIST->isChecked();
    const int startIndex = it->startIndex, endIndex = it->endIndex;
    on_ExtractImagesFromThumbnail(startIndex, endIndex, bSkipExist);
  });

  connect(ins._CUSTOM_RANGE_IMGS, &QAction::triggered, this, [this, &ins]() {
    bool ok = false;
    const QString input = QInputDialog::getText(this->_contentPane, "Extract image range",  //
                                                "Enter range (e.g., 1,3; 1,4; 1,7):",       //
                                                QLineEdit::Normal, "0,9", &ok);             //
    if (!ok || input.isEmpty()) {
      LOG_INFO_P("[Skip] User canceled or invalid input", "input[%s]", qPrintable(input));
      return;
    }
    static const QRegularExpression regex(R"(^\d,\d$)");
    QRegularExpressionMatch match = regex.match(input);
    if (!match.hasMatch()) {
      LOG_WARN_NP("Invalid Range Format", input);
      return;
    }
    const int beg = match.captured(1).toInt();
    const int end = match.captured(2).toInt();
    if (beg < 0 || end < beg) {
      LOG_WARN_P("Invalid range", "Ensure 0 < beg <= end but[%s]", qPrintable(input));
      return;
    }
    bool bSkipExist = ins._SKIP_IF_ALREADY_EXIST->isChecked();
    on_ExtractImagesFromThumbnail(beg, end, bSkipExist);
  });
}

void FileExplorerEvent::subscribe() {
  subsribeCompress();
  subscribeThumbnailActions();

  {
    auto& fileOpInst = g_fileBasicOperationsActions();
    connect(fileOpInst.NEW_FOLDER, &QAction::triggered, this, &FileExplorerEvent::on_NewFolder);
    connect(fileOpInst.NEW_TEXT_FILE, &QAction::triggered, this, &FileExplorerEvent::on_NewTextFile);
    connect(fileOpInst.NEW_JSON_FILE, &QAction::triggered, this, &FileExplorerEvent::on_NewJsonFile);
    connect(fileOpInst.BATCH_NEW_FILES, &QAction::triggered, this, [this]() { FileExplorerEvent::on_BatchNewFilesOrFolders(false); });
    connect(fileOpInst.BATCH_NEW_FOLDERS, &QAction::triggered, this, [this]() { FileExplorerEvent::on_BatchNewFilesOrFolders(true); });

    connect(fileOpInst._REVEAL_IN_EXPLORER, &QAction::triggered, this, &FileExplorerEvent::on_revealInExplorer);
    connect(fileOpInst._OPEN_IN_TERMINAL, &QAction::triggered, this, &FileExplorerEvent::on_OpenInTerminal);

    connect(fileOpInst.UNDO_OPERATION, &QAction::triggered, this, &UndoRedo::on_Undo);
    connect(fileOpInst.REDO_OPERATION, &QAction::triggered, this, &UndoRedo::on_Redo);

    connect(fileOpInst.COPY_FULL_PATH, &QAction::triggered, _contentPane, [this]() { CopyItemPropertiesToClipboardIF::PathCopyTriple(_contentPane->getFilePaths(), "absolute-file-path"); });
    connect(fileOpInst.COPY_PATH, &QAction::triggered, _contentPane, [this]() { CopyItemPropertiesToClipboardIF::PathCopyTriple(_contentPane->getFilePrepaths(), "absolute-path"); });
    connect(fileOpInst.COPY_NAME, &QAction::triggered, _contentPane, [this]() { CopyItemPropertiesToClipboardIF::PathCopyTriple(_contentPane->getFileNames(), "file-name"); });
    connect(fileOpInst.COPY_THE_PATH, &QAction::triggered, _contentPane, [this]() { CopyItemPropertiesToClipboardIF::PathCopyTriple(_contentPane->getTheJpgFolderPaths(), "absolute-file-path+folderName+.jpg(in local seperator)"); });
    connect(fileOpInst.COPY_RECORDS, &QAction::triggered, _contentPane, [this]() { CopyItemPropertiesToClipboardIF::PathCopyTriple(_contentPane->getFullRecords(), "full-record"); });

    connect(fileOpInst.MOVE_TO_TRASHBIN, &QAction::triggered, this, &FileExplorerEvent::on_moveToTrashBin);
    connect(fileOpInst.DELETE_PERMANENTLY, &QAction::triggered, this, &FileExplorerEvent::on_deletePermanently);

    connect(fileOpInst.MERGE, &QAction::triggered, this, [this]() { on_Merge(false); });
    connect(fileOpInst.MERGE_REVERSE, &QAction::triggered, this, [this]() { on_Merge(true); });
    connect(fileOpInst._TS_FILES_MERGE, &QAction::triggered, this, &FileExplorerEvent::on_TsFilesMerge);

    connect(fileOpInst._MOVE_TO, &QAction::triggered, this, [this]() { this->on_MoveTo(); });
    connect(fileOpInst._COPY_TO, &QAction::triggered, this, [this]() { this->on_CopyTo(); });

    connect(fileOpInst.MOVE_TO_PATH_HISTORY, &QActionGroup::triggered, this, [this](const QAction* const act) { on_MoveTo(act->text()); });
    connect(fileOpInst.COPY_TO_PATH_HISTORY, &QActionGroup::triggered, this, [this](const QAction* const act) { on_CopyTo(act->text()); });

    connect(fileOpInst.CUT, &QAction::triggered, this, &FileExplorerEvent::on_Cut);
    connect(fileOpInst.COPY, &QAction::triggered, this, &FileExplorerEvent::on_Copy);
    connect(fileOpInst.PASTE, &QAction::triggered, this, &FileExplorerEvent::on_Paste);

    connect(fileOpInst._PACK_FOLDERS, &QAction::triggered, this, &FileExplorerEvent::on_FileClassify);
    connect(fileOpInst._UNPACK_FOLDERS, &QAction::triggered, this, &FileExplorerEvent::on_FileUnclassify);
    connect(fileOpInst._LOW_RESOLUTION_IMGS_RMV, &QAction::triggered, this, &FileExplorerEvent::on_RemoveDuplicateImages);
    connect(fileOpInst._RMV_01_FILE_FOLDER, &QAction::triggered, this, [this]() {
      ZeroOrOneItemFolderProc rfr;
      FileExplorerEvent::on_RemoveRedundantItem(rfr);
    });
    connect(fileOpInst._RMV_EMPTY_FOLDER, &QAction::triggered, this, [this]() {
      EmptyFolderRmv efr;
      FileExplorerEvent::on_RemoveRedundantItem(efr);
    });
    connect(fileOpInst._RMV_FOLDER_BY_KEYWORD, &QAction::triggered, this, &FileExplorerEvent::on_RMV_FOLDER_BY_KEYWORD);
    connect(fileOpInst._DUPLICATE_VIDEOS_FINDER, &QAction::toggled, this, &FileExplorerEvent::on_DUPLICATE_VIDEOS_FINDER);
    connect(fileOpInst._DUPLICATE_IMAGES_FINDER, &QAction::toggled, this, &FileExplorerEvent::on_DUPLICATE_IMAGES_FINDER);

    connect(fileOpInst.SELECT_ALL, &QAction::triggered, this, &FileExplorerEvent::on_SelectAll);
    connect(fileOpInst.SELECT_NONE, &QAction::triggered, this, &FileExplorerEvent::on_SelectNone);
    connect(fileOpInst.SELECT_INVERT, &QAction::triggered, this, &FileExplorerEvent::on_SelectInvert);

    connect(fileOpInst._LONG_PATH_FINDER, &QAction::triggered, this, [this]() -> void {
      RenameWidget_LongPath pToLongPath{_contentPane};
      on_Rename(pToLongPath);
    });
  }

  {
    auto& rulerInst = g_NameRulerActions();
    connect(rulerInst._NAME_RULER, &QAction::triggered, this, &FileExplorerEvent::on_NameStandardize);
  }

  {
    auto& rightClickMenuInst = g_rightClickActions();
    connect(rightClickMenuInst._SEARCH_IN_NET_EXPLORER, &QAction::triggered, this, &FileExplorerEvent::on_searchKeywordInSystemDefaultExplorer);
    connect(rightClickMenuInst._CALC_MD5_ACT, &QAction::triggered, this, &FileExplorerEvent::on_calcMD5);
    connect(rightClickMenuInst._PROPERTIES, &QAction::triggered, this, &FileExplorerEvent::on_properties);
    connect(rightClickMenuInst._FORCE_REFRESH_FILESYSTEMMODEL, &QAction::triggered, this, &FileExplorerEvent::on_forceRefreshFileSystemModel);
  }

  {
    auto& renameInst = g_renameAg();
    connect(renameInst._NUMERIZER, &QAction::triggered, this, [this]() -> void {
      RenameWidget_Numerize pNumerize{_contentPane};
      on_Rename(pNumerize);
    });
    connect(renameInst._SECTIONS_ARRANGE, &QAction::triggered, this, [this]() -> void {
      RenameWidget_ArrangeSection pArrange{_contentPane};
      on_Rename(pArrange);
    });
    connect(renameInst._REVERSE_NAMES_LIST, &QAction::triggered, this, [this]() -> void {
      RenameWidget_ReverseNames pReverse{_contentPane};
      on_Rename(pReverse);
    });
    connect(renameInst._CASE_NAME, &QAction::triggered, this, [this]() -> void {
      RenameWidget_Case pCase{_contentPane};
      on_Rename(pCase);
    });
    connect(renameInst._STR_INSERTER, &QAction::triggered, this, [this]() -> void {
      RenameWidget_Insert pInsert{_contentPane};
      on_Rename(pInsert);
    });
    connect(renameInst._STR_DELETER, &QAction::triggered, this, [this]() -> void {
      RenameWidget_Delete pDelete{_contentPane};
      on_Rename(pDelete);
    });
    connect(renameInst._STR_REPLACER, &QAction::triggered, this, [this]() -> void {
      RenameWidget_Replace pReplacer{_contentPane};
      on_Rename(pReplacer);
    });
    connect(renameInst._CONTINUOUS_NUMBERING, &QAction::triggered, this, [this]() -> void {
      RenameWidget_ConsecutiveFileNo pNoConsecutive{_contentPane};
      on_Rename(pNoConsecutive);
    });
    connect(renameInst._CONVERT_UNICODE_TO_ASCII, &QAction::triggered, this, [this]() -> void {
      RenameWidget_ConvertBoldUnicodeCharset2Ascii pToAscii{_contentPane};
      on_Rename(pToAscii);
    });
    connect(renameInst._PREPEND_PARENT_FOLDER_NAMES, &QAction::triggered, this, [this]() -> void {
      RenameWidget_PrependParentFolderName pPrependName{_contentPane};
      on_Rename(pPrependName);
    });
  }

  {
    auto& viewInst = g_viewActions();
    connect(viewInst._HAR_VIEW, &QAction::triggered, this, &FileExplorerEvent::on_HarView);
    connect(viewInst._SYS_VIDEO_PLAYERS, &QAction::triggered, this, &FileExplorerEvent::on_PlayVideo);
  }
}

void FileExplorerEvent::on_Rename(AdvanceRenamer& renameWid) {
  if (!_contentPane->IsCurFSView()) {
    LOG_WARN_NP("[Rename] Current view not support rename", _contentPane->GetCurViewName());
    return;
  }
  const QString& currentPath {_fileSysModel->rootPath()};
  if (PathTool::isLinuxRootOrWinEmpty(currentPath)) {
    LOG_WARN_NP("[Abort] Path root or empty", currentPath);
    return;
  }

  const QStringList& preNames = _contentPane->getFileNames();
  if (preNames.isEmpty()) {
    LOG_WARN_NP("[Skip] Nothing selected", "return");
    return;
  }

  const bool bPathSwichedAwayFirst {preNames.size() > 100};
  if (bPathSwichedAwayFirst) {
    _fileSysModel->setRootPath(""); // switch to another path
  }
  renameWid.init();
  renameWid.setModal(true);
  renameWid.InitTextEditContent(currentPath, preNames);
  if (renameWid.exec() != QDialog::Accepted) { // don't mixed with renameWid.show(); (even it can operate on former widget)
    LOG_INFO_P("[Cancel] rename", "User cancel rename %d item(s)", preNames.size());
  }
  if (bPathSwichedAwayFirst) {
    _fileSysModel->setRootPath(currentPath); // switch to another path
  }
}

bool FileExplorerEvent::__CanNewItem() const {
  if (!_contentPane->IsCurFSView()) {
    LOG_INFO_P("Reject New", "view name[%s]", _contentPane->GetCurViewName());
    return false;
  }
  if (PathTool::isLinuxRootOrWinEmpty(_fileSysModel->rootPath())) {
    LOG_INFO_P("Reject New", "Don't create item under path[%s]", qPrintable(_fileSysModel->rootPath()));
    return false;
  }
  return true;
}

bool FileExplorerEvent::__FocusNewItem(const QString& itemPath) {
  if (!_contentPane->IsCurFSView()) {
    return false;
  }
  auto* view = _contentPane->GetCurView();
  const QModelIndex ind = _fileSysModel->index(itemPath);
  if (!ind.isValid()) {
    LOG_D("target lost, skip focus new one");
    return false;
  }
  view->clearSelection();
  view->setCurrentIndex(ind);
  return true;
}

bool FileExplorerEvent::on_revealInExplorer() const {
  // hasSelection: reveal with selection
  // noSelection: folder -> open, file -> open its dir
  auto* view = _contentPane->GetCurView();
  QModelIndex curIndex = view->selectionModel()->currentIndex();

  QStringList args;
  if (!curIndex.isValid()) {
    args << QDir::toNativeSeparators(_contentPane->getRootPath());
  } else {
    const QString absPth{_contentPane->getFilePath(curIndex)};
    if (!QFile::exists(absPth)) {
      LOG_WARN_NP("Reveal path not exists", absPth);
      return false;
    }
#ifdef _WIN32
    args << "/e,"
         << "/select,";
#endif
    args << PathTool::sysPath(absPth);
  }
  QProcess process;
#ifdef _WIN32
  process.setProgram("explorer.exe");
#else
  process.setProgram("xdg-open");
#endif
  process.setArguments(args);
  process.startDetached();  // Start the process in detached mode instead of start
  LOG_W("on_revealInExplorer with program[%s] parms [%s]", qPrintable(process.program()), qPrintable(args.join(',')));
  return true;
}

bool FileExplorerEvent::on_OpenInTerminal() const {
  auto vt = _contentPane->GetVt();
  if (!ViewTypeTool::IsOpenInTerminalAvail(vt)) {
    LOG_WARN_NP("[Skip] Open In Terminal", _contentPane->GetCurViewName());
    return false;
  }
  const QString path = _contentPane->getRootPath();
  const QFileInfo fi{path};
  if (!fi.exists()) {
    LOG_D("path[%s] not exist", qPrintable(path));
    return false;
  }
#ifdef _WIN32
  LOG_W("WINDOWS not support now");
  return false;
#else
  const QString pth = QDir::toNativeSeparators(path);
  QProcess process;
  QStringList args;
  process.setProgram("gnome-terminal");
  args << QString("--working-directory=%1").arg(pth);
  process.setArguments(args);
  return process.startDetached();  // Start the process in detached mode instead of start
#endif
}

bool FileExplorerEvent::on_forceRefreshFileSystemModel() {
  if (!_contentPane->IsCurFSView()) {
    LOG_WARN_NP("[Skip] Refresh", _contentPane->GetCurViewName());
    return false;
  }
  QAbstractItemView* fsView = _contentPane->GetCurView();
  CHECK_NULLPTR_RETURN_FALSE(fsView)

  const QString& path = _fileSysModel->rootPath();
  _fileSysModel->setRootPath("");
  fsView->setRootIndex(_fileSysModel->setRootPath(path));
  LOG_OK_NP("Refresh filesytemmodel of path: ", path);
  return true;
}

bool FileExplorerEvent::on_compress() {
  auto vt = _contentPane->GetVt();
  if (!ViewTypeTool::IsDecompressHereAvail(vt)) {
    LOG_INFO_P("[Skip] Compress", "viewName:%s", _contentPane->GetCurViewName());
    return false;
  }
  const QString rootPath{_contentPane->getRootPath()};
  if (PathTool::isLinuxRootOrWinEmpty(rootPath)) {
    LOG_WARN_P("[Skip] Invalid Rootpath", "rootPath:%s", qPrintable(rootPath));
    return false;
  }

  const QStringList& filesPath = _contentPane->getFilePaths();
  if (filesPath.isEmpty()) {
    LOG_INFO_NP("[Skip] Compress", "nothing selected");
    return false;
  }
  const QString& compressedTo = _contentPane->getRootPath();
  const QString& archieveName = QFileInfo(compressedTo).completeBaseName() + ".qz";
  const QString& archievePath = QDir(compressedTo).absoluteFilePath(archieveName);
  ArchiveFiles af{archievePath, ArchiveFiles::NO_FILTER};
  bool compressedResult = af.CompressNow(ArchiveFiles::OPERATION_TYPE::FILES, filesPath, false);
  LOG_INFO_P("Compressed:", "bResult: %d", compressedResult);
  return compressedResult;
}

bool FileExplorerEvent::on_deCompress() {
  auto vt = _contentPane->GetVt();
  if (!ViewTypeTool::IsDecompressHereAvail(vt)) {
    LOG_INFO_P("[Skip] Decompress", "viewName:%s", _contentPane->GetCurViewName());
    return false;
  }
  const QString rootPath{_contentPane->getRootPath()};
  if (PathTool::isLinuxRootOrWinEmpty(rootPath)) {
    LOG_WARN_P("[Skip] Invalid Rootpath", "rootPath:%s", qPrintable(rootPath));
    return false;
  }

  const QStringList& filesPath = _contentPane->getFilePaths();
  if (filesPath.isEmpty()) {
    LOG_INFO_P("Skip Decompress", "Nothing selected");
    return true;
  }
  QStringList failsQzFiles;
  for (const QString& filePath : filesPath) {
    const QString& decompressedTo = QFileInfo(filePath).absolutePath();
    ArchiveFiles af{filePath};
    bool decompressedResult = af.DecompressToPath(decompressedTo);
    if (!decompressedResult) {
      failsQzFiles << filePath;
    }
  }
  LOG_I("Decompress %d file(s), %d failed", filesPath.size(), failsQzFiles.size());
  if (!failsQzFiles.isEmpty()) {
    LOG_ERR_P("[Partially Failed] Decompress qz", "following %d file(s) failed:\n%s", failsQzFiles.size(), qPrintable(failsQzFiles.join('\n')));
  } else {
    LOG_OK_P("[Ok] Decompress qz", "%d qz files decompressed ok", filesPath.size());
  }
  return failsQzFiles.isEmpty();
}

bool FileExplorerEvent::on_compressImgsByGroup() {
  if (!_contentPane->IsCurFSView()) {
    LOG_INFO_P("[Skip] Compress images", "viewName:%s", _contentPane->GetCurViewName());
    return false;
  }
  const QString pth {_contentPane->getRootPath()};
  ArchiveImagesRecusive air{true};
  int compressFolderCnt = air.CompressImgRecur(pth);
  LOG_INFO_P("[Ok] Compress images", "count: %d", compressFolderCnt);
  return true;
}

bool FileExplorerEvent::on_archivePreview() {
  auto vt = _contentPane->GetVt();
  if (!ViewTypeTool::IsDecompressHereAvail(vt)) {
    return false;
  }
  const QString filePath = _contentPane->getCurFilePath();
  if (filePath.isEmpty()) {
    LOG_WARN_NP("[Abort ArchivePreivew]", "path empty");
    return false;
  }
  if (m_archivePreview == nullptr) {
    m_archivePreview = new (std::nothrow) Archiver;
  }
  bool previewRet = m_archivePreview->operator()(filePath);
  m_archivePreview->show();
  m_archivePreview->activateWindow();
  m_archivePreview->raise();
  return previewRet;
}

bool FileExplorerEvent::on_moveToTrashBin() {
  ViewTypeTool::ViewType vt = _contentPane->GetVt();
  if (!ViewTypeTool::IsDecompressHereAvail(vt)) {
    LOG_WARN_NP("[Abort] Current view not support MoveToTrashbin", _contentPane->GetCurViewName());
    return false;
  }
  const QString pth = _contentPane->getRootPath();
  if (PathTool::isLinuxRootOrWinEmpty(pth)) {
    LOG_WARN_NP("[Abort] Path not recyclable", pth);
    return false;
  }

  QStringList prepaths, names;
  std::tie(prepaths, names) = _contentPane->getFilePrepathsAndName(true);
  if (names.size() <= 0) {
    LOG_INFO_NP("[Skip]", "Nothing selected");
    return true;
  }
  if (prepaths.size() != names.size()) {
    LOG_INFO_P("[Skip]", "Prepaths[%d] and Names[%d] length Inequal", prepaths.size(), names.size());
    return true;
  }

  using namespace FileOperatorType;
  BATCH_COMMAND_LIST_TYPE removeCmds;
  removeCmds.reserve(prepaths.size());
  for (int i = 0; i < prepaths.size(); ++i) {
    removeCmds.append(ACMD::GetInstMOVETOTRASH(prepaths[i], names[i]));
  }
  if (!UndoRedo::GetInst().Do(removeCmds)) {
    LOG_ERR_NP("[MoveToTrash] Partially failed", "Some item(s) move to trashbin failed");
    return false;
  }
  LOG_OK_P("[MoveToTrash] All succeed", "Commands count %d", removeCmds.size());
  return true;
}

bool FileExplorerEvent::on_deletePermanently() {
  auto vt = _contentPane->GetVt();
  if (!ViewTypeTool::isFSView(vt)) {
    LOG_WARN_NP("[Abort] Current view not support Delete Permanently", _contentPane->GetCurViewName());
    return false;
  }
  const QString pth = _contentPane->getRootPath();
  if (PathTool::isLinuxRootOrWinEmpty(pth)) {
    LOG_INFO_P("[Abort] Delete Permanently", "rootPath:%s", qPrintable(pth));
    return false;
  }

  auto* view = _contentPane->GetCurView();
  using namespace FileOperatorType;

  const auto& indexes = view->selectionModel()->selectedRows();
  QStringList deleteNames;
  deleteNames.reserve(indexes.size());
  BATCH_COMMAND_LIST_TYPE cmds;
  cmds.reserve(indexes.size());
  for (const QModelIndex& ind : indexes) {
    const QFileInfo& fi = _fileSysModel->fileInfo(ind);
    if (fi.isDir()) {
      cmds.push_back(ACMD::GetInstRMFOLDERFORCE(pth, fi.fileName()));
    } else if (fi.isFile()) {
      cmds.push_back(ACMD::GetInstRMFILE(pth, fi.fileName()));
    } else {
      LOG_I("Here may some types not removed [%s]", qPrintable(fi.filePath()));
      continue;
    }
    deleteNames.push_back(fi.fileName());
  }

  static constexpr int MAX_FILE_NAME_DISP = 20;
  QString fileNames;
  if (deleteNames.size() > MAX_FILE_NAME_DISP) {
    fileNames += deleteNames.mid(0, MAX_FILE_NAME_DISP).join('\n');
    fileNames += QString("\n---\nAnd the Following %1 item(s)\n---\n").arg(cmds.size() - MAX_FILE_NAME_DISP);
    fileNames += deleteNames.constLast();
  } else {
    fileNames += deleteNames.join('\n');
  }
  QMessageBox msgBox;
  msgBox.setIcon(QMessageBox::Icon::Warning);
  msgBox.setWindowIcon(QIcon(":img/DELETE_ITEMS_PERMANENTLY"));
  msgBox.setWindowTitle("Confirm PERMANENTLY DELETE?");
  msgBox.setText(QString{"<b>NOT RECOVERABLE!</b> Following <b>%1</b> item(s) are about to delete permanently:"}.arg(cmds.size()));
  msgBox.setInformativeText(fileNames);
  msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
  msgBox.setDefaultButton(QMessageBox::Cancel);
  if (msgBox.exec() != QMessageBox::Ok) {
    LOG_INFO_P("[Cancel] User cancel delete", "%d item(s) no change", cmds.size());
    return true;
  }
  if (!UndoRedo::GetInst().Do(cmds)) {
    LOG_ERR_P("[Failed] Partial item(s) deleted failed", "%d cmds", cmds.size());
    return false;
  }
  LOG_OK_P("[Ok] All item(s) deleted succeed", "%d cmds", cmds.size());
  return true;
}

void FileExplorerEvent::on_SelectAll() {
  auto* view = _contentPane->GetCurView();
  if (!view->hasFocus()) {
    return;
  }
  view->selectAll();
}

void FileExplorerEvent::on_SelectNone() {
  auto* view = _contentPane->GetCurView();
  if (!view->hasFocus()) {
    return;
  }
  view->clearSelection();
}

void FileExplorerEvent::on_SelectInvert() {
  QAbstractItemView* view = _contentPane->GetCurView();
  if (!view->hasFocus()) {
    return;
  }
  QModelIndex leftTop, rightBottom;
  std::tie(leftTop, rightBottom) = _contentPane->getTopLeftAndRightDownRectangleIndex();
  if (!leftTop.isValid() || !rightBottom.isValid()) {
    LOG_INFO_NP("[Skip] invert selection", "rectangle indexes invalid");
    return;
  }
  view->selectionModel()->select(QItemSelection{leftTop, rightBottom}, QItemSelectionModel::Toggle);
}

bool FileExplorerEvent::on_HarView() {
  if (!_contentPane->hasSelection()) {
    LOG_INFO_NP("Nothing selected", "return");
    return false;
  }
  const QString& fileAbsPath = _contentPane->getCurFilePath();
  if (!fileAbsPath.endsWith(".har", Qt::CaseInsensitive)) {
    LOG_INFO_NP("File extension is not .har", fileAbsPath);
    return false;
  }
  if (!QFile::exists(fileAbsPath)) {
    LOG_WARN_NP("Har file not exist", fileAbsPath);
    return false;
  }
  auto* harTableview = new (std::nothrow) HarTableView{_contentPane};
  CHECK_NULLPTR_RETURN_FALSE(harTableview);
  harTableview->operator()(fileAbsPath);
  harTableview->raise();
  harTableview->show();
  return true;
}

auto FileExplorerEvent::on_PlayVideo() const -> bool {
  auto vt = _contentPane->GetVt();
  if (!ViewTypeTool::IsOpenVideosAvail(vt)) {
    LOG_INFO_NP("[Abort] Current view not support Play Video", ViewTypeTool::c_str(vt));
    return false;
  }
  // if select an item, path selected path
  QString playPath = _contentPane->getCurFilePath();
  if (playPath.isEmpty() && ViewTypeTool::isFSView(vt)) {
    // if select nothing and file-system model, play current path
    playPath = _fileSysModel->rootPath();
  }
  if (PathTool::isRootOrEmpty(playPath)) {
    LOG_INFO_P("[Abort] Play", "playPath[%s]", qPrintable(playPath));
    return true;
  }
  if (!on_ShiftEnterPlayVideo(playPath)) {
    LOG_INFO_NP("[Failed] Play", playPath);
    return false;
  }
  LOG_OK_NP("Playing...", playPath);
  _logger->onMsgChanged(QString{"Playing... %1"}.arg(playPath), STATUS_ALERT_LEVEL::NORMAL);
  return true;
}

bool FileExplorerEvent::on_Merge(const bool isReverse) {
  if (!_contentPane->IsCurFSView()) {
    return false;
  }
  if (PathTool::isLinuxRootOrWinEmpty(_fileSysModel->rootPath())) {
    return false;
  }

  static const auto GetMergeFromToPath = [] (const QFileSystemModel* fsm, QModelIndex ind1, QModelIndex ind2, bool isReverse) -> std::pair<QString, QString> {
    QString fromPath{fsm->filePath(ind1)};
    QString toPath{fsm->filePath(ind2)};
    const bool shouldSwap = (ind1.row() < ind2.row()) ? isReverse : !isReverse;
    if (shouldSwap) {
      fromPath.swap(toPath);
    }
    return {fromPath, toPath};
  };

  const QAbstractItemView* view = _contentPane->GetCurView();
  const QModelIndexList& inds = view->selectionModel()->selectedRows();
  if (inds.size() != 2) {
    LOG_WARN_P("[Skip Merge] not 2 items", "selections count[%d] is not 2", inds.size());
    return false;
  }
  QString fromPath;
  QString toPath;
  std::tie(fromPath, toPath) = GetMergeFromToPath(_fileSysModel, inds.constFirst(), inds.constLast(), isReverse);
  if (!QFileInfo{fromPath}.isDir() || !QFileInfo{toPath}.isDir()) {
    LOG_WARN_P("[Skip Merge] not 2 folder", "%s\n%s", qPrintable(fromPath), qPrintable(toPath));
    return false;
  }
  using namespace ComplexOperation;
  ComplexMerge cm;
  BATCH_COMMAND_LIST_TYPE aBatch = cm.Merge(fromPath, toPath);
  if (!UndoRedo::GetInst().Do(aBatch)) {
    LOG_ERR_P("[Failed] Partial merge failed", "%s\n->\n%s", qPrintable(fromPath), qPrintable(toPath));
    return false;
  }
  LOG_OK_P("[Ok] All merged succeed", "%s\n->\n%s", qPrintable(fromPath), qPrintable(toPath));
  _logger->onMsgChanged(QString("Succeed merged folder into %1").arg(toPath), STATUS_ALERT_LEVEL::NORMAL);
  return true;
}

void FileExplorerEvent::on_TsFilesMerge() {
  auto vt = _contentPane->GetVt();
  if (!ViewTypeTool::isFSView(vt)) {
    LOG_WARN_NP("[Skip] Current view type not support ts merge", ViewTypeTool::c_str(vt));
    return;
  }
  if (!_contentPane->hasSelection()) {
    LOG_INFO_P("[Skip] User select nothing", "return");
    return;
  }
  const QStringList& tsNames = _contentPane->getFileNames();
  if (tsNames.size() < 2) {
    LOG_INFO_P("[Skip] User select too less file", "%d file", tsNames.size());
    return;
  }
  const QString& currentPath = _contentPane->getRootPath();
  bool mergeResult{false};
  QString largeTsAbsFilePath;
  std::tie(mergeResult, largeTsAbsFilePath) = TSFilesMerger::mergeTsFiles(currentPath, tsNames);
  if (!mergeResult) {
    LOG_ERR_P("[Failed] Merge ts file failed", "User selected %d file(s)", mergeResult);
    return;
  }
  LOG_OK_P("[Ok] Merge ts file succeed", "%d file(s) to\n%s", mergeResult, qPrintable(largeTsAbsFilePath));
  _logger->onMsgChanged(QString("Succeed merged ts file(s) into %1").arg(largeTsAbsFilePath), STATUS_ALERT_LEVEL::NORMAL);
}

bool FileExplorerEvent::on_Copy() {
  using namespace MimeDataHelper;
  MimeDataMember mimeDataRet = _contentPane->getFilePathsAndUrls(Qt::CopyAction);
  const int pathCnt = WriteIntoSystemClipboard(mimeDataRet, Qt::CopyAction);
  _logger->onMsgChanged(QString("%1 path(s) been copied").arg(pathCnt),
                        (pathCnt >= 0 ? STATUS_ALERT_LEVEL::NORMAL : STATUS_ALERT_LEVEL::ABNORMAL));
  return pathCnt >= 0;
}

bool FileExplorerEvent::on_Cut() {
  using namespace MimeDataHelper;
  MimeDataMember mimeDataRet = _contentPane->getFilePathsAndUrls(Qt::MoveAction);
  const int pathCnt = WriteIntoSystemClipboard(mimeDataRet, Qt::MoveAction);
  _logger->onMsgChanged(QString("%1 path(s) been cut").arg(pathCnt),
                        (pathCnt >= 0 ? STATUS_ALERT_LEVEL::NORMAL : STATUS_ALERT_LEVEL::ABNORMAL));
  return pathCnt >= 0;
}

bool FileExplorerEvent::on_Paste() {
  if (!_contentPane->IsCurFSView()) {
    LOG_WARN_NP("[Skip] Current view not support paste", _contentPane->GetCurViewName());
    return false;
  }
  const QString& rTo = _fileSysModel->rootPath();
  if (rTo == "C:/") {
    LOG_WARN_NP("Paste destination cannot be C:/", "Do this operation on system explorer.");
    return false;
  }
  if (PathTool::isLinuxRootOrWinEmpty(rTo)) {
    LOG_WARN_NP("[Paste Skip] Paste destination cannot be empty", rTo);
    return false;
  }
  auto* pMimeData = m_clipboard->mimeData();
  if (pMimeData == nullptr) {
    LOG_CRIT_NP("[Paste Skip] pMimeData is nullptr", "Cannot paste here");
    return false;
  }
  if (!pMimeData->hasUrls()) {
    LOG_CRIT_NP("[Paste Skip] no urls in Clipboard", "Cannot paste here");
    return false;
  }
  using namespace ComplexOperation;
  FILE_STRUCTURE_MODE fileStructMode{GetDefaultFileStructMode()};
  if (fileStructMode == FILE_STRUCTURE_MODE::QUERY) {
    if (!QueryKeepStructureOrFlatten(fileStructMode)) {
      LOG_OK_NP("User cancel paste", "neither flatten or keep");
      return true;
    }
  }
  Qt::DropAction dropAction = GetCutCopyModeFromNativeMimeData(*pMimeData);
  int ret = DoDropAction(dropAction, pMimeData->urls(), rTo, fileStructMode);
  if (ret < 0) {
    LOG_ERR_NP("Paste operation partially failed", rTo);
    return false;
  }
  LOG_OK_NP("Paste operation all succeed", rTo);
  _fileSysModel->ClearCopyAndCutDict();
  if (_contentPane->m_searchSrcModel != nullptr) {
    _contentPane->m_searchSrcModel->ClearCopyAndCutDict();
  }
  return true;
}

bool FileExplorerEvent::on_NameStandardize() {
  if (!_contentPane->IsCurFSView()) {
    LOG_INFO_NP("[Skip] Current view not support NameStandardize", _contentPane->GetCurViewName());
    return false;
  }
  const QString currentPath{_fileSysModel->rootPath()};
  if (PathTool::isRootOrEmpty(currentPath)) {
    LOG_ERR_NP("currentPath is root or empty", currentPath);
    return false;
  }
  QMessageBox msgBox;
  msgBox.setIcon(QMessageBox::Icon::Warning);
  msgBox.setWindowIcon(QIcon(":img/NAME_RULER"));
  msgBox.setWindowTitle("Confirm Name Standardlizer?");
  msgBox.setText(QString{"All item(s) under [%1] will be <b>RENAMED RECURSIVELY</b>!"}.arg(currentPath));
  msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
  msgBox.setDefaultButton(QMessageBox::Ok);
  if (msgBox.exec() != QMessageBox::Ok) {
    LOG_INFO_NP("[Skip] User cancel", "return");
    return true;
  }
  auto* p = Notificator::progress(LOG_LVL_E::I, "Name standardlize", QString{"Start in path[%1]"}.arg(currentPath));
  CHECK_NULLPTR_RETURN_FALSE(p);

  FilesNameBatchStandardizer fnbs;
  if (!fnbs(currentPath)) {
    p->setProgressFailed();
    return false;
  }
  p->setProgressValue(100);
  return true;
}

bool FileExplorerEvent::on_FileClassify() {
  if (!_contentPane->IsCurFSView()) {
    LOG_INFO_NP("[Skip] Current view not support FileClassify", _contentPane->GetCurViewName());
    return false;
  }
  const QString currentPath{_fileSysModel->rootPath()};
  if (PathTool::isRootOrEmpty(currentPath)) {
    LOG_ERR_NP("currentPath is root or empty", currentPath);
    return false;
  }

  QMessageBox msgBox;
  msgBox.setIcon(QMessageBox::Icon::Warning);
  msgBox.setWindowIcon(QIcon{":img/PACK_FOLDERS"});
  msgBox.setWindowTitle("Confirm File Classify?");
  msgBox.setText(QString{"All item(s) under [%1] will be classfied!"}.arg(currentPath));
  msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
  msgBox.setDefaultButton(QMessageBox::Ok);
  if (msgBox.exec() != QMessageBox::Ok) {
    LOG_INFO_NP("[Skip] User cancel", "return");
    return true;
  }

  ItemsPacker classfier;
  classfier(currentPath);

  auto* p = Notificator::progress(LOG_LVL_E::I, "File Classify", QString{"Start in path[%1] command(s) count[%2]"}.arg(currentPath).arg(classfier.CommandsCnt()));
  CHECK_NULLPTR_RETURN_FALSE(p);

  _fileSysModel->setRootPath(""); // switch to another path
  bool classifyResult = classfier.StartToRearrange();
  if (!classifyResult) {
    p->setProgressFailed();
  } else {
    p->setProgressValue(100);
  }

  auto* view = _contentPane->GetCurView();
  view->setRootIndex(_fileSysModel->setRootPath(currentPath));
  return classifyResult;
}

bool FileExplorerEvent::on_FileUnclassify() {
  if (!_contentPane->IsCurFSView()) {
    LOG_INFO_NP("[Skip] Current view not support FileUnclassify", _contentPane->GetCurViewName());
    return false;
  }
  const QString currentPath{_fileSysModel->rootPath()};
  if (PathTool::isRootOrEmpty(currentPath)) {
    LOG_ERR_NP("currentPath is root or empty", currentPath);
    return false;
  }

  QMessageBox msgBox;
  msgBox.setIcon(QMessageBox::Icon::Warning);
  msgBox.setWindowIcon(QIcon(":img/UNPACK_FOLDERS"));
  msgBox.setWindowTitle("Unpile Confirm (lag may cause)?");
  msgBox.setText(QString("All item(s) under [%1] will be unpile out!").arg(currentPath));
  msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
  msgBox.setDefaultButton(QMessageBox::Ok);
  if (msgBox.exec() != QMessageBox::Ok) {
    LOG_INFO_NP("[Skip] User cancel", "Skip unpile");
    return true;
  }

  _fileSysModel->setRootPath("");

  ItemsUnpacker unclassfier;
  unclassfier(currentPath);

  auto* p = Notificator::progress(LOG_LVL_E::I, "File Unclassify", QString{"Start in path[%1] command(s) count[%2]"}.arg(currentPath).arg(unclassfier.CommandsCnt()));
  CHECK_NULLPTR_RETURN_FALSE(p);

  bool unclassifyResult = unclassfier.StartToRearrange();
  if (!unclassifyResult) {
    p->setProgressFailed();
  } else {
    p->setProgressValue(100);
  }

  auto* view = _contentPane->GetCurView();
  view->setRootIndex(_fileSysModel->setRootPath(currentPath));
  return unclassifyResult;
}

bool FileExplorerEvent::on_RemoveDuplicateImages() {
  const auto vt = _contentPane->GetVt();
  if (!ViewTypeTool::isFSView(vt)) {
    LOG_WARN_NP("[Abort] Current View type not support RemoveDuplicateImages", ViewTypeTool::c_str(vt));
    return false;
  }

  const QString& currentPath = _fileSysModel->rootPath();
  if (PathTool::isRootOrEmpty(currentPath)) {
    LOG_WARN_NP("[Abort] Path root or empty", currentPath);
    return false;
  }
  if (QMessageBox::question(_contentPane, "Confirm remove duplicate images?", "Images that differ in resolution will be delete") != QMessageBox::StandardButton::Yes) {
    LOG_OK_NP("[Skip] User Cancel remove duplicate images", "return");
    return false;
  }
  int removedCnt = LowResImgsRemover()(currentPath);
  LOG_OK_P("Remove duplicate image(s) Finished", "count: %d", removedCnt);
  return true;
}

bool FileExplorerEvent::on_RemoveRedundantItem(RedundantRmv& remover) {
  const auto vt = _contentPane->GetVt();
  if (!ViewTypeTool::isFSView(vt)) {
    LOG_WARN_NP("[Abort] Current View type not support RemoveRedundantItem", ViewTypeTool::c_str(vt));
    return false;
  }

  const QString& currentPath = _fileSysModel->rootPath();
  if (PathTool::isRootOrEmpty(currentPath)) {
    LOG_ERR_NP("Path is root or empty", currentPath);
    return false;
  }

  int cmdCnt = remover(currentPath);
  if (cmdCnt == 0) {
    LOG_OK_NP("Skip", "Nothing to remove");
    return true;
  }
  QMessageBox msgBox;
  msgBox.setIcon(QMessageBox::Icon::Warning);
  msgBox.setWindowTitle(QString("Confirm %1 command(s)?").arg(cmdCnt));
  msgBox.setText("Remove Redundant folder");
  msgBox.setStandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
  msgBox.setDefaultButton(QMessageBox::StandardButton::No);
  msgBox.setDetailedText(QString(remover));
  if (msgBox.exec() != QMessageBox::StandardButton::Yes) {
    LOG_INFO_NP("[Skip] User cancel remove redundant item", "return");
    return true;
  }
  bool rmvResult = remover.Exec();
  if (!rmvResult) {
    LOG_ERR_NP("[Failed] Partial Remove Redundant folder failed", currentPath);
    return false;
  }
  LOG_OK_NP("[Ok] All Remove Redundant folder succeed", currentPath);
  return true;
}

void FileExplorerEvent::on_DUPLICATE_VIDEOS_FINDER(const bool checked) {
  m_duplicateVideosFinder = PopupHideWidget<DuplicateVideosFinder>(m_duplicateVideosFinder, checked, _contentPane);  //
  if (checked) {                                                                                                     //
    (*m_duplicateVideosFinder)(_contentPane->getRootPath());                                                         //
  }
}

void FileExplorerEvent::on_DUPLICATE_IMAGES_FINDER(const bool checked) {
  m_redundantImageFinder = PopupHideWidget<RedundantImageFinder>(m_redundantImageFinder, checked, _contentPane);  //
  if (checked) {                                                                                                  //
    (*m_redundantImageFinder)(_contentPane->getRootPath());                                                       //
  }
}

bool FileExplorerEvent::on_MoveCopyEventSkeleton(const Qt::DropAction& dropAct, QString dest) {
  const auto vt = _contentPane->GetVt();
  if (!ViewTypeTool::isFSView(vt)) {
    LOG_WARN_NP("[Abort] Current View type not support Move/Copy to", ViewTypeTool::c_str(vt));
    return false;
  }
  static const QMap<Qt::DropAction, QString> DROP_ACTION_2_STR{{Qt::DropAction::CopyAction, "COPY"}, {Qt::DropAction::MoveAction, "MOVE"}, {Qt::DropAction::IgnoreAction, "IGNORE"}};
  const QString& pOperationNameStr = DROP_ACTION_2_STR.value(dropAct, "IGNORE");
  auto* view = _contentPane->GetCurView();
  if (!view->selectionModel()->hasSelection()) {
    LOG_INFO_NP("Select Some File/Folder First", pOperationNameStr);
    return true;
  }
  const QString& l = _fileSysModel->rootPath();
  if (dest.isEmpty()) {
    dest = ChooseCopyDestination(l, view);
  }
  if (dest.isEmpty() || !QFileInfo{dest}.isDir()) {
    LOG_WARN_NP("Destination path invalid", dest);
    return true;
  }

  Qt::DropAction dropAction{Qt::DropAction::IgnoreAction};
  if (dropAct == Qt::DropAction::MoveAction) {
    Configuration().setValue(MemoryKey::MOVE_TO_PATH_HISTORY.name,  //
                             MoveToNewPathAutoUpdateActionText(dest, g_fileBasicOperationsActions().MOVE_TO_PATH_HISTORY));
  } else if (dropAct == Qt::DropAction::CopyAction) {
    Configuration().setValue(MemoryKey::COPY_TO_PATH_HISTORY.name,  //
                             MoveToNewPathAutoUpdateActionText(dest, g_fileBasicOperationsActions().COPY_TO_PATH_HISTORY));
  } else {
    LOG_D("DropAction[%d] is invalid", static_cast<int>(dropAct));
    return false;
  }

  QStringList lRels;
  for (const QModelIndex& ind : view->selectionModel()->selectedRows()) {
    lRels.append(_fileSysModel->filePath(ind));
  }
  using namespace ComplexOperation;
  int ret = DoDropAction(dropAction, lRels, dest, FILE_STRUCTURE_MODE::PRESERVE);
  if (ret < 0) {
    LOG_ERR_P(pOperationNameStr, "Failed. errorCode:%d", ret);
    return false;
  }
  LOG_OK_NP(pOperationNameStr, "All succeed");
  return true;
}

void FileExplorerEvent::on_RMV_FOLDER_BY_KEYWORD() {
  const QString& keyword = QInputDialog::getItem(_contentPane, "Input keyword here", "filter", {"Marvil Films", "Fox"});
  if (keyword.size() < 3) {
    QMessageBox::warning(_contentPane, "Ignore", "keyword too short:" + keyword);
    return;
  }
  FolderNameContainKeyRmv rirbk{keyword};
  FileExplorerEvent::on_RemoveRedundantItem(rirbk);
}

bool FileExplorerEvent::QueryKeepStructureOrFlatten(ComplexOperation::FILE_STRUCTURE_MODE& mode) {
  auto* msgBox = new (std::nothrow) QMessageBox(QMessageBox::Icon::Information, QString("Keep or Flatten System Structure?"), "Keep file system structure or flatten");
  msgBox->setWindowIcon(QIcon(":img/PASTE_ITEM"));
  msgBox->setStandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::Cancel | QMessageBox::StandardButton::Apply);

  msgBox->button(QMessageBox::StandardButton::Apply)->setText("Keep");
  msgBox->button(QMessageBox::StandardButton::Apply)->setIcon(QIcon(":img/FILE_STRUCTURE_PRESERVE"));
  msgBox->button(QMessageBox::StandardButton::Apply)->setStyleSheet(StyleSheet::SUBMIT_BTN_STYLE);

  msgBox->button(QMessageBox::StandardButton::Yes)->setText("Flatten");
  msgBox->button(QMessageBox::StandardButton::Yes)->setIcon(QIcon(":img/FILE_STRUCTURE_FLATTEN"));

  msgBox->setDefaultButton(QMessageBox::StandardButton::Apply);

  const auto ret = msgBox->exec();
  switch (ret) {
    case QMessageBox::StandardButton::Apply: {
      mode = ComplexOperation::FILE_STRUCTURE_MODE::PRESERVE;
      break;
    }
    case QMessageBox::StandardButton::Yes: {
      mode = ComplexOperation::FILE_STRUCTURE_MODE::FLATTEN;
      break;
    }
    default:
      return false;
  }
  return true;
}
