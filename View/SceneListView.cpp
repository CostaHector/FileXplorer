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

#include <QHeaderView>
#include <QMessageBox>
#include <QMouseEvent>

enum class VideoTierE {
  INVALID, ESSENTIALS, NORMALS, CASUALS, DISPOSABLE,
};

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

  _ARCHIVE_ESSENTIALS = new QAction{"Archived to Essentials", this};
  _ARCHIVE_ESSENTIALS->setProperty("ArchivedVideoTier", (int)VideoTierE::ESSENTIALS);
  _ARCHIVE_NORMALS = new QAction{"Archived to Normals", this};
  _ARCHIVE_NORMALS->setProperty("ArchivedVideoTier", (int)VideoTierE::NORMALS);
  _ARCHIVE_CASUALS = new QAction{"Archived to Casuals", this};
  _ARCHIVE_CASUALS->setProperty("ArchivedVideoTier", (int)VideoTierE::CASUALS);
  _ARCHIVE_DISPOSABLE = new QAction{"Archived to Disposable", this};
  _ARCHIVE_DISPOSABLE->setProperty("ArchivedVideoTier", (int)VideoTierE::DISPOSABLE);

  _ARCHIVE_AG = new QActionGroup(this);
  _ARCHIVE_AG->addAction(_ARCHIVE_ESSENTIALS);
  _ARCHIVE_AG->addAction(_ARCHIVE_NORMALS);
  _ARCHIVE_AG->addAction(_ARCHIVE_CASUALS);
  _ARCHIVE_AG->addAction(_ARCHIVE_DISPOSABLE);

  QList<QAction*> exclusiveActions{
      SceneInPageActions::GetInst()._CREATE_THUMBNAIL_FOR_JSON_RELATED_IMGS,
      _RENAME_SCENE_RELATED_FILES_REPLACE,  //
      _RENAME_SCENE_RELATED_FILES_INSERT,   //
      _RENAME_SCENE_RELATED_FILES_NUMERIZE, //
  };
  exclusiveActions += _ARCHIVE_AG->actions();
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
  connect(_ARCHIVE_AG, &QActionGroup::triggered, this, &SceneListView::onArchiveActionTriggered);

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

VideoTierE MovieRate2VideoTierE(int movieRate) {
  switch (movieRate) {
    case 0:
      return VideoTierE::INVALID;
    case 1:
    case 2:
    case 3:
      return VideoTierE::DISPOSABLE;
    case 4:
    case 5:
      return VideoTierE::CASUALS;
    case 6:
    case 7:
      return VideoTierE::NORMALS;
    case 8:
    case 9:
    case 10:
      return VideoTierE::ESSENTIALS;
    default:
      return VideoTierE::INVALID;
  }
}

QString TierNameStr(int videoTier) {
  switch (videoTier) {
    case (int)VideoTierE::ESSENTIALS:
      return "Essentials";
    case (int)VideoTierE::NORMALS:
      return "Normal";
    case (int)VideoTierE::CASUALS:
      return "Casuals";
    case (int)VideoTierE::DISPOSABLE:
      return "Disposable";
    default:
      return "invalid";
  }
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
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("nothing selected", "skip recycle");
    return 0;
  }

  const QString& jsonLocatedInPath{_sceneModel->rootPath()};
  const QModelIndexList& indexes{selectedRowsSource()};
  const QStringList& jsonFileNames{_sceneModel->rel2fileNames(indexes)};

  QString tierFolderLocatedIn;
  QString tierFolderName = PathTool::GetPrepathAndFileName(_sceneModel->rootPath(), tierFolderLocatedIn) + " " + TierNameStr(videoTier);
  QString tierFolderAbsPath = PathTool::Path2Join(tierFolderLocatedIn, tierFolderName);

  const QStringList& filesNeedArchive = BatchRenameBy::GetFilesNeedProcess(jsonLocatedInPath, jsonFileNames);
  const int relatedFilesCnt{filesNeedArchive.size()};

  if (!RecycleCfmDlg::archiveQuestion(jsonLocatedInPath, filesNeedArchive, tierFolderAbsPath)) {
    LOG_INFO_P("[Cancel] User cancel archive", "%d item(s) no change", relatedFilesCnt);
    return 0;
  }

  FileOperatorType::BATCH_COMMAND_LIST_TYPE archiveCmds;
  archiveCmds.reserve(1 + relatedFilesCnt);
  if (!QFileInfo{tierFolderAbsPath}.isDir()) {
    archiveCmds.append(FileOperatorType::ACMD::GetInstMKDIR(tierFolderLocatedIn, tierFolderName));
  }
  for (const auto& nm : filesNeedArchive) {
    archiveCmds.append(FileOperatorType::ACMD::GetInstMV(jsonLocatedInPath, nm, tierFolderAbsPath));
  }
  bool bAllSucceed = UndoRedo::GetInst().Do(archiveCmds);
  const int archiveRowCnt = _sceneModel->AfterJsonFilesNameRenamed(indexes);
  LOG_OE_P(bAllSucceed, "Archive", "Archive %d json/img/video items, rows[%d]", relatedFilesCnt, archiveRowCnt);
  return relatedFilesCnt;
}
