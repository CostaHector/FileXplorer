#include "FileExplorerEvent.h"

#include "ArchiveFilesActions.h"
#include "ArrangeActions.h"
#include "FileBasicOperationsActions.h"
#include "FileLeafAction.h"
#include "RenameActions.h"
#include "RightClickMenuActions.h"
#include "VideoPlayerActions.h"
#include "ViewActions.h"
#include "ThumbnailProcessActions.h"

#include "AlertSystem.h"
#include "Archiver.h"
#include "CustomStatusBar.h"
#include "MD5Window.h"
#include "NotificatorMacro.h"
#include "PropertiesWindow.h"
#include "RedundantImageFinder.h"
#include "ViewsStackedWidget.h"
#include "AdvanceRenamer.h"
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
#include "ViewSelection.h"
#include "ViewTypeTool.h"
#include "ThumbnailProcesser.h"

#include "OnCheckedPopupOrHideAWidget.h"
#include "PublicTool.h"
#include "MemoryKey.h"
#include "StyleSheet.h"
#include "UndoRedo.h"
#include "ComplexOperation.h"

#include <QApplication>
#include <QInputDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QProcess>

using namespace ViewTypeTool;

FileExplorerEvent* FileExplorerEvent::GetFileExlorerEvent(FileSystemModel* fsm, ViewsStackedWidget* view, CustomStatusBar* logger, QObject* parent) {
  CHECK_NULLPTR_RETURN_NULLPTR(fsm)
  CHECK_NULLPTR_RETURN_NULLPTR(view)
  CHECK_NULLPTR_RETURN_NULLPTR(logger)
  static FileExplorerEvent eve(fsm, view, logger, parent);
  eve.subscribe();
  return &eve;
}

FileExplorerEvent::FileExplorerEvent(FileSystemModel* fsm, ViewsStackedWidget* view, CustomStatusBar* logger, QObject* parent)
  : QObject{parent} {  //
  _fileSysModel = fsm;
  _contentPane = view;
  _logger = logger;
  m_clipboard = QApplication::clipboard();
  // connect(m_clipboard, &QClipboard::dataChanged, this, &FileExplorerEvent::onSystemClipboardDataChanged);
}

auto FileExplorerEvent::on_NewTextFile(QString newTextName, const QString& contents) -> bool {  // not effect by selection;
  if (!__CanNewItem()) {
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
  BATCH_COMMAND_LIST_TYPE cmds{ACMD::GetInstTOUCH(_fileSysModel->rootPath(), newTextName)};
  const auto isAllSucceed = g_undoRedo.Do(cmds);
  if (!isAllSucceed) {
    qWarning("[Error] touch command failed when create plain text file.");
    return false;
  }
  const QString& txtFilePath = _fileSysModel->rootDirectory().absoluteFilePath(newTextName);
  if (!contents.isEmpty()) {
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

bool FileExplorerEvent::on_NewJsonFile() {
  if (!__CanNewItem()) {
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
    LOG_INFO_P("Partial Json file(s) already exists", "%d exists as follows:\n%s", jsonFileAlreadyExist.size(), qPrintable(jsonFileAlreadyExist.join('\n')));
  }
  if (!jsonFilesNameCreated.isEmpty()) {
    LOG_GOOD_P("Partial Json file(s) Create succeed", "%d exists as follows:\n%s", jsonFilesNameCreated.size(), qPrintable(jsonFilesNameCreated.join('\n')));
  } else {
    LOG_GOOD_NP("No jsons need created under path", path);
  }
  return true;
}

bool FileExplorerEvent::on_NewFolder() {  // not effect by selection;
  if (!__CanNewItem()) {
    return false;
  }
  const QString& newFolderName = QString("New Folder %1").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
  if (_fileSysModel->rootDirectory().exists(newFolderName)) {
    qInfo("Skip. There is already a folder[%s] in folder[%s].", qPrintable(newFolderName), qPrintable(_fileSysModel->rootPath()));
    return false;
  }
  using namespace FileOperatorType;
  BATCH_COMMAND_LIST_TYPE cmds{ACMD::GetInstMKPATH(_fileSysModel->rootPath(), newFolderName)};
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
  if (!__CanNewItem()) {
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
    if (isFolder) {
      cmds.append(ACMD::GetInstMKPATH(createInPath, fileNameArray));
    } else {
      cmds.append(ACMD::GetInstTOUCH(createInPath, fileNameArray));
    }
  }
  const auto isAllSucceed = g_undoRedo.Do(cmds);
  if (not isAllSucceed) {
    qWarning("[Error] Some commands failed when create %d file/folder(s).", cmds.size());
    return false;
  }
  return true;
}

bool FileExplorerEvent::on_BatchNewFilesOrFolders(bool isFolder) {
  const QString defNamePattern = isFolder ? Configuration().value(MemoryKey::NAME_PATTERN_USED_CREATE_BATCH_FOLDERS.name, MemoryKey::NAME_PATTERN_USED_CREATE_BATCH_FOLDERS.v).toString()
                                          : Configuration().value(MemoryKey::NAME_PATTERN_USED_CREATE_BATCH_FILES.name, MemoryKey::NAME_PATTERN_USED_CREATE_BATCH_FILES.v).toString();

  const QString userInputRule = QInputDialog::getText(//
        _contentPane, //
        QString("Create Batch %1").arg(isFolder ? "Folders" : "Files"),
        "Rule Pattern: C-styleFormatString%StartIndex$EndIndex",
        QLineEdit::Normal,
        defNamePattern);
  const QStringList& userInputLst = userInputRule.split('$');
  if (userInputLst.size() != 3) {
    LOG_WARN_P("[Error] Invalid Rule pattern",
               "Rule pattern[%s] should contains 3 parts seperated by '$', but there are %d parts",
               qPrintable(userInputRule), userInputLst.size());
    return false;
  }
  char namePattern[260] = "\0";
  strncpy(namePattern, userInputLst[0].toStdString().c_str(), sizeof(namePattern) - 1);
  int startIndex = userInputLst[1].toInt();
  int endIndex = userInputLst[2].toInt();
  if (startIndex >= endIndex) {
    LOG_INFO_P("Skip", "Create %d file/folders", endIndex - startIndex)
    return true;
  }
  Configuration().setValue(isFolder ? MemoryKey::NAME_PATTERN_USED_CREATE_BATCH_FOLDERS.name : MemoryKey::NAME_PATTERN_USED_CREATE_BATCH_FILES.name, userInputRule);

  return on_BatchNewFilesOrFolders(namePattern, startIndex, endIndex, isFolder);
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
  if (!ThumbnailProcesser::IsDimensionXValid(dimensionX)) {
    LOG_INFO_NP("Dimension of row invalid", QString::number(dimensionX));
    return false;
  }
  if (!ThumbnailProcesser::IsDimensionYValid(dimensionY)) {
    LOG_INFO_NP("Dimension of column invalid", QString::number(dimensionY));
    return false;
  }
  if (!ThumbnailProcesser::IsWidthPixelAllowed(widthPx)) {
    LOG_INFO_NP("images width invalid", QString::number(widthPx));
    return false;
  }
  const int curSamplePeriod = Configuration().value(MemoryKey::DEFAULT_THUMBNAIL_SAMPLE_PERIOD.name, MemoryKey::DEFAULT_THUMBNAIL_SAMPLE_PERIOD.v).toInt();
  if (!ThumbnailProcesser::IsSamplePeriodAllowed(curSamplePeriod)) {
    LOG_INFO_NP("Sample period not allowed", QString::number(curSamplePeriod));
    return false;
  }
  const int cnt = ThumbnailProcesser::CreateThumbnailImages(selectedFiles, dimensionX, dimensionY, widthPx, curSamplePeriod, true);
  if (cnt <= 0) {
    LOG_BAD_NP("Create thumbnail failed", "see details in log");
    return false;
  }
  LOG_GOOD_P("Create thumbnail(s) for video(s) succeed", "count: %d", cnt);
  return true;
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
      LOG_INFO_NP("User cancel", "Skip extract images out of thumbnail")
      return true;
    }
  }
  ThumbnailProcesser tp{skipIfExist};
  int extractedOutCnt = tp(currentPath, beg, end);
  if (!tp.mErrImg.isEmpty()) {
    LOG_INFO_P("Extract Failed", "%d images, as follows:\n", extractedOutCnt, qPrintable(tp.mErrImg.join('\n')));
    return false;
  }
  if (extractedOutCnt == 0) {
    LOG_GOOD_NP("Nothing need extracted out", currentPath);
  } else {
    LOG_GOOD_P("[Ok] Extract", "%d image(s) from path[%s]", extractedOutCnt, qPrintable(currentPath));
  }
  return true;
}

QModelIndexList FileExplorerEvent::selectedIndexes() const {
  // ignore other column, keep the first column
  return ViewSelection::selectedIndexes(_contentPane->GetCurView());
}

bool FileExplorerEvent::on_searchKeywordInSystemDefaultExplorer() const {
  ViewTypeTool::ViewType vt = _contentPane->GetVt();
  if (!(_contentPane->isFSView() || vt == ViewTypeTool::ViewType::CAST)) {
    qDebug("View[%d] not suport", (int)vt);
  }
  QString absFilePath = _contentPane->getCurFilePath();
  QString noExtAbsFilePath = PathTool::GetFileNameExtRemoved(absFilePath);
  QString imgFileAbsPathGuess = QDir::toNativeSeparators(noExtAbsFilePath) + ' ';
  QApplication::clipboard()->setText(imgFileAbsPathGuess, QClipboard::Mode::Clipboard);
  QString fileBaseName = PathTool::GetBaseName(absFilePath);
  QString forSearch = fileBaseName.replace(JSON_RENAME_REGEX::INVALID_GOOGLE_SEARCH_LETTER, " ");
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
  auto* md5W = new (std::nothrow) MD5Window{this->_contentPane};
  CHECK_NULLPTR_RETURN_FALSE(md5W);
  md5W->show();
  if (!_contentPane->isFSView()) {
    return true;
  }
  const QStringList& items = selectedItems();
  if (items.isEmpty()) {
    return true;
  }
  const int filesCnt = md5W->operator()(items);
  qDebug("%d md5(s) calculate.", filesCnt);
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
  if (_contentPane->GetVt() == ViewType::MOVIE) {
    pW = new PropertiesWindow(this->_contentPane);
    pW->show();
    pW->operator()(_contentPane->m_movieDbModel, _contentPane->m_movieView);
    return true;
  }
  LOG_INFO_P("Reject", "Not support in view[%s]", qPrintable(_contentPane->GetCurViewName()));
  return false;
}

void FileExplorerEvent::subsribeCompress() {
  connect(g_AchiveFilesActions().COMPRESSED_HERE, &QAction::triggered, this, &FileExplorerEvent::on_compress);
  connect(g_AchiveFilesActions().DECOMPRESSED_HERE, &QAction::triggered, this, &FileExplorerEvent::on_deCompress);
  connect(g_AchiveFilesActions().COMPRESSED_IMAGES, &QAction::triggered, this, &FileExplorerEvent::on_compressImgsByGroup);
  connect(g_AchiveFilesActions().ARCHIVE_PREVIEW, &QAction::triggered, this, &FileExplorerEvent::on_archivePreview);
}

void FileExplorerEvent::subsribeFileActions() {
  connect(g_fileLeafActions()._SETTINGS, &QAction::triggered, this,                           //
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
    Configuration().setValue(MemoryKey::LANGUAGE_ZH_CN.name, cnEnabled);
    LOG_INFO_NP("Language switch", "work after reopen")
  });
}

void FileExplorerEvent::subscribeThumbnailActions() {
  auto& ins = g_ThumbnailProcessActions();
  connect(ins._CREATE_THUMBNAIL_AG, &QActionGroup::triggered, this, [this, &ins](QAction* createThumbnailAct) {
    auto it = ins.mCreateThumbnailDimension.find(createThumbnailAct);
    if (it == ins.mCreateThumbnailDimension.cend()) {
      qWarning("create thumbnail action[%p] not support", createThumbnailAct);
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
      LOG_INFO_NP("[Skip] User cancel set sample period", "return")
      return;
    }
    if (newSamplePeriod == curSamplePeriod) {
      return;
    }
    Configuration().setValue(MemoryKey::DEFAULT_THUMBNAIL_SAMPLE_PERIOD.name, newSamplePeriod);
    LOG_GOOD_P("ThumbnailSamplePeriod", "T=%d", newSamplePeriod);
  });

  connect(ins._EXTRACT_THUMBNAIL_AG, &QActionGroup::triggered, this, [this, &ins](QAction* extractThumbnailAct) {
    auto it = ins.mExtractThumbnailRange.find(extractThumbnailAct);
    if (it == ins.mExtractThumbnailRange.cend()) {
      qWarning("extract thumbnail action[%p] not support", extractThumbnailAct);
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
      LOG_INFO_P("[Skip]User canceled or invalid input", "input[%s]", qPrintable(input));
      return;
    }
    static const QRegularExpression regex(R"(^\d,\d$)");
    QRegularExpressionMatch match = regex.match(input);
    if (!match.hasMatch()) {
      LOG_WARN_NP("Invalid Range Format", input)
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
  subsribeFileActions();
  subscribeThumbnailActions();

  {
    auto& fileOpInst = g_fileBasicOperationsActions();
    connect(fileOpInst.NEW_FOLDER, &QAction::triggered, this, &FileExplorerEvent::on_NewFolder);
    connect(fileOpInst.NEW_TEXT_FILE, &QAction::triggered, this, [this]() { this->on_NewTextFile(); });
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

    connect(fileOpInst._MOVE_TO, &QAction::triggered, this, [this]() { this->on_MoveTo(); });
    connect(fileOpInst._COPY_TO, &QAction::triggered, this, [this]() { this->on_CopyTo(); });

    connect(fileOpInst.MOVE_TO_PATH_HISTORY, &QActionGroup::triggered, this, [this](const QAction* const act) { on_MoveTo(act->text()); });
    connect(fileOpInst.COPY_TO_PATH_HISTORY, &QActionGroup::triggered, this, [this](const QAction* const act) { on_CopyTo(act->text()); });

    connect(fileOpInst.CUT, &QAction::triggered, this, &FileExplorerEvent::on_Cut);
    connect(fileOpInst.COPY, &QAction::triggered, this, &FileExplorerEvent::on_Copy);
    connect(fileOpInst.PASTE, &QAction::triggered, this, &FileExplorerEvent::on_Paste);

    connect(fileOpInst._NAME_RULER, &QAction::triggered, this, &FileExplorerEvent::on_NameStandardize);
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
      auto* pToLongPath = new RenameWidget_LongPath(_contentPane);
      onRename(pToLongPath);
    });

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
      auto* pNumerize = new RenameWidget_Numerize(_contentPane);
      onRename(pNumerize);
    });
    connect(renameInst._SECTIONS_ARRANGE, &QAction::triggered, this, [this]() -> void {
      auto* pArrange = new RenameWidget_ArrangeSection(_contentPane);
      onRename(pArrange);
    });
    connect(renameInst._REVERSE_NAMES_LIST, &QAction::triggered, this, [this]() -> void {
      auto* pReverse = new RenameWidget_ReverseNames(_contentPane);
      onRename(pReverse);
    });
    connect(renameInst._CASE_NAME, &QAction::triggered, this, [this]() -> void {
      auto* pCase = new RenameWidget_Case(_contentPane);
      onRename(pCase);
    });
    connect(renameInst._STR_INSERTER, &QAction::triggered, this, [this]() -> void {
      auto* pInsert = new RenameWidget_Insert(_contentPane);
      onRename(pInsert);
    });
    connect(renameInst._STR_DELETER, &QAction::triggered, this, [this]() -> void {
      auto* pDelete = new RenameWidget_Delete(_contentPane);
      onRename(pDelete);
    });
    connect(renameInst._STR_REPLACER, &QAction::triggered, this, [this]() -> void {
      auto* pReplacer = new RenameWidget_Replace(_contentPane);
      onRename(pReplacer);
    });
    connect(renameInst._CONTINUOUS_NUMBERING, &QAction::triggered, this, [this]() -> void {
      auto* pNoConsecutive = new RenameWidget_ConsecutiveFileNo(_contentPane);
      onRename(pNoConsecutive);
    });
    connect(renameInst._CONVERT_UNICODE_TO_ASCII, &QAction::triggered, this, [this]() -> void {
      auto* pToAscii = new RenameWidget_ConvertBoldUnicodeCharset2Ascii(_contentPane);
      onRename(pToAscii);
    });
    connect(renameInst._PREPEND_PARENT_FOLDER_NAMES, &QAction::triggered, this, [this]() -> void {
      auto* pPrependName = new RenameWidget_PrependParentFolderName(_contentPane);
      onRename(pPrependName);
    });
  }

  {
    auto& viewInst = g_viewActions();
    connect(viewInst._HAR_VIEW, &QAction::triggered, this, &FileExplorerEvent::on_HarView);
    connect(viewInst._SYS_VIDEO_PLAYERS, &QAction::triggered, this, &FileExplorerEvent::on_PlayVideo);
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

bool FileExplorerEvent::__CanNewItem() const {
  if (!_contentPane->isFSView()) {
    LOG_INFO_P("Reject New", "view name[%s]", qPrintable(_contentPane->GetCurViewName()));
    return false;
  }
  if (PathTool::isLinuxRootOrWinEmpty(_fileSysModel->rootPath())) {
    LOG_INFO_P("Reject New", "Don't create item under path[%s]", qPrintable(_fileSysModel->rootPath()));
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
  qWarning("on_revealInExplorer with program[%s] parms [%s]", qPrintable(process.program()), qPrintable(args.join(',')));
  return true;
}

bool FileExplorerEvent::on_OpenInTerminal() const {
  if (!(_contentPane->isFSView() || _contentPane->GetCurViewName() == ENUM_2_STR(SEARCH))) {
    LOG_WARN_NP("[Skip] Open In Terminal", _contentPane->GetCurViewName());
    return false;
  }
  return SysTerminal()(_contentPane->getRootPath());
}

bool FileExplorerEvent::on_forceRefreshFileSystemModel() {
  if (!_contentPane->isFSView()) {
    LOG_WARN_NP("[Skip] Refresh", _contentPane->GetCurViewName());
    return false;
  }
  QAbstractItemView* fsView = _contentPane->GetCurView();
  if (fsView == nullptr) {
    return false;
  }
  const QString& path = _fileSysModel->rootPath();
  _fileSysModel->setRootPath("");
  fsView->setRootIndex(_fileSysModel->setRootPath(path));
  LOG_GOOD_NP("Refresh filesytemmodel of path: ", path);
  return true;
}

bool FileExplorerEvent::on_compress() {
  if (!((_contentPane->isFSView() || _contentPane->GetCurViewName() == ENUM_2_STR(SEARCH)) && !PathTool::isLinuxRootOrWinEmpty(_contentPane->getRootPath()))) {
    LOG_INFO_P("[Skip] Compress", "viewName:%s, rootPath:%s",
               qPrintable(_contentPane->GetCurViewName()), qPrintable(_contentPane->getRootPath()));
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
  if (!((_contentPane->isFSView() || _contentPane->GetCurViewName() == ENUM_2_STR(SEARCH)) && !PathTool::isLinuxRootOrWinEmpty(_contentPane->getRootPath()))) {
    LOG_INFO_P("[Skip] Decompress", "viewName:%s, rootPath:%s", qPrintable(_contentPane->GetCurViewName()), qPrintable(_contentPane->getRootPath()));
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
  qInfo("Decompress %d file(s), %d failed", filesPath.size(), failsQzFiles.size());
  if (!failsQzFiles.isEmpty()) {
    LOG_BAD_P("[Partially Failed] Decompress qz", "following %d file(s) failed:\n%s", failsQzFiles.size(), qPrintable(failsQzFiles.join('\n')));
  } else {
    LOG_GOOD_P("[Ok] Decompress qz", "%d qz files decompressed ok", filesPath.size());
  }
  return failsQzFiles.isEmpty();
}

bool FileExplorerEvent::on_compressImgsByGroup() {
  if (!_contentPane->isFSView()) {
    LOG_INFO_P("[Skip] Compress images", "viewName:%s", qPrintable(_contentPane->GetCurViewName()));
    return false;
  }
  const QString& pth = _contentPane->getRootPath();
  ArchiveImagesRecusive air{true};
  int compressFolderCnt = air.CompressImgRecur(pth);
  LOG_INFO_P("[Ok] Compress images", "count: %d", compressFolderCnt)
  return true;
}

bool FileExplorerEvent::on_archivePreview() {
  auto vt = _contentPane->GetVt();
  if (!ViewTypeTool::isFSView(vt) && vt != ViewType::SEARCH) {
    return false;
  }
  const QString pth = _contentPane->getRootPath();
  if (PathTool::isLinuxRootOrWinEmpty(pth)) {
    LOG_WARN_P("[Abort ArchivePreivew]", "Path[%s] root or empty", qPrintable(pth));
    return false;
  }
  const QStringList& filesPath = _contentPane->getFilePaths();
  if (filesPath.isEmpty() || filesPath.size() > 1) {
    LOG_WARN_P("[Abort ArchivePreivew]", "archives files count[%d]", filesPath.size());
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
  ViewTypeTool::ViewType vt = _contentPane->GetVt();
  if (!ViewTypeTool::isFSView(vt) && vt != ViewType::SEARCH) {
    LOG_WARN_P("[Skip Move to trashbin] Not FileSytemView/Search View", "viewType %d", (int)vt);
    return false;
  }
  const QString pth = _contentPane->getRootPath();
  if (PathTool::isLinuxRootOrWinEmpty(pth)) {
    LOG_WARN_NP("[Skip Move to trashbin] Path not support", pth);
    return false;
  }

  QStringList prepaths, names;
  std::tie(prepaths, names) = _contentPane->getFilePrepathsAndName(true);
  if (names.size() <= 0) {
    LOG_INFO_NP("[Skip Move to trashbin]", "Nothing selected");
    return true;
  }
  if (prepaths.size() != names.size()) {
    LOG_INFO_P("[Skip Move to trashbin]", "Prepaths[%d] and Names[%d] length Inequal", prepaths.size(), names.size());
    return true;
  }

  using namespace FileOperatorType;
  BATCH_COMMAND_LIST_TYPE removeCmds;
  removeCmds.reserve(prepaths.size());
  for (int i = 0; i < prepaths.size(); ++i) {
    removeCmds.append(ACMD::GetInstMOVETOTRASH(prepaths[i], names[i]));
  }
  const bool isAllSucceed = g_undoRedo.Do(removeCmds);
  if (isAllSucceed) {
    LOG_GOOD_P("Move to trash all succeed", "Commands count %d", removeCmds.size());
  } else {
    LOG_BAD_NP("Move to trash partially failed", "Some item(s) move to trashbin failed");
  }
  return isAllSucceed;
}

bool FileExplorerEvent::on_deletePermanently() {
  auto vt = _contentPane->GetVt();
  if (!ViewTypeTool::isFSView(vt)) {
    return false;
  }
  const QString pth = _contentPane->getRootPath();
  if (PathTool::isLinuxRootOrWinEmpty(pth)) {
    LOG_INFO_P("[Abort] Delete Permanently", "viewName:%s, rootPath:%s", qPrintable(_contentPane->GetCurViewName()), qPrintable(pth));
    return false;
  }
  using namespace FileOperatorType;
  BATCH_COMMAND_LIST_TYPE cmds;
  for (const QModelIndex& ind : selectedIndexes()) {
    QFileInfo fi = _fileSysModel->fileInfo(ind);
    if (fi.isDir()) {
      cmds.append(ACMD::GetInstRMFOLDERFORCE(pth, fi.fileName()));
    } else if (fi.isFile()) {
      cmds.append(ACMD::GetInstRMFILE(pth, fi.fileName()));
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
      fileNames += (sl.parms.back() + "\n");
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
  ViewType viewType = _contentPane->GetVt();
  if (!isFSView(viewType)) {
    qDebug("[selection invert] only available on FileSytemView but[%c]", (char)viewType);
    return;
  }

  QAbstractItemView* view = _contentPane->GetCurView();
  const QModelIndex& rootIndex = view->rootIndex();
  const int row = _fileSysModel->rowCount(rootIndex);
  const int col = (viewType == ViewType::LIST) ? 1 : _fileSysModel->columnCount(rootIndex);
  _contentPane->disconnectSelectionChanged();  // Avoid lags when selection changed frequently
  qInfo("Path[%s] Dimension of file system model %d-by-%d", qPrintable(_fileSysModel->rootPath()), row, col);
  const QModelIndex& topLeft = _fileSysModel->index(0, 0, rootIndex);
  const QModelIndex& bottomRight = _fileSysModel->index(row - 1, col - 1, rootIndex);
  view->selectionModel()->select(QItemSelection(topLeft, bottomRight), QItemSelectionModel::Toggle);
  _contentPane->connectSelectionChanged(viewType);
}

bool FileExplorerEvent::on_HarView() {
  const QString& fileAbsPath = _contentPane->getCurFilePath();
  if (!fileAbsPath.toLower().endsWith(".har")) {
    LOG_WARN_NP("File extension is not .har", fileAbsPath);
    return false;
  }
  if (!QFile::exists(fileAbsPath)) {
    LOG_WARN_NP("Har file not exist", fileAbsPath);
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

auto FileExplorerEvent::on_PlayVideo() const -> bool {
  auto vt = _contentPane->GetVt();
  const bool supportViewType{vt == ViewTypeTool::ViewType::LIST         //
                             || vt == ViewTypeTool::ViewType::TABLE     //
                             || vt == ViewTypeTool::ViewType::TREE      //
                             || vt == ViewTypeTool::ViewType::MOVIE     //
                             || vt == ViewTypeTool::ViewType::SEARCH};  //
  if (!supportViewType) {
    LOG_INFO_P("[Skip] Play Video", "viewName:%s", qPrintable(_contentPane->GetCurViewName()));
    return false;
  }
  // select an item or select nothing
  QString playPath = _contentPane->getCurFilePath();
  if (playPath.isEmpty() && _contentPane->isFSView()) {
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
  LOG_INFO_NP("Playing...", playPath);
  return true;
}

bool FileExplorerEvent::on_Merge(const bool isReverse) {
  // reverse left right folder;
  if (!_contentPane->isFSView() or PathTool::isRootOrEmpty(_fileSysModel->rootPath())) {
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
  const QString msg{l + "\n" + r};
  if (!QFileInfo{l}.isDir() || !QFileInfo{r}.isDir()) {
    LOG_INFO_NP("[Merge Folder Skip]. Select two folder", msg);
    return false;
  }

  using namespace ComplexOperation;
  ComplexMerge cm;
  BATCH_COMMAND_LIST_TYPE aBatch = cm.Merge(l, r);
  const bool isAllSucceed{g_undoRedo.Do(aBatch)};
  if (!isAllSucceed) {
    LOG_WARN_NP("Merge partial failed", msg);
    return false;
  }
  LOG_GOOD_NP("[ok] Merged", msg);
  return true;
}

bool FileExplorerEvent::on_Copy() {
  QStringList absPaths;
  QList<QUrl> urls;
  std::tie(absPaths, urls) = _contentPane->getFilePathsAndUrls(Qt::CopyAction);
  QMimeData* pMimeData = new (std::nothrow) QMimeData;
  CHECK_NULLPTR_RETURN_FALSE(pMimeData)
  pMimeData->setText(absPaths.join('\n'));
  pMimeData->setUrls(urls);
  if (!SetMimeDataCutCopy(*pMimeData, Qt::CopyAction)) {
    LOG_BAD_NP("Set Copy Action in QMimedata failed", "Abort");
    return false;
  }
  m_clipboard->setMimeData(pMimeData);
  if (_logger != nullptr) {
    _logger->msg(QString("%1 path(s) been copied").arg(absPaths.size()), STATUS_STR_TYPE::NORMAL);
  }
  return true;
}

bool FileExplorerEvent::on_Cut() {
  QStringList absPaths;
  QList<QUrl> urls;
  std::tie(absPaths, urls) = _contentPane->getFilePathsAndUrls(Qt::MoveAction);
  QMimeData* pMimeData = new (std::nothrow) QMimeData;
  pMimeData->setText(absPaths.join('\n'));
  pMimeData->setUrls(urls);
  if (!SetMimeDataCutCopy(*pMimeData, Qt::MoveAction)) {
    LOG_BAD_NP("Set Cut Action in QMimedata failed", "Abort");
    return false;
  }
  m_clipboard->setMimeData(pMimeData);
  if (_logger != nullptr) {
    _logger->msg(QString("%1 path(s) been cut").arg(absPaths.size()), STATUS_STR_TYPE::NORMAL);
  }
  return true;
}

bool FileExplorerEvent::on_Paste() {
  if (!_contentPane->isFSView()) {
    LOG_WARN_NP("[Paste Skip] Only available on FileSytemView", "Skip");
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
      LOG_GOOD_NP("User cancel paste", "neither flatten or keep");
      return true;
    }
  }
  Qt::DropAction dropAction = GetCutCopyModeFromNativeMimeData(*pMimeData);
  int ret = DoDropAction(dropAction, pMimeData->urls(), rTo, fileStructMode);
  if (ret < 0) {
    LOG_BAD_NP("Paste operation partially failed", rTo);
    return false;
  }
  LOG_GOOD_NP("Paste operation all succeed", rTo);
  _fileSysModel->ClearCopyAndCutDict();
  if (_contentPane->m_searchSrcModel != nullptr) {
    _contentPane->m_searchSrcModel->ClearCopyAndCutDict();
  }
  return true;
}

bool FileExplorerEvent::on_NameStandardize() {
  if (!_contentPane->isFSView() || PathTool::isRootOrEmpty(_fileSysModel->rootPath())) {
    LOG_INFO_P("[Skip] NameStandardize", "viewName:%s, rootPath:%s",
               qPrintable(_contentPane->GetCurViewName()), qPrintable(_fileSysModel->rootPath()));
    return false;
  }
  const QString currentPath{_fileSysModel->rootPath()};
  auto* msgBox = new QMessageBox(QMessageBox::Icon::Question, "Name Standardlizer Confirm?", QString("All item(s) under [%1] will be RENAMED RECURSIVELY!").arg(currentPath));
  msgBox->setWindowIcon(QIcon(":img/NAME_RULER"));
  msgBox->setInformativeText(QString("Work path [%1]?").arg(currentPath));
  msgBox->setDetailedText(QString("path:[%1].\nOperation Recoverable.").arg(currentPath));
  msgBox->setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
  msgBox->setDefaultButton(QMessageBox::Ok);
  const auto ret = msgBox->exec();
  if (ret != QMessageBox::Ok) {
    LOG_INFO_NP("User cancel", "Name standardlize");
    return true;
  }
  LOG_INFO_NP("Name standardlize", "Start...");
  FilesNameBatchStandardizer fnbs;
  const bool isAllSuccess = fnbs(currentPath);
  if (isAllSuccess) {
    LOG_GOOD_NP("[Ok] Name standardlize", "Finshed");
  } else {
    LOG_BAD_NP("[Partial Failed] Name standardlize", "Some Failed");
  }
  return isAllSuccess;
}

bool FileExplorerEvent::on_FileClassify() {
  if (!_contentPane->isFSView() || PathTool::isRootOrEmpty(_fileSysModel->rootPath())) {
    LOG_INFO_P("[Skip] FileClassify", "viewName:%s, rootPath:%s",
               qPrintable(_contentPane->GetCurViewName()), qPrintable(_fileSysModel->rootPath()));
    return false;
  }
  const auto beforeOpt = _fileSysModel->options();
  _fileSysModel->setOptions(QFileSystemModel::DontWatchForChanges);
  const QString& currentPath = _fileSysModel->rootPath();
  ItemsPacker classfier;
  classfier(currentPath);
  QString startMsg = "Item(s) Classify Start... total:" + QString::number(classfier.CommandsCnt()) + "cmd(s)..";
  LOG_INFO_NP(startMsg, currentPath);
  bool classifyResult = classfier.StartToRearrange();
  QString endMsg = "Item(s) Classify Finished... bAllSucceed:" + QString::number(classifyResult);
  LOG_GOOD_NP(endMsg, currentPath);
  _fileSysModel->setOptions(beforeOpt);
  _fileSysModel->setRootPath("");
  _contentPane->GetCurView()->setRootIndex(_fileSysModel->setRootPath(currentPath));
  return true;
}

bool FileExplorerEvent::on_FileUnclassify() {
  if (!_contentPane->isFSView() || PathTool::isRootOrEmpty(_fileSysModel->rootPath())) {
    LOG_INFO_P("[Skip] FileUnclassify", "viewName:%s, rootPath:%s",
      qPrintable(_contentPane->GetCurViewName()), qPrintable(_fileSysModel->rootPath()));
    return false;
  }
  const QDir pathdir = this->_fileSysModel->rootDirectory();
  const QString& currentPath = pathdir.absolutePath();
  if (currentPath.isEmpty() || pathdir.isRoot()) {
    LOG_WARN_P("[Skip] FileUnclassify", "current path[%s]", qPrintable(currentPath));
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
    LOG_INFO_NP("User cancel", "Skip unpile");
    return true;
  }
  const auto beforeOpt = _fileSysModel->options();
  _fileSysModel->setOptions(QFileSystemModel::DontWatchForChanges);
  ItemsUnpacker unclassfier;
  unclassfier(currentPath);
  QString startMsg = "Item(s) Unclassify Start... total:" + QString::number(unclassfier.CommandsCnt()) + "cmd(s)..";
  LOG_INFO_NP(startMsg, currentPath);
  bool classifyResult = unclassfier.StartToRearrange();
  QString endMsg = "Item(s) Unclassify Finished... bAllSucceed:" + QString::number(classifyResult);
  LOG_GOOD_NP(endMsg, currentPath);
  _fileSysModel->setOptions(beforeOpt);
  _fileSysModel->setRootPath("");
  _contentPane->GetCurView()->setRootIndex(_fileSysModel->setRootPath(currentPath));
  return true;
}

bool FileExplorerEvent::on_RemoveDuplicateImages() {
  if (!_contentPane->isFSView() || PathTool::isRootOrEmpty(_fileSysModel->rootPath())) {
    LOG_INFO_P("[Skip] RemoveDuplicateImages", "viewName:%s, rootPath:%s",
             qPrintable(_contentPane->GetCurViewName()), qPrintable(_fileSysModel->rootPath()));
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
  if (!_contentPane->isFSView()) {
    LOG_INFO_NP("[Remove redundant item] Only available on FileSytemView", "Skip");
    return false;
  }
  const QString& path = _fileSysModel->rootPath();
  if (PathTool::isRootOrEmpty(path)) {
    LOG_INFO_NP("[Remove redundant item] Path invalid", path);
    return false;
  }

  int cmdCnt = remover(path);
  if (cmdCnt == 0) {
    LOG_INFO_NP("Skip", "Nothing to remove");
    return true;
  }
  auto* msgBox = new QMessageBox(QMessageBox::Icon::Warning, QString("Confirm %1 command(s)?").arg(cmdCnt), "Remove Redundant folder");
  msgBox->setStandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
  msgBox->setDefaultButton(QMessageBox::StandardButton::No);
  msgBox->setDetailedText(QString(remover));
  const auto ret = msgBox->exec();
  if (ret != QMessageBox::StandardButton::Yes) {
    LOG_GOOD_NP("[Skip]User cancel remove redundant item", "return");
    return true;
  }
  return remover.Exec();
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
    LOG_WARN_P("[Move/Copy to] only available on FileSytemView", "Now: %d", (int)vt);
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
    qDebug("DropAction[%d] is invalid", (int)dropAct);
    return false;
  }

  QStringList lRels;
  for (const QModelIndex ind : selectedIndexes()) {
    lRels.append(_fileSysModel->filePath(ind));
  }
  using namespace ComplexOperation;
  int ret = DoDropAction(dropAction, lRels, dest, FILE_STRUCTURE_MODE::PRESERVE);
  if (ret < 0) {
    LOG_BAD_P(pOperationNameStr, "Failed. errorCode:%d", ret);
    return false;
  }
  LOG_GOOD_NP(pOperationNameStr, "All succeed");
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

bool FileExplorerEvent::SetMimeDataCutCopy(QMimeData& mimeData, const Qt::DropAction dropAction) {
#ifdef _WIN32
  QByteArray preferred(4, 0x0);
  if (dropAction == Qt::DropAction::MoveAction) {  // # 2 for cut and 5 for copy
    preferred[0] = 0x2;
  } else if (dropAction == Qt::DropAction::CopyAction) {
    preferred[0] = 0x5;
  } else {
    qWarning("cannot refill base DropEffect");
    return false;
  }
  mimeData.setData("Preferred DropEffect", preferred);
#else
  if (dropAction == Qt::DropAction::MoveAction) {
    mimeData.setData("XdndAction", "XdndActionMove");
  } else if (dropAction == Qt::DropAction::CopyAction) {
    mimeData.setData("XdndAction", "XdndActionCopy");
  } else {
    qWarning("cannot refill base DropEffect");
    return false;
  }
#endif
  return true;
}
