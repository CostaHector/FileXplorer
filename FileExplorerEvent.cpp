#include "FileExplorerEvent.h"
#include <QApplication>
#include <QInputDialog>
#include <QMessageBox>

#include "Actions/ArchiveFilesActions.h"
#include "Actions/ArrangeActions.h"
#include "Actions/FileBasicOperationsActions.h"
#include "Actions/FileLeafAction.h"
#include "Actions/RenameActions.h"
#include "Actions/RightClickMenuActions.h"
#include "Actions/SyncFileSystemModificationActions.h"
#include "Actions/VideoPlayerActions.h"
#include "Actions/ViewActions.h"
#include "Actions/ThumbnailProcessActions.h"

#include "Component/AlertSystem.h"
#include "Component/Archiver.h"
#include "Component/ConflictsRecycle.h"
#include "Component/CustomStatusBar.h"
#include "Component/MD5Window.h"
#include "Component/Notificator.h"
#include "Component/PropertiesWindow.h"
#include "Component/RedundantImageFinder.h"
#include "Component/SyncModifiyFileSystem.h"
#include "Component/VideoPlayer.h"
#include "Component/ContentPanel.h"
#include "Component/RenameWidgets/AdvanceRenamer.h"
#include "Component/RenameWidgets/RenameWidget_LongPath.h"
#include "Component/RenameWidgets/RenameWidget_ArrangeSection.h"
#include "Component/RenameWidgets/RenameWidget_ConsecutiveFileNo.h"
#include "Component/RenameWidgets/RenameWidget_ConvertBoldUnicodeCharset2Ascii.h"
#include "Component/RenameWidgets/RenameWidget_Case.h"
#include "Component/RenameWidgets/RenameWidget_Insert.h"
#include "Component/RenameWidgets/RenameWidget_Numerize.h"
#include "Component/RenameWidgets/RenameWidget_Replace.h"
#include "Component/RenameWidgets/RenameWidget_ReverseNames.h"

#include "View/DuplicateVideosFinder.h"
#include "View/HarTableview.h"

#include "Tools/ExtractPileItemsOutFolder.h"
#include "Tools/ArchiveFiles.h"
#include "Tools/Classify/ItemsClassifier.h"
#include "Tools/CopyItemPropertiesToClipboardIF.h"
#include "Tools/LowResImgsRemover.h"
#include "Tools/FilesNameBatchStandardizer.h"
#include "Tools/Json/JsonHelper.h"
#include "Tools/MimeDataCX.h"
#include "Tools/PlayVideo.h"
#include "Tools/SysTerminal.h"
#include "Tools/ViewSelection.h"
#include "Tools/ViewTypeTool.h"
#include "Tools/ThumbnailProcesser.h"

#include "public/OnCheckedPopupOrHideAWidget.h"
#include "public/PublicTool.h"
#include "public/MemoryKey.h"
#include "public/UndoRedo.h"

using namespace ViewTypeTool;

FileExplorerEvent* FileExplorerEvent::GetFileExlorerEvent(MyQFileSystemModel* fsm, ContentPanel* view, CustomStatusBar* logger, QObject* parent) {
  if (fsm == nullptr) {
    qWarning() << "MyQFileSystemModel* is nullptr";
    return nullptr;
  }
  if (view == nullptr) {
    qWarning() << "ContentPanel* is nullptr";
    return nullptr;
  }
  if (logger == nullptr) {
    qWarning() << "CustomStatusBar* is nullptr";
    return nullptr;
  }
  static FileExplorerEvent eve(fsm, view, logger, parent);
  eve.subscribe();
  return &eve;
}

FileExplorerEvent::FileExplorerEvent(MyQFileSystemModel* fsm, ContentPanel* view, CustomStatusBar* logger, QObject* parent)
    : QObject(parent), _fileSysModel(fsm), _contentPane(view), _logger(logger), m_clipboard(new MyClipboard(this)) {}

auto FileExplorerEvent::on_NewTextFile(QString newTextName, const QString& contents) -> bool {  // not effect by selection;
  if (not __CanNewItem()) {
    return false;
  }
  if (newTextName.isEmpty()) {
    newTextName = QString("New Text Document %1.txt").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
  }
  if (_fileSysModel->rootDirectory().exists(newTextName)) {
    qInfo("Skipped. There is already a file[%s] in folder[%s].", qPrintable(newTextName), qPrintable(_fileSysModel->rootPath()));
    return false;
  }
  using namespace FileOperatorType;
  BATCH_COMMAND_LIST_TYPE cmds{ACMD{TOUCH, {_fileSysModel->rootPath(), newTextName}}};
  const auto isAllSucceed = g_undoRedo.Do(cmds);
  if (!isAllSucceed) {
    qWarning("[Error] touch command failed when create plain text file.");
    return false;
  }
  const QString& txtFilePath = _fileSysModel->rootDirectory().absoluteFilePath(newTextName);
  if (not contents.isEmpty()) {
    QFile file(txtFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QTextStream stream(&file);
      stream << contents;
    }
    file.close();
  }
  __FocusNewItem(txtFilePath);
  return isAllSucceed;
}
auto FileExplorerEvent::on_NewJsonFile() -> bool {  // not effect by selection;
  if (not __CanNewItem()) {
    return false;
  }
  QStringList jsonFilesNameCreated;
  QStringList jsonFileAlreadyExist;
  const QString& path = _fileSysModel->rootPath();
  for (const QString& fileItem : selectedItems()) {
    QString jsonBaseName, ext;
    std::tie(jsonBaseName, ext) = PathTool::GetBaseNameExt(fileItem);
    const QString jsonAbsPath = path + '/' + jsonBaseName + ".json";
    const QFile fi{jsonAbsPath};
    if (fi.exists()) {
      jsonFileAlreadyExist << jsonAbsPath;
      continue;
    }
    const auto& dict = JsonKey::GetJsonDictDefault(jsonBaseName);
    if (JsonHelper::DumpJsonDict(dict, jsonAbsPath)) {
      jsonFilesNameCreated << jsonBaseName;
    }
  }
  if (!jsonFileAlreadyExist.isEmpty()) {
    qDebug("Json already exists, skip these %d item(s): \n%s", jsonFileAlreadyExist.size(), qPrintable(jsonFileAlreadyExist.join('\n')));
    Notificator::information(QString("Json already exists, skip these %1 item(s)").arg(jsonFileAlreadyExist.size()), jsonFileAlreadyExist.join('\n'));
  }
  if (!jsonFilesNameCreated.isEmpty()) {
    qDebug("Create %d json files succeed, json file name as follows:\n%s", jsonFilesNameCreated.size(), qPrintable(jsonFilesNameCreated.join('\n')));
    Notificator::goodNews(QString("Create %1 json files succeed").arg(jsonFilesNameCreated.size()), jsonFilesNameCreated.join('\n'));
  } else {
    qDebug("No jsons need created under path[%s]", qPrintable(path));
    Notificator::goodNews("No jsons need created under path", path);
  }
  return true;
}
auto FileExplorerEvent::on_NewFolder() -> bool {  // not effect by selection;
  if (not __CanNewItem()) {
    return false;
  }
  const QString& newFolderName = QString("New Folder %1").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
  if (_fileSysModel->rootDirectory().exists(newFolderName)) {
    qInfo("Skip. There is already a folder[%s] in folder[%s].", qPrintable(newFolderName), qPrintable(_fileSysModel->rootPath()));
    return false;
  }
  using namespace FileOperatorType;
  BATCH_COMMAND_LIST_TYPE cmds{ACMD{MKPATH, {_fileSysModel->rootPath(), newFolderName}}};
  const auto isAllSucceed = g_undoRedo.Do(cmds);
  if (!isAllSucceed) {
    qWarning("[Error] Make path failed[%s/%s]", qPrintable(_fileSysModel->rootPath()), qPrintable(newFolderName));
    return false;
  }
  const QString& folderPath = _fileSysModel->rootDirectory().absoluteFilePath(newFolderName);
  __FocusNewItem(folderPath);
  return isAllSucceed;
}
bool FileExplorerEvent::on_BatchNewFilesOrFolders(const char* namePattern, int startIndex, int endIndex, bool isFolder) {
  if (not __CanNewItem()) {
    return false;
  }
  const QDir createInDir = _fileSysModel->rootDirectory();
  const QString& createInPath = _fileSysModel->rootPath();
  using namespace FileOperatorType;
  FileOperatorType::BATCH_COMMAND_LIST_TYPE cmds;

  char fileNameArray[256] = "\0";
  for (int itemIndex = startIndex; itemIndex < endIndex; ++itemIndex) {
    memset(fileNameArray, '\0', sizeof(fileNameArray));
    sprintf(fileNameArray, namePattern, itemIndex);
    if (createInDir.exists(fileNameArray)) {
      qInfo("Skip. File/Folder[%s] already exists in folder[%s]", fileNameArray, qPrintable(createInPath));
      continue;
    }
    cmds.append(ACMD{isFolder ? MKPATH : TOUCH, {createInPath, fileNameArray}});
  }
  const auto isAllSucceed = g_undoRedo.Do(cmds);
  if (not isAllSucceed) {
    qWarning("[Error] Some commands failed when create %d file/folder(s).", cmds.size());
    return false;
  }
  return true;
}

bool FileExplorerEvent::on_BatchNewFilesOrFolders(bool isFolder) {
  const QString defNamePattern = isFolder ? PreferenceSettings().value(MemoryKey::NAME_PATTERN_USED_CREATE_BATCH_FOLDERS.name, MemoryKey::NAME_PATTERN_USED_CREATE_BATCH_FOLDERS.v).toString()
                                          : PreferenceSettings().value(MemoryKey::NAME_PATTERN_USED_CREATE_BATCH_FILES.name, MemoryKey::NAME_PATTERN_USED_CREATE_BATCH_FILES.v).toString();

  const QString userInputRule =
      QInputDialog::getText(_contentPane, QString("Create Batch %1").arg(isFolder ? "Folders" : "Files"), "Rule Pattern: C-styleFormatString%StartIndex$EndIndex", QLineEdit::Normal, defNamePattern);
  const QStringList& userInputLst = userInputRule.split('$');
  if (userInputLst.size() != 3 or userInputLst.size() >= 280) {
    qWarning("[Error] Invalid Rule pattern[%s]. It must contains exactly 3 parts but now %d. and must below 280 letter but now %d", qPrintable(userInputRule), userInputLst.size(),
             userInputRule.size());
    Notificator::warning(QString("Invalid Rule pattern[%1]").arg(userInputRule), QString("3 parts[%1] and below 280 letters[%2]").arg(userInputLst.size()).arg(userInputRule.size()));
    return false;
  }
  char namePattern[280] = "\0";
  strncpy(namePattern, userInputLst[0].toStdString().c_str(), sizeof(namePattern));
  int startIndex = userInputLst[1].toInt();
  int endIndex = userInputLst[2].toInt();
  if (startIndex >= endIndex) {
    qInfo("Skip Create %d file/folders", endIndex - startIndex);
    Notificator::information(QString("Skip").arg(userInputRule), QString("%1 file/folders").arg(endIndex - startIndex));
    return true;
  }
  PreferenceSettings().setValue(isFolder ? MemoryKey::NAME_PATTERN_USED_CREATE_BATCH_FOLDERS.name : MemoryKey::NAME_PATTERN_USED_CREATE_BATCH_FILES.name, userInputRule);

  return on_BatchNewFilesOrFolders(namePattern, startIndex, endIndex, isFolder);
}

bool FileExplorerEvent::on_ExtractImagesFromThumbnail(int beg, int end, bool skipIfExist) {
  if (!__CanNewItem()) {
    return false;
  }
  const QString currentPath = _fileSysModel->rootPath();
  if (currentPath.count('/') < 3) {
    auto* msgBox = new QMessageBox(QMessageBox::Icon::Question, "Extract images out of thumbnail Confirm (lag may cause)?", QString("All item(s) under [%1] will be unpile out!").arg(currentPath));
    msgBox->setWindowIcon(QIcon(":img/THUMBNAIL_EXTRACTOR_B_E"));
    msgBox->setInformativeText(QString("Work path [%1]?").arg(currentPath));
    msgBox->setDetailedText(QString("path:[%1].\nOperation Recoverable.").arg(currentPath));
    msgBox->setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
    msgBox->setDefaultButton(QMessageBox::Ok);
    const auto ret = msgBox->exec();
    if (ret != QMessageBox::Ok) {
      qInfo("User cancel. Skip extract images out of thumbnail");
      Notificator::information("User cancel", "Skip extract images out of thumbnail");
      return true;
    }
  }
  ThumbnailProcesser tp{skipIfExist};
  int extractedOutCnt = tp(currentPath, beg, end);
  if (tp.mErrImg.isEmpty()) {
    if (extractedOutCnt == 0) {
      qDebug("Nothing need extracted out in [%s]", qPrintable(currentPath));
      Notificator::goodNews("Nothing need extracted out", currentPath);
    } else {
      qDebug("Extract %d from [%s] all succeed", extractedOutCnt, qPrintable(currentPath));
      Notificator::goodNews(QString("Extract %1 from [%2]").arg(extractedOutCnt).arg(currentPath), "all succeed");
    }
  } else {
    qWarning("extract %d images out from %s. error msg: %s", extractedOutCnt, qPrintable(currentPath), qPrintable(tp.mErrImg.join('\n')));
    Notificator::information(QString("extract %1 images out from %2. error msg:").arg(extractedOutCnt).arg(currentPath), tp.mErrImg.join('\n'));
  }
  return true;
}

QModelIndexList FileExplorerEvent::selectedIndexes() const {
  // ignore other column, keep the first column
  return ViewSelection::selectedIndexes(_contentPane->GetCurView());
}

bool FileExplorerEvent::on_searchKeywordInSystemDefaultExplorer() const {
  const QString& absFilePath = _contentPane->getCurFilePath();
  const QString& noExtAbsFilePath = PathTool::GetFileNameExtRemoved(absFilePath);
  const QString& imgFileAbsPathGuess = QDir::toNativeSeparators(noExtAbsFilePath) + ' ';
  QApplication::clipboard()->setText(imgFileAbsPathGuess, QClipboard::Mode::Clipboard);
  QString fileBaseName = PathTool::GetBaseName(absFilePath);
  const QString& forSearch = fileBaseName.replace(JSON_RENAME_REGEX::INVALID_TABLE_NAME_LETTER, " ");
  QStringList searchKeyWordArgs;
  if (!forSearch.isEmpty()) {
    const static QString HTML_URL_TEMPLATE{"https://www.google.com/search?&q=%1&udm=2"};
    searchKeyWordArgs << HTML_URL_TEMPLATE.arg(forSearch);
  } else {
    searchKeyWordArgs << "https://www.google.com/";
  }

  QProcess process;
#ifdef _WIN32
  process.setProgram("explorer.exe");
#else
  process.setProgram("xdg-open");
#endif
  process.setArguments(searchKeyWordArgs);
  process.startDetached();
  qWarning("Search in default file system net explorer using program[%s] Parms[%s].", qPrintable(process.program()), qPrintable(searchKeyWordArgs.join(',')));
  return true;
}

bool FileExplorerEvent::on_calcMD5() const {
  if (!_contentPane->isFSView()) {
    qWarning("MD5 only support on File System Model");
    return false;
  }
  const QStringList& items = selectedItems();
  if (items.empty()) {
    qDebug("Nothing selected, no need calc MD5");
    return false;
  }
  auto* md5W = new MD5Window(_fileSysModel->rootPath(), items, this->_contentPane);
  md5W->show();
  return true;
}

bool FileExplorerEvent::on_properties() const {
  PropertiesWindow* pW = nullptr;
  if (_contentPane->isFSView()) {
    const QStringList& items = selectedItems();
    pW = new PropertiesWindow(this->_contentPane);
    pW->show();
    pW->operator()(items);
    return true;
  }
  if (_contentPane->GetCurViewType() == ViewType::MOVIE) {
    pW = new PropertiesWindow(this->_contentPane);
    pW->show();
    pW->operator()(_contentPane->m_dbModel, _contentPane->m_movieView);
    return true;
  }
  qDebug("Reject. not support in view[%s]", qPrintable(_contentPane->GetCurViewName()));
  Notificator::information("Reject", QString("Not support in view[%s]").arg(_contentPane->GetCurViewName()));
  return false;
}

void FileExplorerEvent::subsribeCompress() {
  connect(g_AchiveFilesActions().COMPRESSED_HERE, &QAction::triggered, this, &FileExplorerEvent::on_compress);
  connect(g_AchiveFilesActions().DECOMPRESSED_HERE, &QAction::triggered, this, &FileExplorerEvent::on_deCompress);
  connect(g_AchiveFilesActions().COMPRESSED_IMAGES, &QAction::triggered, this, &FileExplorerEvent::on_compressImgsByGroup);
  connect(g_AchiveFilesActions().ARCHIVE_PREVIEW, &QAction::triggered, this, &FileExplorerEvent::on_archivePreview);
}

void FileExplorerEvent::subsribeFileActions() {
  connect(g_fileLeafActions()._ALERT_ITEMS, &QAction::triggered, this,                           //
          [this](const bool checked) {                                                           //
            m_alertSystem = PopupHideWidget<AlertSystem>(m_alertSystem, checked, _contentPane);  //
            if (checked) {                                                                       //
              (*m_alertSystem)(_contentPane->getRootPath());                                     //
            }                                                                                    //
          });

  connect(g_fileLeafActions()._ABOUT_FILE_EXPLORER, &QAction::triggered, this, [this]() {
    QMessageBox::about(_contentPane, "FileExplorer",
                       "Version: 46.0\n"
                       "Introduction: A minimalism app for image/video/json/folder explorer\n"
                       "Platform-supported: Linux and Win");
  });

  connect(g_fileLeafActions()._LANUAGE, &QAction::triggered, this, [](const bool cnEnabled) {
    PreferenceSettings().setValue(MemoryKey::LANGUAGE_ZH_CN.name, cnEnabled);
    qDebug("Language will be changed next time open");
    Notificator::information("Language switch", "work after reopen");
  });
}

void FileExplorerEvent::subscribeThumbnailActions() {
  auto& ins = g_ThumbnailProcessActions();
  connect(ins._EXTRACT_1ST_IMG, &QAction::triggered, this, [this, &ins]() {
    bool bSkipExist = ins._SKIP_IF_ALREADY_EXIST->isChecked();
    on_ExtractImagesFromThumbnail(0, 1, bSkipExist);
  });
  connect(ins._EXTRACT_2ND_IMGS, &QAction::triggered, this, [this, &ins]() {
    bool bSkipExist = ins._SKIP_IF_ALREADY_EXIST->isChecked();
    on_ExtractImagesFromThumbnail(0, 2, bSkipExist);
  });
  connect(ins._EXTRACT_4TH_IMGS, &QAction::triggered, this, [this, &ins]() {
    bool bSkipExist = ins._SKIP_IF_ALREADY_EXIST->isChecked();
    on_ExtractImagesFromThumbnail(0, 4, bSkipExist);
  });
  connect(ins._CUSTOM_RANGE_IMGS, &QAction::triggered, this, [this, &ins]() {
    bool bSkipExist = ins._SKIP_IF_ALREADY_EXIST->isChecked();
    bool bok = false;
    int beg = QInputDialog::getInt(this->_contentPane, "Extract image start index", "beg=", 0, 0, 100, 1, &bok);
    if (!bok) {
      qWarning("User input image start index invalid");
      return;
    }
    int end = QInputDialog::getInt(this->_contentPane, "Extract image end index", "end=", 9, beg, 9, 1, &bok);
    if (!bok) {
      qWarning("User input image end index invalid");
      return;
    }
    on_ExtractImagesFromThumbnail(beg, end, bSkipExist);
  });
}

void FileExplorerEvent::subscribe() {
  subsribeCompress();
  subsribeFileActions();
  subscribeThumbnailActions();
  {
    auto* NEW_FOLDER = g_fileBasicOperationsActions().NEW_FOLDER;
    auto* NEW_TEXT_FILE = g_fileBasicOperationsActions().NEW_TEXT_FILE;
    auto* NEW_JSON_FILE = g_fileBasicOperationsActions().NEW_JSON_FILE;
    auto* BATCH_NEW_FILES = g_fileBasicOperationsActions().BATCH_NEW_FILES;
    auto* BATCH_NEW_FOLDERS = g_fileBasicOperationsActions().BATCH_NEW_FOLDERS;
    connect(NEW_FOLDER, &QAction::triggered, this, &FileExplorerEvent::on_NewFolder);
    connect(NEW_TEXT_FILE, &QAction::triggered, this, [this]() { this->on_NewTextFile(); });
    connect(NEW_JSON_FILE, &QAction::triggered, this, &FileExplorerEvent::on_NewJsonFile);
    connect(BATCH_NEW_FILES, &QAction::triggered, this, [this]() { FileExplorerEvent::on_BatchNewFilesOrFolders(false); });
    connect(BATCH_NEW_FOLDERS, &QAction::triggered, this, [this]() { FileExplorerEvent::on_BatchNewFilesOrFolders(true); });
  }

  {
    auto* _PLAY_VIDEOS = g_viewActions()._SYS_VIDEO_PLAYERS;
    auto* _REVEAL_IN_EXPLORER = g_fileBasicOperationsActions()._REVEAL_IN_EXPLORER;
    auto* _OPEN_IN_TERMINAL = g_fileBasicOperationsActions()._OPEN_IN_TERMINAL;
    connect(_PLAY_VIDEOS, &QAction::triggered, this, &FileExplorerEvent::on_PlayVideo);
    connect(_REVEAL_IN_EXPLORER, &QAction::triggered, this, &FileExplorerEvent::on_revealInExplorer);
    connect(_OPEN_IN_TERMINAL, &QAction::triggered, this, &FileExplorerEvent::on_OpenInTerminal);
  }
  {
    auto* UNDO_OPERATION = g_fileBasicOperationsActions().UNDO_OPERATION;
    auto* REDO_OPERATION = g_fileBasicOperationsActions().REDO_OPERATION;
    connect(UNDO_OPERATION, &QAction::triggered, this, &UndoRedo::on_Undo);
    connect(REDO_OPERATION, &QAction::triggered, this, &UndoRedo::on_Redo);
  }
  {
    auto* COPY_FULL_PATH = g_fileBasicOperationsActions().COPY_FULL_PATH;
    auto* COPY_PATH = g_fileBasicOperationsActions().COPY_PATH;
    auto* COPY_NAME = g_fileBasicOperationsActions().COPY_NAME;
    auto* COPY_THE_PATH = g_fileBasicOperationsActions().COPY_THE_PATH;
    auto* COPY_RECORDS = g_fileBasicOperationsActions().COPY_RECORDS;
    connect(COPY_FULL_PATH, &QAction::triggered, _contentPane, [this]() { CopyItemPropertiesToClipboardIF::PathCopyTriple(_contentPane->getFilePaths(), "absolute-file-path"); });
    connect(COPY_PATH, &QAction::triggered, _contentPane, [this]() { CopyItemPropertiesToClipboardIF::PathCopyTriple(_contentPane->getFilePrepaths(), "absolute-path"); });
    connect(COPY_NAME, &QAction::triggered, _contentPane, [this]() { CopyItemPropertiesToClipboardIF::PathCopyTriple(_contentPane->getFileNames(), "file-name"); });
    connect(COPY_THE_PATH, &QAction::triggered, _contentPane,
            [this]() { CopyItemPropertiesToClipboardIF::PathCopyTriple(_contentPane->getTheJpgFolderPaths(), "absolute-file-path+folderName+.jpg(in local seperator)"); });
    connect(COPY_RECORDS, &QAction::triggered, _contentPane, [this]() { CopyItemPropertiesToClipboardIF::PathCopyTriple(_contentPane->getFullRecords(), "full-record"); });
  }

  {
    auto* MOVE_TO_TRASHBIN = g_fileBasicOperationsActions().MOVE_TO_TRASHBIN;
    auto* DELETE_PERMANENTLY = g_fileBasicOperationsActions().DELETE_PERMANENTLY;
    connect(MOVE_TO_TRASHBIN, &QAction::triggered, this, &FileExplorerEvent::on_moveToTrashBin);
    connect(DELETE_PERMANENTLY, &QAction::triggered, this, &FileExplorerEvent::on_deletePermanently);
  }

  {
    QList<QAction*> FOLDER_MERGEList = g_fileBasicOperationsActions().FOLDER_MERGE->actions();
    auto* MERGE = FOLDER_MERGEList[0];
    auto* MERGE_REVERSE = FOLDER_MERGEList[1];
    connect(MERGE, &QAction::triggered, this, [this]() { on_Merge(false); });
    connect(MERGE_REVERSE, &QAction::triggered, this, [this]() { on_Merge(true); });
  }

  {
    auto* MOVE_TO = g_fileBasicOperationsActions()._MOVE_TO;
    auto* COPY_TO = g_fileBasicOperationsActions()._COPY_TO;

    connect(MOVE_TO, &QAction::triggered, this, [this]() { this->on_MoveTo(); });
    connect(COPY_TO, &QAction::triggered, this, [this]() { this->on_CopyTo(); });

    auto MOVE_TO_PATH_HISTORY = g_fileBasicOperationsActions().MOVE_TO_PATH_HISTORY;
    auto COPY_TO_PATH_HISTORY = g_fileBasicOperationsActions().COPY_TO_PATH_HISTORY;

    connect(MOVE_TO_PATH_HISTORY, &QActionGroup::triggered, this, [this](const QAction* const act) { on_MoveTo(act->text()); });
    connect(COPY_TO_PATH_HISTORY, &QActionGroup::triggered, this, [this](const QAction* const act) { on_CopyTo(act->text()); });
  }

  {
    auto* CUT = g_fileBasicOperationsActions().CUT;
    auto* COPY = g_fileBasicOperationsActions().COPY;
    auto* PASTE = g_fileBasicOperationsActions().PASTE;

    connect(CUT, &QAction::triggered, this, &FileExplorerEvent::on_Cut);
    connect(COPY, &QAction::triggered, this, &FileExplorerEvent::on_Copy);
    connect(PASTE, &QAction::triggered, this, &FileExplorerEvent::on_Paste);
  }

  {
    auto* _NAME_RULER = g_fileBasicOperationsActions()._NAME_RULER;
    auto* _PACK_FOLDERS = g_fileBasicOperationsActions()._PACK_FOLDERS;
    auto* _UNPACK_FOLDERS = g_fileBasicOperationsActions()._UNPACK_FOLDERS;
    auto* _LOW_RESOLUTION_IMGS_RMV = g_fileBasicOperationsActions()._LOW_RESOLUTION_IMGS_RMV;
    auto* _RMV_01_FILE_FOLDER = g_fileBasicOperationsActions()._RMV_01_FILE_FOLDER;
    auto* _REMOVE_EMPTY_FOLDER = g_fileBasicOperationsActions()._RMV_EMPTY_FOLDER_R;
    auto* _REMOVE_FOLDER_BY_KEYWORD = g_fileBasicOperationsActions()._RMV_FOLDER_BY_KEYWORD;
    auto* _DUPLICATE_VIDEOS_FINDER = g_fileBasicOperationsActions()._DUPLICATE_VIDEOS_FINDER;
    auto* _DUPLICATE_IMAGES_FINDER = g_fileBasicOperationsActions()._DUPLICATE_IMAGES_FINDER;
    connect(_NAME_RULER, &QAction::triggered, this, &FileExplorerEvent::on_NameStandardize);
    connect(_PACK_FOLDERS, &QAction::triggered, this, &FileExplorerEvent::on_FileClassify);
    connect(_UNPACK_FOLDERS, &QAction::triggered, this, &FileExplorerEvent::on_FileUnclassify);
    connect(_LOW_RESOLUTION_IMGS_RMV, &QAction::triggered, this, &FileExplorerEvent::on_RemoveDuplicateImages);
    connect(_RMV_01_FILE_FOLDER, &QAction::triggered, this, [this]() {
      ZeroOrOneItemFolderProc rfr;
      FileExplorerEvent::on_RemoveRedundantItem(rfr);
    });
    connect(_REMOVE_EMPTY_FOLDER, &QAction::triggered, this, [this]() {
      EmptyFolderRmv efr;
      FileExplorerEvent::on_RemoveRedundantItem(efr);
    });
    connect(_REMOVE_FOLDER_BY_KEYWORD, &QAction::triggered, this, [this]() {
      const QString& keyword = QInputDialog::getItem(_contentPane, "Input keyword here", "filter", {"Marvil Films", "Fox"});
      if (keyword.size() < 3) {
        QMessageBox::warning(_contentPane, "Ignore", "keyword too short:" + keyword);
        return;
      }
      FolderNameContainKeyRmv rirbk{keyword};
      FileExplorerEvent::on_RemoveRedundantItem(rirbk);
    });
    connect(_DUPLICATE_VIDEOS_FINDER, &QAction::triggered, this,                                                                 //
            [this](const bool checked) {                                                                                         //
              m_duplicateVideosFinder = PopupHideWidget<DuplicateVideosFinder>(m_duplicateVideosFinder, checked, _contentPane);  //
              if (checked) {                                                                                                     //
                (*m_duplicateVideosFinder)(_contentPane->getRootPath());                                                         //
              }
            });
    connect(_DUPLICATE_IMAGES_FINDER, &QAction::triggered, this,                                                              //
            [this](const bool checked) {                                                                                      //
              m_redundantImageFinder = PopupHideWidget<RedundantImageFinder>(m_redundantImageFinder, checked, _contentPane);  //
              if (checked) {                                                                                                  //
                (*m_redundantImageFinder)(_contentPane->getRootPath());                                                       //
              }
            });
  }

  {
    connect(g_rightClickActions()._SEARCH_IN_NET_EXPLORER, &QAction::triggered, this, &FileExplorerEvent::on_searchKeywordInSystemDefaultExplorer);
    connect(g_rightClickActions()._CALC_MD5_ACT, &QAction::triggered, this, &FileExplorerEvent::on_calcMD5);
    connect(g_rightClickActions()._PROPERTIES, &QAction::triggered, this, &FileExplorerEvent::on_properties);
    connect(g_rightClickActions()._FORCE_REFRESH_FILESYSTEMMODEL, &QAction::triggered, this, &FileExplorerEvent::on_forceRefreshFileSystemModel);
  }

  {
    auto* SELECT_ALL = g_fileBasicOperationsActions().SELECTION_RIBBONS->actions()[0];
    auto* SELECT_NONE = g_fileBasicOperationsActions().SELECTION_RIBBONS->actions()[1];
    auto* SELECT_INVERT = g_fileBasicOperationsActions().SELECTION_RIBBONS->actions()[2];

    connect(SELECT_ALL, &QAction::triggered, this, &FileExplorerEvent::on_SelectAll);
    connect(SELECT_NONE, &QAction::triggered, this, &FileExplorerEvent::on_SelectNone);
    connect(SELECT_INVERT, &QAction::triggered, this, &FileExplorerEvent::on_SelectInvert);
  }
  {
    connect(g_renameAg()._NUMERIZER, &QAction::triggered, this, [this]() -> void {
      auto* pNumerize = new RenameWidget_Numerize(_contentPane);
      onRename(pNumerize);
    });
    connect(g_renameAg()._SECTIONS_SWAPPER, &QAction::triggered, this, [this]() -> void {
      auto* pArrange = new RenameWidget_ArrangeSection(_contentPane);
      onRename(pArrange);
    });
    connect(g_renameAg()._SWAP_2_NAMES, &QAction::triggered, this, [this]() -> void {
      auto* pReverse = new RenameWidget_ReverseNames(_contentPane);
      onRename(pReverse);
    });
    connect(g_renameAg()._CASE_NAME, &QAction::triggered, this, [this]() -> void {
      auto* pCase = new RenameWidget_Case(_contentPane);
      onRename(pCase);
    });
    connect(g_renameAg()._STR_INSERTER, &QAction::triggered, this, [this]() -> void {
      auto* pInsert = new RenameWidget_Insert(_contentPane);
      onRename(pInsert);
    });
    connect(g_renameAg()._STR_DELETER, &QAction::triggered, this, [this]() -> void {
      auto* pDelete = new RenameWidget_Delete(_contentPane);
      onRename(pDelete);
    });
    connect(g_renameAg()._STR_REPLACER, &QAction::triggered, this, [this]() -> void {
      auto* pReplacer = new RenameWidget_Replace(_contentPane);
      onRename(pReplacer);
    });
    connect(g_renameAg()._CONTINUOUS_NUMBERING, &QAction::triggered, this, [this]() -> void {
      auto* pNoConsecutive = new RenameWidget_ConsecutiveFileNo(_contentPane);
      onRename(pNoConsecutive);
    });
    connect(g_renameAg()._CONVERT_UNICODE_TO_ASCII, &QAction::triggered, this, [this]() -> void {
      auto* pToAscii = new RenameWidget_ConvertBoldUnicodeCharset2Ascii(_contentPane);
      onRename(pToAscii);
    });

    auto* _LONG_PATH_FINDER = g_fileBasicOperationsActions()._LONG_PATH_FINDER;
    connect(_LONG_PATH_FINDER, &QAction::triggered, this, [this]() -> void {
      auto* pToLongPath = new RenameWidget_LongPath(_contentPane);
      onRename(pToLongPath);
    });
  }

  {
    connect(g_viewActions()._VIDEO_PLAYER_EMBEDDED, &QAction::triggered, this, &FileExplorerEvent::on_PlaySelectedItemsInView);
    connect(g_viewActions()._HAR_VIEW, &QAction::triggered, this, &FileExplorerEvent::on_HarView);
  }

  {
    connect(g_videoPlayerActions()._PLAY_SELECTION, &QAction::triggered, this, &FileExplorerEvent::on_PlaySelectedItemsInView);
    connect(g_videoPlayerActions()._PLAY_CURRENT_PATH, &QAction::triggered, this, &FileExplorerEvent::on_PlayCurrentPathOfView);
  }

  {
    auto& syncMod = g_syncFileSystemModificationActions();
    connect(syncMod._SYNC_MOD_SWITCH, &QAction::triggered, this, [&syncMod](const bool sw) {
      if (sw) {
        SyncModifiyFileSystem::LoadFromMemory();
      }
      SyncModifiyFileSystem::m_syncModifyFileSystemSwitch = sw;
      syncMod._BASIC_PATH->setEnabled(sw);
      syncMod._SYNC_TO_PATH->setEnabled(sw);
      syncMod._SYNC_REVERSE_SWITCH->setEnabled(sw);
      PreferenceSettings().setValue("SYNC_FS_MOD", sw);
    });
    connect(syncMod._SYNC_REVERSE_SWITCH, &QAction::triggered, this, [](const bool sw) {
      SyncModifiyFileSystem::m_alsoSyncReversebackSwitch = sw;
      PreferenceSettings().setValue("SYNC_REVERSE_BACK", sw);
    });
    connect(syncMod._BASIC_PATH, &QLineEdit::returnPressed, this, [&syncMod]() {
      SyncModifiyFileSystem::SetBasicPath(syncMod._BASIC_PATH->text());
      PreferenceSettings().setValue("SYNC_BASIC_PATH", SyncModifiyFileSystem::m_basicPath);
    });
    connect(syncMod._SYNC_TO_PATH, &QLineEdit::returnPressed, this, [&syncMod]() {
      SyncModifiyFileSystem::SetSynchronizedToPaths(syncMod._SYNC_TO_PATH->text());
      PreferenceSettings().setValue("SYNC_TO_PATH", SyncModifiyFileSystem::m_synchronizedToPath);
    });
  }

  g_ArrangeActions().subscribe();
}

void FileExplorerEvent::onRename(AdvanceRenamer* renameWid) {
  if (renameWid == nullptr) {
    qCritical("renameWid is nullptr");
    return;
  }
  if (!_contentPane->isFSView()) {
    qWarning("[Rename] only available on FileSystemModel but[%s]", qPrintable(_contentPane->GetCurViewName()));
    return;
  }
  const QString& filePath = _fileSysModel->rootPath();
  const QStringList& preNames = _contentPane->getFileNames();
  if (preNames.isEmpty()) {
    qDebug("nothing item(s) selected, skip rename");
    return;
  }

  const auto beforeOption = _fileSysModel->options();
  if (preNames.size() > 100) {
    _fileSysModel->setOptions(QFileSystemModel::DontWatchForChanges);
  }
  renameWid->init();
  renameWid->show();
  renameWid->InitTextEditContent(filePath, preNames);
  renameWid->exec();

  if (preNames.size() > 100) {
    _fileSysModel->setOptions(beforeOption);
  }
}

auto FileExplorerEvent::__CanNewItem() const -> bool {
  if (!_contentPane->isFSView()) {
    qDebug("Reject. Only new item in file system view[%s]", qPrintable(_contentPane->GetCurViewName()));
    Notificator::information("Reject", QString("Not file system view[%s]").arg(_contentPane->GetCurViewName()));
    return false;
  }
  if (PathTool::isLinuxRootOrWinEmpty(_fileSysModel->rootPath())) {
    qDebug("Reject. Don't create item under path[%s]", qPrintable(_fileSysModel->rootPath()));
    Notificator::information("Reject", QString("Don't create item under path[%s]").arg(_fileSysModel->rootPath()));
    return false;
  }
  return true;
}

bool FileExplorerEvent::__FocusNewItem(const QString& itemPath) {
  if (!_contentPane->isFSView()) {
    return false;
  }
  auto* view = _contentPane->GetCurView();
  const QModelIndex ind = _fileSysModel->index(itemPath);
  if (!ind.isValid()) {
    qDebug("target lost, skip focus new one");
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
  QString reveal_path;
  if (!curIndex.isValid()) {
    args << QDir::toNativeSeparators(_contentPane->getRootPath());
  } else {
    const QFileInfo fi{_contentPane->getFilePath(curIndex)};
    if (!fi.exists()) {
      qWarning("Reveal path[%s] not exists", qPrintable(fi.absoluteFilePath()));
      Notificator::warning("Reveal path[%1] not exists", fi.absoluteFilePath());
      return false;
    }
#ifdef _WIN32
    args << "/e,"
         << "/select,";
#endif
    args << QDir::toNativeSeparators(fi.absoluteFilePath());
  }
  QProcess process;
#ifdef _WIN32
  process.setProgram("explorer.exe");
#else
  process.setProgram("xdg-open");
#endif
  process.setArguments(args);
  process.startDetached();  // Start the process in detached mode instead of start
  qWarning("on_revealInExplorer with program[%s] parms [%s]", qPrintable(process.program()), qPrintable(args.join(',')));
  return true;
}

bool FileExplorerEvent::on_OpenInTerminal() const {
  if (not(_contentPane->isFSView() or _contentPane->GetCurViewName() == ENUM_2_STR(SEARCH))) {
    qWarning("[Open in Terminal] only support in filesystem/search/movie but[%s]", qPrintable(_contentPane->GetCurViewName()));
    Notificator::warning("[Reveal in Terminal] only support in filesystem/search/movie but[%1]", _contentPane->GetCurViewName());
    return false;
  }
  return SysTerminal()(_contentPane->getRootPath());
}

bool FileExplorerEvent::on_forceRefreshFileSystemModel() {
  if (!_contentPane->isFSView()) {
    qWarning("[Refresh] only support in filesystem but[%s]", qPrintable(_contentPane->GetCurViewName()));
    Notificator::warning("[Refresh] only support in filesystem but[%1]", _contentPane->GetCurViewName());
    return false;
  }
  QAbstractItemView* fsView = _contentPane->GetCurView();
  if (fsView == nullptr) {
    return false;
  }
  const QString& path = _fileSysModel->rootPath();
  _fileSysModel->setRootPath("");
  fsView->setRootIndex(_fileSysModel->setRootPath(path));
  qDebug("Refresh filesytemmodel of path: %s", qPrintable(path));
  Notificator::goodNews("Refresh filesytemmodel of path: ", path);
  return true;
}

bool FileExplorerEvent::on_compress() {
  if (not((_contentPane->isFSView() or _contentPane->GetCurViewName() == ENUM_2_STR(SEARCH)) and not PathTool::isLinuxRootOrWinEmpty(_contentPane->getRootPath()))) {
    qInfo("[Compress] Only available on FileSytemView/search[%s] and non-empty-path[%s]", qPrintable(_contentPane->GetCurViewName()), qPrintable(_contentPane->getRootPath()));
    Notificator::information("[Compress] Only available on FileSytemView/search[%1] and non-empty-path[%2]", _contentPane->GetCurViewName() + '|' + _contentPane->getRootPath());
    return false;
  }
  const QStringList& filesPath = _contentPane->getFilePaths();
  if (filesPath.isEmpty()) {
    qDebug("skip nothing selected");
    Notificator::information("[Compress] skip", "nothing selected");
    return false;
  }
  const QString& compressedTo = _contentPane->getRootPath();
  const QString& archieveName = QFileInfo(compressedTo).completeBaseName() + ".qz";
  const QString& archievePath = QDir(compressedTo).absoluteFilePath(archieveName);
  ArchiveFiles af{archievePath, ArchiveFiles::NO_FILTER};
  bool compressedResult = af.CompressNow(ArchiveFiles::OPERATION_TYPE::FILES, filesPath, false);
  Notificator::information("Compressed result bool:", QString::number(compressedResult));
  return compressedResult;
}

bool FileExplorerEvent::on_deCompress() {
  if (!((_contentPane->isFSView() || _contentPane->GetCurViewName() == ENUM_2_STR(SEARCH)) && !PathTool::isLinuxRootOrWinEmpty(_contentPane->getRootPath()))) {
    qInfo("[Decompress] Only available on FileSytemView/search[%s] and non-empty-path[%s]", qPrintable(_contentPane->GetCurViewName()), qPrintable(_contentPane->getRootPath()));
    Notificator::warning("[Decompress] Only available on FileSytemView/search[%1] and non-empty-path[%2]", _contentPane->GetCurViewName() + '|' + _contentPane->getRootPath());
    return false;
  }
  const QStringList& filesPath = _contentPane->getFilePaths();
  if (filesPath.isEmpty()) {
    Notificator::warning("Skip", "Nothing need decompress");
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
  qInfo("decompress %d file(s), %d failed", filesPath.size(), failsQzFiles.size());
  if (failsQzFiles.isEmpty()) {
    Notificator::goodNews("All decompress ok", QString("%1 qz files").arg(filesPath.size()));
  } else {
    qWarning("decompress failed item(s) as following:\n%s", qPrintable(failsQzFiles.join('\n')));
    Notificator::badNews(QString("Decompressed following %1 file(s) failed").arg(failsQzFiles.size()), failsQzFiles.join('\n'));
  }
  return failsQzFiles.isEmpty();
}

bool FileExplorerEvent::on_compressImgsByGroup() {
  if (!_contentPane->isFSView()) {
    qDebug("[Compress images] only available on FileSystemModel but[%s]", qPrintable(_contentPane->GetCurViewName()));
    return false;
  }
  const QString& pth = _contentPane->getRootPath();
  ArchiveImagesRecusive air{true};
  int compressFolderCnt = air.CompressImgRecur(pth);
  Notificator::information("Compress images result bool:", QString::number(compressFolderCnt));
  return true;
}

bool FileExplorerEvent::on_archivePreview() {
  auto vt = _contentPane->GetCurViewType();
  if (!ViewTypeTool::isFSView(vt) && vt != ViewType::SEARCH) {
    return false;
  }
  const QString pth = _contentPane->getRootPath();
  if (PathTool::isLinuxRootOrWinEmpty(pth)) {
    qInfo("[ArchivePreivew] Only available on FileSytemView/search[%c] and non-empty-path[%s]", (char)vt, qPrintable(pth));
    Notificator::warning("[ArchivePreivew] Only available on FileSytemView/search and non-empty-path", QString::number((int)vt) + '|' + pth);
    return false;
  }
  const QStringList& filesPath = _contentPane->getFilePaths();
  if (filesPath.isEmpty() || filesPath.size() > 1) {
    qWarning("Can ArchivePreivew only 1 qz files");
    Notificator::warning("[ArchivePreivew] Can ArchivePreivew only 1 qz files", QString("real: %1 count").arg(filesPath.size()));
    return false;
  }
  if (m_archivePreview == nullptr) {
    m_archivePreview = new Archiver;
  }
  bool previewRet = m_archivePreview->operator()(filesPath.front());
  m_archivePreview->show();
  m_archivePreview->activateWindow();
  m_archivePreview->raise();
  return previewRet;
}

bool FileExplorerEvent::on_moveToTrashBin() {
  auto vt = _contentPane->GetCurViewType();
  if (!ViewTypeTool::isFSView(vt) && vt != ViewType::SEARCH) {
    return false;
  }
  const QString pth = _contentPane->getRootPath();
  if (PathTool::isLinuxRootOrWinEmpty(pth)) {
    qInfo("[Move to trashbin] Only available on FileSytemView/search[%c] and non-empty-path[%s]", (char)vt, qPrintable(pth));
    Notificator::warning("[Move to trashbin] Only available on FileSytemView/search and non-empty-path", QString::number((int)vt) + '|' + pth);
    return false;
  }

  QStringList prepaths, names;
  std::tie(prepaths, names) = _contentPane->getFilePrepathsAndName(true);
  if (!(prepaths.size() == names.size() && names.size() > 0)) {
    qInfo("Skip. Nothing selected or inequal length to move to trashbin");
    Notificator::information("Skip", "Nothing selected or inequal length to move to trashbin");
    return true;
  }
  using namespace FileOperatorType;
  BATCH_COMMAND_LIST_TYPE removeCmds;
  removeCmds.reserve(prepaths.size());
  for (int i = 0; i < prepaths.size(); ++i) {
    removeCmds.append(ACMD{MOVETOTRASH, {prepaths[i], names[i]}});
  }

  const bool isAllSucceed = g_undoRedo.Do(removeCmds);
  const QString succeedMsg = QString("All %1 item(s) move to trashbin succeed.").arg(removeCmds.size());
  qDebug("%s", isAllSucceed ? qPrintable(succeedMsg) : "Some item(s) move to trashbin failed");
  Notificator::information("Move to trash", isAllSucceed ? succeedMsg : "Some item(s) move to trashbin failed");
  return isAllSucceed;
}

bool FileExplorerEvent::on_deletePermanently() {
  auto vt = _contentPane->GetCurViewType();
  if (!ViewTypeTool::isFSView(vt)) {
    return false;
  }
  const QString pth = _contentPane->getRootPath();
  if (PathTool::isLinuxRootOrWinEmpty(pth)) {
    qInfo("[Delete Permanently] Only available on FileSytemView[%c] and non-empty-path[%s]", (char)vt, qPrintable(pth));
    Notificator::warning("[Delete Permanently] Only available on FileSytemView and non-empty-path", QString::number((int)vt) + '|' + pth);
    return false;
  }
  using namespace FileOperatorType;
  BATCH_COMMAND_LIST_TYPE cmds;
  for (const QModelIndex ind : selectedIndexes()) {
    QFileInfo fi = _fileSysModel->fileInfo(ind);
    if (fi.isDir()) {
      cmds.append(ACMD{RMDIR, {pth, fi.fileName()}});
    } else if (fi.isFile()) {
      cmds.append(ACMD{RMFILE, {pth, fi.fileName()}});
    } else {
      qInfo("Here may some types not removed [%s]", qPrintable(fi.filePath()));
    }
  }
  static constexpr int fileNameMaximumCntWhenShowDeleteQueryDialog = 20;
  QString fileNames;
  QString deleteCmds;
  for (int i = 0; i < cmds.size(); ++i) {
    const ACMD& sl = cmds[i];
    if (i < fileNameMaximumCntWhenShowDeleteQueryDialog) {
      fileNames += (sl.lst.back() + "\n");
    }
    deleteCmds += (sl.toStr() + "\n");
  }
  if (cmds.size() > fileNameMaximumCntWhenShowDeleteQueryDialog) {
    fileNames += QString("---\nAnd the Following %1 item(s) find in details\n...").arg(cmds.size() - fileNameMaximumCntWhenShowDeleteQueryDialog);
  }

  QMessageBox* msgBox = new QMessageBox(QMessageBox::Icon::NoIcon, QString("PERMANENTLY Delete these %1 item(s)?").arg(cmds.size()), "NOT RECOVERABLE!");
  msgBox->setWindowIcon(QIcon(":img/DELETE_ITEMS_PERMANENTLY"));
  msgBox->setInformativeText(fileNames);
  msgBox->setDetailedText(deleteCmds);

  msgBox->setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
  msgBox->setDefaultButton(QMessageBox::Cancel);
  auto ret = msgBox->exec();
  if (ret != QMessageBox::Ok) {
    qInfo("[Cancelled] Delete");
    return true;
  }
  const auto isAllSucceed = g_undoRedo.Do(cmds);
  const QString& sucMsg = QString("%1 item(s) deleted permanently succeed.").arg(cmds.size());
  qWarning("%s", isAllSucceed ? qPrintable(sucMsg) : "Some item(s) deleted permanently failed");
  return isAllSucceed;
}

auto FileExplorerEvent::on_SelectAll() -> void {
  auto* view = _contentPane->GetCurView();
  if (not view->hasFocus()) {
    return;
  }
  view->selectAll();
}

auto FileExplorerEvent::on_SelectNone() -> void {
  auto* view = _contentPane->GetCurView();
  if (not view->hasFocus()) {
    return;
  }
  view->clearSelection();
}

auto FileExplorerEvent::on_SelectInvert() -> void {
  ViewType viewType = _contentPane->GetCurViewType();
  if (!isFSView(viewType)) {
    qDebug("[selection invert] only available on FileSytemView but[%c]", (char)viewType);
    return;
  }

  QAbstractItemView* view = _contentPane->GetCurView();
  const QModelIndex& rootIndex = view->rootIndex();
  const int row = _fileSysModel->rowCount(rootIndex);
  const int col = (viewType == ViewType::LIST) ? 1 : _fileSysModel->columnCount(rootIndex);
  _contentPane->disconnectSelectionChanged(viewType);  // Avoid lags when selection changed frequently
  qInfo("Path[%s] Dimension of file system model %d-by-%d", qPrintable(_fileSysModel->rootPath()), row, col);
  const QModelIndex& topLeft = _fileSysModel->index(0, 0, rootIndex);
  const QModelIndex& bottomRight = _fileSysModel->index(row - 1, col - 1, rootIndex);
  view->selectionModel()->select(QItemSelection(topLeft, bottomRight), QItemSelectionModel::Toggle);
  _contentPane->connectSelectionChanged(viewType);
}

bool FileExplorerEvent::on_HarView() {
  const QString& fileAbsPath = _contentPane->getCurFilePath();
  if (!fileAbsPath.toLower().endsWith(".har")) {
    qWarning("file[%s] type is not .har", qPrintable(fileAbsPath));
    Notificator::warning("file[%s] type is not .har", fileAbsPath);
    return false;
  }
  if (!QFile::exists(fileAbsPath)) {
    qWarning("Har file[%s] not exist", qPrintable(fileAbsPath));
    Notificator::warning("Har file[%s] not exist", fileAbsPath);
    return false;
  }
  static HarTableView* harTableview{nullptr};
  if (harTableview == nullptr) {
    harTableview = new HarTableView;
  }
  harTableview->operator()(fileAbsPath);
  harTableview->show();
  harTableview->raise();
  return true;
}

auto FileExplorerEvent::on_PlaySelectedItemsInView() -> bool {
  if (videoPlayer == nullptr) {
    videoPlayer = new VideoPlayer(this->_contentPane);
  }
  videoPlayer->show();
  const QStringList& filesList = _contentPane->getFilePaths();
  if (filesList.size() == 0) {
    return true;
  }
  if (filesList.size() == 1) {
    (*videoPlayer)(filesList.front());
    return true;
  }
  return (*videoPlayer)(filesList);
}

bool FileExplorerEvent::on_PlayCurrentPathOfView() {
  if (not _contentPane->isFSView() or PathTool::isRootOrEmpty(_fileSysModel->rootPath())) {
    qInfo("[Play current folder] only available on FileSytemView[%s] and non-empty-path[%s]", qPrintable(_contentPane->GetCurViewName()), qPrintable(_fileSysModel->rootPath()));
    Notificator::information("[Play current folder] only available on FileSytemView", _contentPane->GetCurViewName() + '|' + _fileSysModel->rootPath());
    return false;
  }
  if (videoPlayer == nullptr) {
    videoPlayer = new VideoPlayer(this->_contentPane);
  }
  videoPlayer->show();
  return videoPlayer->operator()(_fileSysModel->rootPath());
}

auto FileExplorerEvent::on_PlayVideo() const -> bool {
  auto vt = _contentPane->GetCurViewType();
  const bool supportViewType{vt == ViewTypeTool::ViewType::LIST         //
                             || vt == ViewTypeTool::ViewType::TABLE     //
                             || vt == ViewTypeTool::ViewType::TREE      //
                             || vt == ViewTypeTool::ViewType::MOVIE     //
                             || vt == ViewTypeTool::ViewType::SEARCH};  //
  if (!supportViewType) {
    qDebug("[Play Video] only available on list/table/tree/movie/search [%s]", qPrintable(_contentPane->GetCurViewName()));
    return false;
  }
  // select an item or select nothing
  QString playPath = _contentPane->getCurFilePath();
  if (playPath.isEmpty() && _contentPane->isFSView()) {
    playPath = _fileSysModel->rootPath();
  }
  if (PathTool::isRootOrEmpty(playPath)) {
    qWarning("Play skip. Empty path or root path[%s]", qPrintable(playPath));
    Notificator::warning("Play skip. Empty path or root path", playPath);
    return true;
  }
  if (!on_ShiftEnterPlayVideo(playPath)) {
    qInfo("Play[%s] failed", qPrintable(playPath));
    Notificator::information("Play failed", playPath);
    return false;
  }
  qInfo("Playing[%s]", qPrintable(playPath));
  Notificator::information("Playing...", playPath);
  return true;
}

bool FileExplorerEvent::on_Merge(const bool isReverse) {
  // reverse left right folder;
  if (not _contentPane->isFSView() or PathTool::isRootOrEmpty(_fileSysModel->rootPath())) {
    return false;
  }
  if (selectedIndexes().size() != 2) {
    QMessageBox::warning(_contentPane, "Merge tasks skip", "Only select two directory.");
    return false;
  }
  // l < r and not isReverse => merge into r
  // l not < folder r and isReverse => merge into r
  QString l = _fileSysModel->filePath(selectedIndexes().front());
  QString r = _fileSysModel->filePath(selectedIndexes().back());
  if ((l.toLower() < r.toLower()) == isReverse) {  // ignore case compare
    l.swap(r);
  }
  if (not(QFileInfo(l).isDir() and QFileInfo(r).isDir())) {
    qInfo("[Merge Folder Skip]. Only two folder can merged[%s][%s]", qPrintable(l), qPrintable(r));
    Notificator::information("[Merge Folder Skip]. Only select two directory", l + '|' + r);
    return false;
  }
  ConflictsItemHelper conflictIF(l, r, CCMMode::MERGE_OP);
  auto* tfm = new ConflictsRecycle(conflictIF);
  if (!conflictIF) {
    const bool mergeRet = tfm->on_completeMerge();
    if (_logger) {
      _logger->msg(QString("[%1] merged to [%2], Not conflict result:%3").arg(l).arg(r).arg(mergeRet));
    }
  } else {
    tfm->show();
  }
  if (_logger) {
    _logger->msg(QString("[%1] merged to [%2].").arg(l, r));
  }
  return true;
}
auto FileExplorerEvent::on_Copy() -> bool {
  int urlsCnt = -1;
  QStringList absPathsList;
  QList<QUrl> urlsList;
  std::tie(absPathsList, urlsList) = _contentPane->getFilePathsAndUrls(Qt::CopyAction);

  if (_contentPane->GetCurViewName() == ENUM_2_STR(SEARCH)) {
    urlsCnt = m_clipboard->FillIntoClipboardKeepFilesLevelBehavior(absPathsList, urlsList, CCMMode::COPY_OP);
    return true;
  }
  if (_contentPane->isFSView()) {
    urlsCnt = m_clipboard->FillIntoClipboardFSKeepFilesLevelBehavior(_fileSysModel->rootPath(), absPathsList, urlsList, CCMMode::COPY_OP);
  }
  if (_logger) {
    _logger->msg(QString("%1 path(s) copied").arg(urlsCnt), STATUS_STR_TYPE::NORMAL);
  }
  return true;
}
auto FileExplorerEvent::on_Cut() -> bool {
  int urlsCnt = -1;
  QStringList absPathsList;
  QList<QUrl> urlsList;
  std::tie(absPathsList, urlsList) = _contentPane->getFilePathsAndUrls(Qt::MoveAction);
  if (_contentPane->GetCurViewName() == ENUM_2_STR(SEARCH)) {
    urlsCnt = m_clipboard->FillIntoClipboardKeepFilesLevelBehavior(absPathsList, urlsList, CCMMode::CUT_OP);
    return true;
  }
  if (_contentPane->isFSView()) {
    urlsCnt = m_clipboard->FillIntoClipboardFSKeepFilesLevelBehavior(_fileSysModel->rootPath(), absPathsList, urlsList, CCMMode::CUT_OP);
  }
  if (_logger) {
    _logger->msg(QString("%1 path(s) cut").arg(urlsCnt), STATUS_STR_TYPE::NORMAL);
  }
  return true;
}

bool FileExplorerEvent::on_Paste() {
  if (!_contentPane->isFSView() || PathTool::isLinuxRootOrWinEmpty(_fileSysModel->rootPath())) {
    qInfo("[Paste] only available on FileSytemView[%s] and non-empty-path[%s]", qPrintable(_contentPane->GetCurViewName()), qPrintable(_fileSysModel->rootPath()));
    Notificator::information("[Paste] only available on FileSytemView", _contentPane->GetCurViewName() + '|' + _fileSysModel->rootPath());
    return false;
  }
  MimeDataCX dataInClipboard(MimeDataCX::fromPlainMimeData(m_clipboard->mimeData()));
  if (not dataInClipboard.hasUrls()) {
    return false;
  }
  const QString& rTo = _fileSysModel->rootPath();
  if (rTo == "C:/") {
    qWarning("Destination Folder Access Denied. Do this operation on system explorer.");
    Notificator::warning("Destination Folder Access Denied", "Do this operation on system explorer.");
    return false;
  }
  if (dataInClipboard.m_cutCopy == CCMMode::ERROR_OP) {
    const CCMMode::Mode retMode = QueryCopyOrCut();
    if (retMode == CCMMode::ERROR_OP) {
      if (_logger)
        _logger->msg("[Cancel] Paste operation", STATUS_STR_TYPE::NORMAL);
      return true;
    }
    dataInClipboard.determineMode(retMode);
  }

  _fileSysModel->ClearCopyAndCutDict();
  if (_contentPane->m_searchSrcModel != nullptr) {
    _contentPane->m_searchSrcModel->ClearCopyAndCutDict();
  }

  ConflictsItemHelper conflictIF(dataInClipboard.l, rTo, dataInClipboard.lRels, dataInClipboard.m_cutCopy);
  auto* tfm = new ConflictsRecycle(conflictIF);
  if (!conflictIF) {
    const bool ret = tfm->on_completeMerge();
    qDebug("No conflict, paste result:%d", ret);
  } else {
    tfm->show();
  }
  return true;
}

bool FileExplorerEvent::on_NameStandardize() {
  if (not _contentPane->isFSView() or PathTool::isRootOrEmpty(_fileSysModel->rootPath())) {
    qDebug("[Name Standardize] Only available on FileSytemView[%s] and non-empty-path[%s]", qPrintable(_contentPane->GetCurViewName()), qPrintable(_fileSysModel->rootPath()));
    Notificator::information("[Name Standardize] Only available on FileSytemView and non-empty-path", _contentPane->GetCurViewName() + '|' + _fileSysModel->rootPath());
    return false;
  }
  const QDir pathdir = this->_fileSysModel->rootDirectory();
  const QString& currentPath = pathdir.absolutePath();
  if (currentPath.isEmpty() or pathdir.isRoot()) {
    qWarning("Abort. Cannot work on [%s]", qPrintable(currentPath));
    Notificator::warning("Abort", QString("Cannot work on [%1]").arg(currentPath));
    return false;
  }
  auto* msgBox = new QMessageBox(QMessageBox::Icon::Question, "Name Standardlizer Confirm?", QString("All item(s) under [%1] will be RENAMED RECURSIVELY!").arg(currentPath));
  msgBox->setWindowIcon(QIcon(":img/NAME_RULER"));
  msgBox->setInformativeText(QString("Work path [%1]?").arg(currentPath));
  msgBox->setDetailedText(QString("path:[%1].\nOperation Recoverable.").arg(currentPath));
  msgBox->setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
  msgBox->setDefaultButton(QMessageBox::Ok);
  const auto ret = msgBox->exec();
  if (ret != QMessageBox::Ok) {
    qInfo("User cancel. Name standardlize");
    Notificator::information("User cancel", "Name standardlize");
    return true;
  }
  qInfo("Name standardlize Start...");
  Notificator::information("Name standardlize", "Start...");
  FilesNameBatchStandardizer fnbs;
  const auto isAllSuccess = fnbs(currentPath);
  qInfo(isAllSuccess ? "Name standardlize Finshed" : "Name standardlize Some Failed");
  if (isAllSuccess) {
    Notificator::goodNews("Name standardlize", "Finshed");
  } else {
    Notificator::badNews("Name standardlize", "Some Failed");
  }
  return isAllSuccess;
}

bool FileExplorerEvent::on_FileClassify() {
  if (not _contentPane->isFSView() or PathTool::isRootOrEmpty(_fileSysModel->rootPath())) {
    qDebug("[File Classify] Only available on FileSytemView[%s] and non-empty-path[%s]", qPrintable(_contentPane->GetCurViewName()), qPrintable(_fileSysModel->rootPath()));
    Notificator::information("[File Classify] Only available on FileSytemView and non-empty-path", _contentPane->GetCurViewName() + '|' + _fileSysModel->rootPath());
    return false;
  }
  const auto beforeOpt = _fileSysModel->options();
  _fileSysModel->setOptions(QFileSystemModel::DontWatchForChanges);
  const QString& currentPath = _fileSysModel->rootPath();
  ItemsClassifier classfier;
  classfier(currentPath);
  QString startMsg = "Item(s) Classify Start... total:" + QString::number(classfier.CommandsCnt()) + "cmd(s)..";
  qInfo("%s", qPrintable(startMsg));
  Notificator::information(startMsg, currentPath);
  bool classifyResult = classfier.StartToRearrange();
  QString endMsg = "Item(s) Classify Finished... bAllSucceed:" + QString::number(classifyResult);
  qInfo("%s", qPrintable(endMsg));
  Notificator::goodNews(endMsg, currentPath);
  _fileSysModel->setOptions(beforeOpt);
  _fileSysModel->setRootPath("");
  _contentPane->GetCurView()->setRootIndex(_fileSysModel->setRootPath(currentPath));
  return true;
}

bool FileExplorerEvent::on_FileUnclassify() {
  if (not _contentPane->isFSView() or PathTool::isRootOrEmpty(_fileSysModel->rootPath())) {
    qDebug("[File Unclassify] Only available on FileSytemView[%s] and non-empty-path[%s]", qPrintable(_contentPane->GetCurViewName()), qPrintable(_fileSysModel->rootPath()));
    Notificator::information("[File Classify] Only available on FileSytemView and non-empty-path", _contentPane->GetCurViewName() + '|' + _fileSysModel->rootPath());
    return false;
  }
  const QDir pathdir = this->_fileSysModel->rootDirectory();
  const QString& currentPath = pathdir.absolutePath();
  if (currentPath.isEmpty() or pathdir.isRoot()) {
    qWarning("Abort. Unclassify cannot work on [%s]", qPrintable(currentPath));
    Notificator::warning("Abort", QString("Unclassify cannot work on [%1]").arg(currentPath));
    return false;
  }

  auto* msgBox = new QMessageBox(QMessageBox::Icon::Question, "Unpile Confirm (lag may cause)?", QString("All item(s) under [%1] will be unpile out!").arg(currentPath));
  msgBox->setWindowIcon(QIcon(":img/UNPACK_FOLDERS"));
  msgBox->setInformativeText(QString("Work path [%1]?").arg(currentPath));
  msgBox->setDetailedText(QString("path:[%1].\nOperation Recoverable.").arg(currentPath));
  msgBox->setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
  msgBox->setDefaultButton(QMessageBox::Ok);
  const auto ret = msgBox->exec();
  if (ret != QMessageBox::Ok) {
    qInfo("User cancel. Skip unpile");
    Notificator::information("User cancel", "Skip unpile");
    return true;
  }
  const auto beforeOpt = _fileSysModel->options();
  _fileSysModel->setOptions(QFileSystemModel::DontWatchForChanges);
  ExtractPileItemsOutFolder unclassfier;
  unclassfier(currentPath);
  QString startMsg = "Item(s) Unclassify Start... total:" + QString::number(unclassfier.CommandsCnt()) + "cmd(s)..";
  qInfo("%s", qPrintable(startMsg));
  Notificator::information(startMsg, currentPath);
  bool classifyResult = unclassfier.StartToRearrange();
  QString endMsg = "Item(s) Unclassify Finished... bAllSucceed:" + QString::number(classifyResult);
  qInfo("%s", qPrintable(endMsg));
  Notificator::goodNews(endMsg, currentPath);
  _fileSysModel->setOptions(beforeOpt);
  _fileSysModel->setRootPath("");
  _contentPane->GetCurView()->setRootIndex(_fileSysModel->setRootPath(currentPath));
  return true;
}

bool FileExplorerEvent::on_RemoveDuplicateImages() {
  if (!_contentPane->isFSView() || PathTool::isRootOrEmpty(_fileSysModel->rootPath())) {
    qDebug("[on_RemoveDuplicateImages] Only available on FileSytemView[%s] and non-empty-path[%s]", qPrintable(_contentPane->GetCurViewName()), qPrintable(_fileSysModel->rootPath()));
    Notificator::information("[on_RemoveDuplicateImages] Only available on FileSytemView and non-empty-path", _contentPane->GetCurViewName() + '|' + _fileSysModel->rootPath());
    return false;
  }

  const QString& currentPath = _fileSysModel->rootPath();
  if (_logger) {
    _logger->msg("Start to Remove duplicate imgs[Only resolution differs]...", STATUS_STR_TYPE::NORMAL);
    _logger->SetProgressValue(0);
  }
  auto choice = QMessageBox::question(_contentPane, "Confirm remove duplicate images?", "Images that differ in resolution will be delete");
  if (choice != QMessageBox::StandardButton::Yes) {
    if (_logger)
      _logger->msg("User Cancel remove", STATUS_STR_TYPE::NORMAL);
    return false;
  }
  int removedCnt = LowResImgsRemover()(currentPath);
  if (_logger) {
    _logger->msg(QString("Remove duplicate %1 image(s) Finished").arg(removedCnt), STATUS_STR_TYPE::NORMAL);
    _logger->SetProgressValue(100);
  }
  return true;
}

bool FileExplorerEvent::on_RemoveRedundantItem(RedundantRmv& remover) {
  if (! _contentPane->isFSView() || PathTool::isRootOrEmpty(_fileSysModel->rootPath())) {
    qDebug("[Remove redundant item] Only available on FileSytemView[%s] and non-empty-path[%s]", qPrintable(_contentPane->GetCurViewName()), qPrintable(_fileSysModel->rootPath()));
    Notificator::information("[Remove redundant item] Only available on FileSytemView and non-empty-path", _contentPane->GetCurViewName() + '|' + _fileSysModel->rootPath());
    return false;
  }

  const QString& path = _fileSysModel->rootPath();
  int cmdCnt = remover(path);
  if (cmdCnt == 0) {
    qInfo("Skip. Nothing to remove");
    Notificator::information("Skip", "Nothing to remove");
    return true;
  }
  auto* msgBox = new QMessageBox(QMessageBox::Icon::Warning, QString("Confirm %1 command(s)?").arg(cmdCnt), "Remove Redundant folder");
  msgBox->setStandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
  msgBox->setDefaultButton(QMessageBox::StandardButton::No);
  msgBox->setDetailedText(QString(remover));
  const auto ret = msgBox->exec();
  if (ret != QMessageBox::StandardButton::Yes) {
    qDebug("User cancel");
    return true;
  }
  return remover.Exec();
}

bool FileExplorerEvent::on_MoveCopyEventSkeleton(const CCMMode::Mode operationName, QString r) {
  const auto vt = _contentPane->GetCurViewType();
  if (vt == ViewType::SEARCH) {
    qDebug("[Move/Copy to] for [%s view] use Copy/Cut/Paste to filesytem instead", qPrintable(_contentPane->GetCurViewName()));
    Notificator::warning("[Move/Copy to] for [%s view] use Copy/Cut/Paste to filesytem instead", _contentPane->GetCurViewName());
    return false;
  }
  if (!ViewTypeTool::isFSView(vt) || PathTool::isLinuxRootOrWinEmpty(_fileSysModel->rootPath())) {
    qDebug("[Move/Copy to] only available on FileSytemView[%s] and non-empty-path[%s]", qPrintable(_contentPane->GetCurViewName()), qPrintable(_fileSysModel->rootPath()));
    Notificator::warning("[Move/Copy to] only available on FileSytemView[%1] and non-empty-path[%2]", _contentPane->GetCurViewName() + '|' + _contentPane->GetCurViewName());
    return false;
  }
  auto* view = _contentPane->GetCurView();
  if (not view->selectionModel()->hasSelection()) {
    qInfo("Select Some File/Folder First. %s", CCMMode::MCCL2STR[operationName]);
    Notificator::information("Select Some File/Folder First", CCMMode::MCCL2STR[operationName]);
    return true;
  }
  const QString& l = _fileSysModel->rootPath();
  if (r.isEmpty()) {
    r = ChooseCopyDestination(l, view);
  }
  if (r.isEmpty() || !QFileInfo{r}.isDir()) {
    qWarning("Destination[%s] of [%s to] Error", qPrintable(r), CCMMode::MCCL2STR[operationName]);
    Notificator::warning("Destination[%1] of [%2 to] Error", r + '|' + CCMMode::MCCL2STR[operationName]);
    return true;
  }

  if (operationName == CCMMode::CUT_OP) {
    const QString& newPathHistory = MoveToNewPathAutoUpdateActionText(r, g_fileBasicOperationsActions().MOVE_TO_PATH_HISTORY);
    PreferenceSettings().setValue(MemoryKey::MOVE_TO_PATH_HISTORY.name, newPathHistory);
  } else if (operationName == CCMMode::COPY_OP) {
    const QString& newPathHistory = MoveToNewPathAutoUpdateActionText(r, g_fileBasicOperationsActions().COPY_TO_PATH_HISTORY);
    PreferenceSettings().setValue(MemoryKey::COPY_TO_PATH_HISTORY.name, newPathHistory);
  } else {
    qDebug("OperationName[%s] is invalid", CCMMode::MCCL2STR[operationName]);
    return false;
  }

  QStringList lRels;
  for (const QModelIndex ind : selectedIndexes()) {
    lRels.append(_fileSysModel->fileName(ind));
  }
  ConflictsItemHelper conflictIF(l, r, lRels, operationName);
  auto* tfm = new ConflictsRecycle(conflictIF);
  if (not conflictIF) {
    tfm->on_completeMerge();
  } else {
    tfm->show();
  }

  qInfo("[%s to] Destination[%s] Finished.", CCMMode::MCCL2STR[operationName], qPrintable(r));
  Notificator::information("[%1 to] Destination[%2] Finished", r + '|' + CCMMode::MCCL2STR[operationName]);
  return true;
}

CCMMode::Mode FileExplorerEvent::QueryCopyOrCut() {
  auto* msgBox = new QMessageBox(QMessageBox::Icon::Information, QString("Move or Copy?"), "Both of these Will keep source folder structure");
  msgBox->setStandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::Cancel | QMessageBox::StandardButton::Apply);
  msgBox->button(QMessageBox::StandardButton::Apply)->setText("Copy To");
  msgBox->button(QMessageBox::StandardButton::Apply)->setIcon(QIcon(":img/CP_TO_COMMAND_PATH"));

  msgBox->button(QMessageBox::StandardButton::Yes)->setText("Move To");
  msgBox->button(QMessageBox::StandardButton::Yes)->setIcon(QIcon(":img/MV_TO_COMMAND_PATH"));
  msgBox->button(QMessageBox::StandardButton::Yes)->setStyleSheet(SUBMIT_BTN_STYLE);

  msgBox->setDefaultButton(QMessageBox::StandardButton::Cancel);
  const auto ret = msgBox->exec();
  switch (ret) {
    case QMessageBox::StandardButton::Apply:
      return CCMMode::COPY_OP;
    case QMessageBox::StandardButton::Yes:
      return CCMMode::CUT_OP;
    default:
      return CCMMode::ERROR_OP;
  }
}
