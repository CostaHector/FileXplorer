#include "SceneListView.h"
#include "NotificatorMacro.h"
#include "ScenesListModel.h"
#include "PlayVideo.h"
#include "PublicMacro.h"
#include "NotificatorMacro.h"
#include "PathTool.h"
#include "SceneInPageActions.h"
#include "SceneInfoManager.h"
#include "BatchRenameBy.h"
#include "FileOperatorPub.h"
#include "UndoRedo.h"
#include "RecycleCfmDlg.h"
#include "InputDialogHelper.h"
#include "ViewHelper.h"
#include "VideoTierTool.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QMouseEvent>

SceneListView::SceneListView(ScenesListModel* sceneModel,
                             SceneSortProxyModel* sceneSortProxyModel,
                             ScenePageControl* scenePageControl,
                             QWidget* parent) //
  : CustomListView{"SCENES_TABLE", parent}
  , //
  _sceneModel{sceneModel}
  , //
  _sceneSortProxyModel{sceneSortProxyModel}
  ,                                   //
  _scenePageControl{scenePageControl} //
{
  CHECK_NULLPTR_RETURN_VOID(_sceneModel)
  CHECK_NULLPTR_RETURN_VOID(sceneSortProxyModel)
  CHECK_NULLPTR_RETURN_VOID(_scenePageControl)
  _sceneSortProxyModel->setSourceModel(_sceneModel);
  setModel(_sceneSortProxyModel);
  setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectItems);

  mAlignDelegate = new (std::nothrow) SceneStyleDelegate{this};
  CHECK_NULLPTR_RETURN_VOID(mAlignDelegate)
  setItemDelegate(mAlignDelegate);

  _RENAME_SCENE_RELATED_FILES_REPLACE = new (std::nothrow) QAction(QIcon(":img/RENAME"), tr("Rename related(replace)"), this);
  _RENAME_SCENE_RELATED_FILES_REPLACE->setShortcutVisibleInContextMenu(true);
  _RENAME_SCENE_RELATED_FILES_REPLACE->setToolTip(QString("<b>%1 (%2)</b><br/>Rename selected json file(s) and associated files by replacing a substring in the file names.") //
                                                      .arg(_RENAME_SCENE_RELATED_FILES_REPLACE->text())
                                                      .arg(_RENAME_SCENE_RELATED_FILES_REPLACE->shortcut().toString()));

  _RENAME_SCENE_RELATED_FILES_INSERT = new (std::nothrow) QAction(QIcon(":img/NAME_STR_INSERTER_PATH"), tr("Rename related(Insert)"), this);
  _RENAME_SCENE_RELATED_FILES_INSERT->setShortcutVisibleInContextMenu(true);
  _RENAME_SCENE_RELATED_FILES_INSERT->setToolTip(QString("<b>%1 (%2)</b><br/>Rename selected json file(s) and associated files by inserting a string into the file names.") //
                                                     .arg(_RENAME_SCENE_RELATED_FILES_INSERT->text())
                                                     .arg(_RENAME_SCENE_RELATED_FILES_INSERT->shortcut().toString()));

  _RENAME_SCENE_RELATED_FILES_NUMERIZE = new (std::nothrow) QAction(QIcon(":img/NAME_STR_NUMERIZER_PATH"), tr("Rename (ith)"), this);
  _RENAME_SCENE_RELATED_FILES_NUMERIZE->setShortcutVisibleInContextMenu(true);
  _RENAME_SCENE_RELATED_FILES_NUMERIZE->setToolTip(
      QString("<b>%1 (%2)</b><br/> Numerizer each file in a sequence.").arg(_RENAME_SCENE_RELATED_FILES_NUMERIZE->text()).arg(_RENAME_SCENE_RELATED_FILES_NUMERIZE->shortcut().toString()));

  _RECYCLE_SCENE_RELATED_FILES = new (std::nothrow) QAction{QIcon{":img/MOVE_TO_TRASH_BIN"}, tr("Recycle related files"), this};
  CHECK_NULLPTR_RETURN_VOID(_RECYCLE_SCENE_RELATED_FILES)
  _RECYCLE_SCENE_RELATED_FILES->setToolTip(QString("<b>%1 (%2)</b><br/> Move selected scene related file(s) name to trash bin") //
                                               .arg(_RECYCLE_SCENE_RELATED_FILES->text())
                                               .arg(_RECYCLE_SCENE_RELATED_FILES->shortcut().toString()));

  _OPEN_CORRESPONDING_FOLDER = new (std::nothrow) QAction{QIcon{":img/SYSTEM_APPLICATION_VIDEO"}, tr("Play this folder"), this};
  CHECK_NULLPTR_RETURN_VOID(_OPEN_CORRESPONDING_FOLDER)

  auto& inst = SceneInPageActions::GetInst();
  QList<QAction*> exclusiveActions{
      inst._CREATE_THUMBNAIL_FOR_JSON_RELATED_IMGS,
      _RENAME_SCENE_RELATED_FILES_REPLACE,  //
      _RENAME_SCENE_RELATED_FILES_INSERT,   //
      _RENAME_SCENE_RELATED_FILES_NUMERIZE, //
  };
  exclusiveActions += inst._ARCHIVE_AG->actions();
  exclusiveActions.push_back(inst._ARCHIVE_BY_MOVIE_SCORE);
  exclusiveActions.push_back(_RECYCLE_SCENE_RELATED_FILES);
  exclusiveActions.push_back(_OPEN_CORRESPONDING_FOLDER);

  PushFrontExclusiveActions(exclusiveActions);
  PushBackExclusiveActions(_sceneModel->GetExcusiveActions());

  subscribe();

  // setMouseTracking(true);
  // setAttribute(Qt::WA_Hover, true);
  // viewport()->setMouseTracking(true);
}

SceneListView::~SceneListView() {
  const auto& inst = SceneInPageActions::GetInst();
  SceneInfo::SaveInitialSortRole(inst.GetSortRole());
  SceneInfo::SaveSortOrderReverse(inst.GetSortOrderReverse());
  SceneInfo::SaveIncludeScnInSubdirectories(inst.GetbSubdirectories());
}

bool SceneListView::onOpenCorrespondingFolder() {
  const QModelIndex& curInd = currentIndex();
  if (!curInd.isValid()) {
    return false;
  }
  const QModelIndex& srcInd = _sceneSortProxyModel->mapToSource(curInd);
  const QString& scenePath = _sceneModel->absolutePath(srcInd);
  bool openResult = on_ShiftEnterPlayVideo(scenePath);
  LOG_D("Play path: [%s]", qPrintable(scenePath));
  return openResult;
}

void SceneListView::subscribe() {
  connect(_OPEN_CORRESPONDING_FOLDER, &QAction::triggered, this, &SceneListView::onOpenCorrespondingFolder);
  connect(this, &QListView::iconSizeChanged, _sceneModel, &QAbstractListModelPub::onIconSizeChange);
  connect(_RENAME_SCENE_RELATED_FILES_REPLACE, &QAction::triggered, this, &SceneListView::onRenameSceneAndRelated);
  connect(_RENAME_SCENE_RELATED_FILES_INSERT, &QAction::triggered, this, &SceneListView::onRenameSceneAndRelatedInsert);
  connect(_RENAME_SCENE_RELATED_FILES_NUMERIZE, &QAction::triggered, this, &SceneListView::onRenameSceneAndRelatedNumerize);
  connect(_RECYCLE_SCENE_RELATED_FILES, &QAction::triggered, this, &SceneListView::onRecycleSceneAndRelated);

  connect(_scenePageControl, &ScenePageControl::currentPageIndexChanged, _sceneModel, &ScenesListModel::onPageIndexChanged);
  connect(_scenePageControl, &ScenePageControl::maxScenesCountPerPageChanged, _sceneModel, &ScenesListModel::onScenesCountsPerPageChanged);
  connect(_sceneModel, &ScenesListModel::pagesCountChanged, _scenePageControl, &ScenePageControl::onPagesCountChanged);
  SceneInPageActions& sceneActInst = SceneInPageActions::GetInst();
  {
    // initial signal-slot connection
    toggleSortRequestImplementer(sceneActInst.GetSortRangeCurrentPageOnly());
  }

  connect(&sceneActInst, &SceneInPageActions::sortImplementerChanged, this, &SceneListView::toggleSortRequestImplementer);
  connect(&sceneActInst, &SceneInPageActions::disableImageDecorationChanged, _sceneModel, &ScenesListModel::onDisableImageDecorationChanged);
  connect(sceneActInst._UPDATE_JSON, &QAction::triggered, this, &SceneListView::onUpdateJsonFiles);
  connect(sceneActInst._UPDATE_SCN, &QAction::triggered, this, &SceneListView::onUpdateScnFiles);
  connect(sceneActInst._CREATE_THUMBNAIL_FOR_JSON_RELATED_IMGS, &QAction::triggered, this, &SceneListView::onCreateFrontImageThumbnail);
  connect(sceneActInst._INCLUDEING_SUBDIRECTORIES, &QAction::toggled, _sceneModel, &ScenesListModel::onSubdirectoriesToggled);
  connect(sceneActInst._CLEAR_SCN_FILE, &QAction::triggered, this, &SceneListView::onClearScnFiles);
  connect(sceneActInst._ARCHIVE_BY_MOVIE_SCORE, &QAction::triggered, this, &SceneListView::onArchiveToByMovieRate);
  connect(sceneActInst._ARCHIVE_AG, &QActionGroup::triggered, this, &SceneListView::onArchiveActionTriggered);

  connect(this, &SceneListView::sceneGridClicked, mAlignDelegate, &SceneStyleDelegate::onSceneClicked);
  connect(mAlignDelegate, &SceneStyleDelegate::cellVisualUpdateRequested, this, &SceneListView::onCellVisualUpdateRequested);
}

void SceneListView::toggleSortRequestImplementer(bool bPageByPage) {
  if (mSortRoleConn) {
    SceneInPageActions::disconnect(mSortRoleConn);
  }
  if (mSortOrderReverseConn) {
    SceneInPageActions::disconnect(mSortOrderReverseConn);
  }

  const SceneInPageActions& sceneActInst = SceneInPageActions::GetInst();
  if (bPageByPage) { // locally
    if (!_sceneSortProxyModel->isSortProxyInited()) {
      const SceneInfo::Role initSortRole{sceneActInst.GetSortRole()};
      const bool bOrderReverse{sceneActInst.GetSortOrderReverse()};
      _sceneSortProxyModel->initSortSetting(initSortRole, bOrderReverse);
    }
    mSortRoleConn = connect(&sceneActInst, &SceneInPageActions::sceneSortDimensionChanged, _sceneSortProxyModel, &SceneSortProxyModel::setSortRole);
    mSortOrderReverseConn = connect(&sceneActInst, &SceneInPageActions::sceneSortReverseOrderChanged, _sceneSortProxyModel, &SceneSortProxyModel::setSortOrder);
  } else { // globally
    if (!_sceneModel->isSortProxyInited()) {
      const SceneInfo::Role initSortRole{sceneActInst.GetSortRole()};
      const bool bOrderReverse{sceneActInst.GetSortOrderReverse()};
      _sceneModel->initSortSetting(initSortRole, bOrderReverse);
    }
    mSortRoleConn = connect(&sceneActInst, &SceneInPageActions::sceneSortDimensionChanged, _sceneModel, &ScenesListModel::setSortDimension);
    mSortOrderReverseConn = connect(&sceneActInst, &SceneInPageActions::sceneSortReverseOrderChanged, _sceneModel, &ScenesListModel::setSortResultReverse);
  }
}

void SceneListView::setRootPath(const QString& rootPath) {
  const bool bNeedSkip{
#ifdef RUNNING_UNIT_TESTS
      false
#else
      PathTool::isPathAtShallowDepth(rootPath)
      && !InputDialogHelper::YesOrCancelBox(QMessageBox::Icon::Question,
                                            QIcon{},
                                            "Large Directory Warning - Performance Impact",
                                            "This directory appears to be at a high level in the filesystem and may contain a large number of items. "
                                            "Loading it could cause performance issues.\n\n"
                                            "Directory: "
                                                + rootPath + "\n\n Do you want to proceed?",
                                            "", "")
#endif
  };
  if (bNeedSkip) {
    // Potential large directory
    LOG_INFO_P("User canceled setting root path", "large directory:[%s]", qPrintable(rootPath));
    return;
  }
  const bool bSubdirectories = SceneInPageActions::GetInst().GetbSubdirectories();
  _sceneModel->setRootPath(rootPath, false, bSubdirectories);
}

int SceneListView::onUpdateJsonFiles() {
  const QString workPath = _sceneModel->rootPath();
  if (PathTool::isPathAtShallowDepth(workPath)) {
    LOG_ERR_P("Update aborted",
              "Path [%s] is too close to root directory. "
              "System files may get accidentally modified at this level.",
              qPrintable(workPath));
    return -1;
  }

  using namespace SceneInfoManager;
  ScnMgr scnMgr;
  Counter cnt = scnMgr(workPath);
  LOG_OK_P("Json file K-V updated",
           "updated:%d, used:%d\nimgUpdate:%d, vidUpdate:%d\nunder path[%s]", //
           cnt.m_jsonUpdatedCnt,
           cnt.m_jsonUsedCnt, //
           cnt.m_ImgNameKeyFieldUpdatedCnt,
           cnt.m_VidNameKeyFieldUpdatedCnt, //
           qPrintable(workPath));
  return cnt.m_jsonUpdatedCnt;
}

int SceneListView::onUpdateScnFiles() {
  const QString workPath = _sceneModel->rootPath();
  if (PathTool::isPathAtShallowDepth(workPath)) {
    LOG_ERR_P("Update aborted",
              "Path [%s] is too close to root directory. "
              "System files may get accidentally modified at this level.",
              qPrintable(workPath));
    return -1;
  }
  using namespace SceneInfoManager;
  int scnFileCnt = ScnMgr::UpdateScnFiles(workPath);
  LOG_OE_P(scnFileCnt >= 0, "Scn file updated", "count: %d, workPath[%s]", scnFileCnt, qPrintable(workPath));
  const bool bSubdirectories = SceneInPageActions::GetInst().GetbSubdirectories();
  _sceneModel->setRootPath(workPath, true, bSubdirectories);
  return scnFileCnt;
}

int SceneListView::onCreateFrontImageThumbnail() {
  const QModelIndexList& srcIndexes = selectedRowsSource();
  if (srcIndexes.isEmpty()) {
    LOG_INFO_NP("Skip create thumbnail", "no item selected");
    return 0;
  }
  using namespace SceneInfoManager;
  const int tnFileCnt = _sceneModel->createFrontImageThumbnail(srcIndexes, true);
  LOG_OE_P(tnFileCnt >= 0, "Json related image thumbnail updated", "count: %d", tnFileCnt);
  return tnFileCnt;
}

int SceneListView::onClearScnFiles() {
  const QString workPath = _sceneModel->rootPath();
  using namespace SceneInfoManager;
  int deleteCnt = ScnMgr::ClearScnFiles(workPath);
  LOG_OK_P("Delete scn file", "cnt: %d under[%s]", deleteCnt, qPrintable(workPath));
  const bool bSubdirectories = SceneInPageActions::GetInst().GetbSubdirectories();
  _sceneModel->setRootPath(workPath, true, bSubdirectories);
  return deleteCnt;
}

bool SceneListView::onClickEvent(const QModelIndex& current) {
  if (!current.isValid()) {
    emit currentSceneChanged("Nothing selected", "", {}, {});
    return false;
  }
  const QModelIndex& srcInd = _sceneSortProxyModel->mapToSource(current);
  const QString& name = _sceneModel->baseName(srcInd);
  const QString& jsonPath = _sceneModel->GetJson(srcInd);
  emit currentSceneChanged(name, jsonPath, _sceneModel->GetImgs(srcInd), _sceneModel->GetVids(srcInd));
  return true;
}

void SceneListView::initExclusivePreferenceSetting() {
  CustomListView::m_defaultFlowLeft2Right = true;
  CustomListView::m_defaultViewModeIcon = true;
  CustomListView::m_defaultWrapping = true;
}

QModelIndexList SceneListView::selectedRowsSource() const {
  const QModelIndexList& proIndexes = selectedIndexes();
  QModelIndexList srcIndexes;
  srcIndexes.reserve(proIndexes.size());
  for (const auto& proIndex : proIndexes) {
    srcIndexes.append(_sceneSortProxyModel->mapToSource(proIndex));
  }
  return srcIndexes;
}

void SceneListView::mousePressEvent(QMouseEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  if (event->button() == Qt::MouseButton::LeftButton) {
    const QPoint pos = event->pos();
    const QModelIndex proIndex = indexAt(pos); // here no need use mapToSource
    if (mLastClickedIndex != proIndex) {
      onClickEvent(proIndex);
    }
    mLastClickedIndex = proIndex;
    const QRect imageRect{mAlignDelegate->GetRatingAreaRect(visualRect(proIndex))};
    emit sceneGridClicked(proIndex, imageRect, pos);
  }
  QListView::mousePressEvent(event);
}

void SceneListView::keyPressEvent(QKeyEvent* e) {
  CHECK_NULLPTR_RETURN_VOID(e);
  if (ViewHelper::keyPressEventCore(e)) {
    return;
  }
  CustomListView::keyPressEvent(e);
}

void SceneListView::onCellVisualUpdateRequested(const QModelIndex& ind) {
  if (!ind.isValid()) {
    LOG_W("index invalid, no need visual update at all");
    return;
  }
  update(ind);
}

int SceneListView::onRenameSceneAndRelated() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("nothing selected", "skip rename");
    return 0;
  }
  emit requestStopMediaPlay();
  const QString& jsonLocatedInPath{_sceneModel->rootPath()};
  const QModelIndexList& indexes{selectedRowsSource()};
  const QStringList& jsonFileNames{_sceneModel->rel2fileNames(indexes)};
  const int relatedFilesCnt{BatchRenameBy::ReplaceBySpecifiedJson(jsonLocatedInPath, jsonFileNames)};
  if (relatedFilesCnt <= 0) {
    return 0;
  }
  const int removeRowCnt{_sceneModel->AfterJsonFilesNameRenamed(indexes)};
  return relatedFilesCnt;
}

int SceneListView::onRenameSceneAndRelatedInsert() {
  const QModelIndexList& srcIndexes{selectedRowsSource()};
  if (srcIndexes.isEmpty()) {
    LOG_INFO_NP("Skip rename(insert)", "no row selected");
    return 0;
  }
  emit requestStopMediaPlay();
  const QString& jsonLocatedInPath{_sceneModel->rootPath()};
  const QStringList& jsonFileNames{_sceneModel->rel2fileNames(srcIndexes)};
  const int relatedFilesCnt{BatchRenameBy::InsertBySpecifiedJson(jsonLocatedInPath, jsonFileNames)};
  if (relatedFilesCnt <= 0) {
    return 0;
  }

  const int removeRowCnt{_sceneModel->AfterJsonFilesNameRenamed(srcIndexes)};
  return relatedFilesCnt;
}

int SceneListView::onRenameSceneAndRelatedNumerize() {
  const QModelIndexList& srcIndexes{selectedRowsSource()};
  if (srcIndexes.isEmpty()) {
    LOG_INFO_NP("Skip rename(numerize)", "no row selected");
    return 0;
  }
  emit requestStopMediaPlay();
  const QString& jsonLocatedInPath{_sceneModel->rootPath()};
  const QStringList& jsonFileNames{_sceneModel->rel2fileNames(srcIndexes)};
  const int relatedFilesCnt{BatchRenameBy::NumerizerBySpecifiedJson(jsonLocatedInPath, jsonFileNames)};
  if (relatedFilesCnt <= 0) {
    return 0;
  }

  const int removeRowCnt{_sceneModel->AfterJsonFilesNameRenamed(srcIndexes)};
  return relatedFilesCnt;
}

int SceneListView::onRecycleSceneAndRelated() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("nothing selected", "skip recycle");
    return 0;
  }
  emit requestStopMediaPlay();
  const QString& jsonLocatedInPath{_sceneModel->rootPath()};
  const QModelIndexList& indexes{selectedRowsSource()};
  const QStringList& jsonFileNames{_sceneModel->rel2fileNames(indexes)};
  const QStringList& filesNeedRecycle = BatchRenameBy::GetFilesNeedProcess(jsonLocatedInPath, jsonFileNames);
  const int relatedFilesCnt{filesNeedRecycle.size()};

  if (!RecycleCfmDlg::recycleQuestion(jsonLocatedInPath, filesNeedRecycle, false)) {
    LOG_INFO_P("[Cancel] User cancel recycle", "%d item(s) no change", relatedFilesCnt);
    return 0;
  }

  FileOperatorType::BATCH_COMMAND_LIST_TYPE removeCmds;
  removeCmds.reserve(relatedFilesCnt);
  for (const auto& nm : filesNeedRecycle) {
    removeCmds.append(FileOperatorType::ACMD::GetInstMOVETOTRASH(jsonLocatedInPath, nm));
  }
  bool bAllSucceed = UndoRedo::GetInst().Do(removeCmds);
  const int removeRowCnt = _sceneModel->AfterJsonFilesNameRenamed(indexes);
  LOG_OE_P(bAllSucceed, "Recycle", "recycle %d json/img/video items, rows[%d]", relatedFilesCnt, removeRowCnt);
  return relatedFilesCnt;
}

bool SceneListView::onArchiveActionTriggered(const QAction* archivedToAct) {
  if (archivedToAct == nullptr) {
    return false;
  }
  bool bHasValidVideoTier{false};
  const int videoTier = archivedToAct->property("ArchivedVideoTier").toInt(&bHasValidVideoTier);
  if (!bHasValidVideoTier) {
    return false;
  }
  onArchiveTo(videoTier);
  return true;
}

int SceneListView::onArchiveTo(int videoTier) {
  if (!VideoTierTool::isVideoTierValid(videoTier)) {
    LOG_WARN_P("Cannot archive to videoTier", "VideoTier[%d]", videoTier);
    return -1;
  }
  const QModelIndexList& indexes{selectedRowsSource()};
  QStringList movieTier2Jsons[(int)VideoTierTool::VideoTierE::BUTT_INVALID];
  movieTier2Jsons[videoTier] = _sceneModel->rel2fileNames(indexes);
  return ArchiveToCore(indexes, movieTier2Jsons);
}

int SceneListView::onArchiveToByMovieRate() {
  const QModelIndexList& indexes{selectedRowsSource()};
  QModelIndexList nonRate0Indexes;
  const std::array<QStringList, RateHelper::BUTT_V>& movieRate2JsonsArr = _sceneModel->movieRate2Jsons(indexes, nonRate0Indexes);

  using namespace VideoTierTool;
  int needArchiveJsonFileCount = 0;
  QStringList movieTier2Jsons[(int)VideoTierE::BUTT_INVALID];
  for (int movieRate = (int)RateHelper::MOVIE_RATE_VALUE::MIN_V; movieRate < (int)RateHelper::MOVIE_RATE_VALUE::BUTT_V; ++movieRate) {
    const VideoTierE videoTier = MovieRate2VideoTierE(movieRate);
    if (!isVideoTierValid(videoTier)) {
      continue;
    }
    const QStringList& jsonFileNames = movieRate2JsonsArr[movieRate];
    if (jsonFileNames.isEmpty()) {
      continue;
    }
    needArchiveJsonFileCount += jsonFileNames.size();

    movieTier2Jsons[(int)videoTier] += jsonFileNames;
  }
  if (needArchiveJsonFileCount == 0) {
    LOG_INFO_NP("No need archive", "None of the selected movies have ratings within the valid VideoTier range");
    return 0;
  }

  return ArchiveToCore(nonRate0Indexes, movieTier2Jsons);
}

int SceneListView::ArchiveToCore(const QModelIndexList& indexes, const QStringList (&movieTier2Jsons)[(int)VideoTierTool::VideoTierE::BUTT_INVALID]) {
  if (indexes.isEmpty()) {
    LOG_INFO_NP("Skip", "No items selected");
    return 0;
  }
  using namespace VideoTierTool;

  QStringList allFilesNeedArchive;
  FileOperatorType::BATCH_COMMAND_LIST_TYPE archiveCmds;

  const QString& jsonLocatedInPath{_sceneModel->rootPath()};
  QString tierFolderLocatedIn;
  QString parentFolderName = PathTool::GetPrepathAndFileName(jsonLocatedInPath, tierFolderLocatedIn);
  for (int videoTier = (int)VideoTierE::BEGIN; videoTier < (int)VideoTierE::BUTT_INVALID; ++videoTier) {
    const QStringList& jsonFileNames = movieTier2Jsons[videoTier];
    if (jsonFileNames.isEmpty()) {
      continue;
    }
    QString tierFolderName = VideoTierTool::GetArchiveToFolderName(parentFolderName, static_cast<VideoTierE>(videoTier));
    QString tierFolderAbsPath = PathTool::Path2Join(tierFolderLocatedIn, tierFolderName);
    if (!QFileInfo{tierFolderAbsPath}.isDir()) {
      archiveCmds.append(FileOperatorType::ACMD::GetInstMKDIR(tierFolderLocatedIn, tierFolderName));
    }
    const QStringList& filesNeedArchive = BatchRenameBy::GetFilesNeedProcess(jsonLocatedInPath, jsonFileNames);
    allFilesNeedArchive += filesNeedArchive;
    for (const auto& nm : filesNeedArchive) {
      archiveCmds.append(FileOperatorType::ACMD::GetInstMV(jsonLocatedInPath, nm, tierFolderAbsPath));
    }
  }
  if (allFilesNeedArchive.isEmpty()) {
    LOG_INFO_NP("Skip", "No files need archive");
    return 0;
  }

  if (!RecycleCfmDlg::archiveQuestion(jsonLocatedInPath, allFilesNeedArchive, tierFolderLocatedIn + "/FolderXYZ Level")) {
    LOG_INFO_P("[Cancel] User cancel archive", "%d item(s) no change", allFilesNeedArchive.size());
    return 0;
  }
  // "Avoid file blocking; stop playback before archiving."
  emit requestStopMediaPlay();
  bool bAllSucceed = UndoRedo::GetInst().Do(archiveCmds);
  const int archiveRowCnt = _sceneModel->AfterJsonFilesNameRenamed(indexes);
  LOG_OE_P(bAllSucceed, "Archive", "Archive [%d] json/img/video items, rows[%d]", allFilesNeedArchive.size(), archiveRowCnt);
  return allFilesNeedArchive.size();
}
