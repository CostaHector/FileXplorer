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

#include <QHeaderView>
#include <QMessageBox>
#include <QMouseEvent>

SceneListView::SceneListView(ScenesListModel* sceneModel,
                             SceneSortProxyModel* sceneSortProxyModel,
                             ScenePageControl* scenePageControl,
                             QWidget* parent)     //
    : CustomListView{"SCENES_TABLE", parent},     //
      _sceneModel{sceneModel},                    //
      _sceneSortProxyModel{sceneSortProxyModel},  //
      _scenePageControl{scenePageControl}         //
{
  CHECK_NULLPTR_RETURN_VOID(_sceneModel)
  CHECK_NULLPTR_RETURN_VOID(sceneSortProxyModel)
  CHECK_NULLPTR_RETURN_VOID(_scenePageControl)

  const SceneInPageActions& sceneActInst = g_SceneInPageActions();
  _sceneModel->initSortSetting(sceneActInst.GetSortDimension(), sceneActInst.GetSortOrderReverse());
  _sceneSortProxyModel->setSourceModel(_sceneModel);
  setModel(_sceneSortProxyModel);
  setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectItems);

  mAlignDelegate = new (std::nothrow) SceneStyleDelegate{this};
  CHECK_NULLPTR_RETURN_VOID(mAlignDelegate)
  setItemDelegate(mAlignDelegate);

  _RENAME_SCENE_RELATED_FILES_REPLACE = new (std::nothrow) QAction(QIcon(":img/RENAME"), tr("Rename related(replace)"), this);
  _RENAME_SCENE_RELATED_FILES_REPLACE->setShortcutVisibleInContextMenu(true);
  _RENAME_SCENE_RELATED_FILES_REPLACE->setToolTip(
      QString("<b>%1 (%2)</b><br/>Rename selected json file(s) and associated files by replacing a substring in the file names.")  //
          .arg(_RENAME_SCENE_RELATED_FILES_REPLACE->text())
          .arg(_RENAME_SCENE_RELATED_FILES_REPLACE->shortcut().toString()));

  _RENAME_SCENE_RELATED_FILES_INSERT = new (std::nothrow) QAction(QIcon(":img/NAME_STR_INSERTER_PATH"), tr("Rename related(Insert)"), this);
  _RENAME_SCENE_RELATED_FILES_INSERT->setShortcutVisibleInContextMenu(true);
  _RENAME_SCENE_RELATED_FILES_INSERT->setToolTip(
      QString("<b>%1 (%2)</b><br/>Rename selected json file(s) and associated files by inserting a string into the file names.")  //
          .arg(_RENAME_SCENE_RELATED_FILES_INSERT->text())
          .arg(_RENAME_SCENE_RELATED_FILES_INSERT->shortcut().toString()));

  _RENAME_SCENE_RELATED_FILES_NUMERIZE = new (std::nothrow) QAction(QIcon(":img/NAME_STR_NUMERIZER_PATH"), tr("Rename (ith)"), this);
  _RENAME_SCENE_RELATED_FILES_NUMERIZE->setShortcutVisibleInContextMenu(true);
  _RENAME_SCENE_RELATED_FILES_NUMERIZE->setToolTip(QString("<b>%1 (%2)</b><br/> Numerizer each file in a sequence.")
                                                       .arg(_RENAME_SCENE_RELATED_FILES_NUMERIZE->text())
                                                       .arg(_RENAME_SCENE_RELATED_FILES_NUMERIZE->shortcut().toString()));

  _RECYCLE_SCENE_RELATED_FILES = new (std::nothrow) QAction{QIcon{":img/MOVE_TO_TRASH_BIN"}, tr("Recycle related files"), this};
  CHECK_NULLPTR_RETURN_VOID(_RECYCLE_SCENE_RELATED_FILES)
  _RECYCLE_SCENE_RELATED_FILES->setToolTip(QString("<b>%1 (%2)</b><br/> Move selected scene related file(s) name to trash bin")  //
                                               .arg(_RECYCLE_SCENE_RELATED_FILES->text())
                                               .arg(_RECYCLE_SCENE_RELATED_FILES->shortcut().toString()));

  _OPEN_CORRESPONDING_FOLDER = new (std::nothrow) QAction{QIcon{":img/SYSTEM_APPLICATION_VIDEO"}, tr("Play this folder"), this};
  CHECK_NULLPTR_RETURN_VOID(_OPEN_CORRESPONDING_FOLDER)

  QList<QAction*> exclusiveActions{
      _RENAME_SCENE_RELATED_FILES_NUMERIZE,  //
      _RENAME_SCENE_RELATED_FILES_REPLACE,   //
      _RENAME_SCENE_RELATED_FILES_INSERT,    //
      _RECYCLE_SCENE_RELATED_FILES,          //
      _OPEN_CORRESPONDING_FOLDER,            //
  };
  PushFrontExclusiveActions(exclusiveActions);
  PushBackExclusiveActions(_sceneModel->GetExcusiveActions());

  subscribe();

  // setMouseTracking(true);
  // setAttribute(Qt::WA_Hover, true);
  // viewport()->setMouseTracking(true);
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
  SceneInPageActions& sceneActInst = g_SceneInPageActions();
  connect(&sceneActInst, &SceneInPageActions::sceneSortDimensionChanged, _sceneModel, &ScenesListModel::setSortDimension);
  connect(&sceneActInst, &SceneInPageActions::sceneSortReverseOrderChanged, _sceneModel, &ScenesListModel::setSortOrderReverse);
  connect(&sceneActInst, &SceneInPageActions::scenesSortPolicyChanged, _sceneSortProxyModel, &SceneSortProxyModel::sortByFieldDimension);
  connect(sceneActInst._UPDATE_JSON, &QAction::triggered, this, &SceneListView::onUpdateJsonFiles);
  connect(sceneActInst._UPDATE_SCN, &QAction::triggered, this, &SceneListView::onUpdateScnFiles);
  connect(sceneActInst._CLEAR_SCN_FILE, &QAction::triggered, this, &SceneListView::onClearScnFiles);

  connect(this, &SceneListView::sceneGridClicked, mAlignDelegate, &SceneStyleDelegate::onSceneClicked);
  connect(mAlignDelegate, &SceneStyleDelegate::cellVisualUpdateRequested, this, &SceneListView::onCellVisualUpdateRequested);
}

void SceneListView::setRootPath(const QString& rootPath) {
  if (IsPathAtShallowDepth(rootPath)) {  // Potential large directory
    LOG_D("Root path[%s] may contain a large number of items", qPrintable(rootPath));
    const QString cfmTitle = "Large Directory Warning - Performance Impact";
    const QString hintMsg =
        "This directory appears to be at a high level in the filesystem and may contain a large number of items. "
        "Loading it could cause performance issues.\n\n"
        "Directory: " +
        rootPath + "\n\n Do you want to proceed?";
    QMessageBox::StandardButton retBtn;
#ifdef RUNNING_UNIT_TESTS
    retBtn = SceneListViewMocker::MockSetRootPathQuery() ? QMessageBox::StandardButton::Yes : QMessageBox::StandardButton::No;
#else
    retBtn = QMessageBox::warning(this, cfmTitle, hintMsg, QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No,
                                  QMessageBox::StandardButton::No);
#endif
    if (retBtn != QMessageBox::StandardButton::Yes) {
      LOG_INFO_P("User canceled setting root path", "large directory:[%s]", qPrintable(rootPath));
      return;
    }
  }
  _sceneModel->setRootPath(rootPath);
}

int SceneListView::onUpdateJsonFiles() {
  const QString workPath = _sceneModel->rootPath();
  if (IsPathAtShallowDepth(workPath)) {
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
           "updated:%d, used:%d\nimgUpdate:%d, vidUpdate:%d\nunder path[%s]",  //
           cnt.m_jsonUpdatedCnt,
           cnt.m_jsonUsedCnt,  //
           cnt.m_ImgNameKeyFieldUpdatedCnt,
           cnt.m_VidNameKeyFieldUpdatedCnt,  //
           qPrintable(workPath));
  return cnt.m_jsonUpdatedCnt;
}

int SceneListView::onUpdateScnFiles() {
  const QString workPath = _sceneModel->rootPath();
  if (IsPathAtShallowDepth(workPath)) {
    LOG_ERR_P("Update aborted",
              "Path [%s] is too close to root directory. "
              "System files may get accidentally modified at this level.",
              qPrintable(workPath));
    return -1;
  }
  using namespace SceneInfoManager;
  int scnFileCnt = ScnMgr::UpdateScnFiles(workPath);
  LOG_OE_P(scnFileCnt >= 0, "Scn file updated", "count: %d, workPath[%s]", scnFileCnt, qPrintable(workPath));
  _sceneModel->setRootPath(workPath, true);
  return scnFileCnt;
}

int SceneListView::onClearScnFiles() {
  const QString workPath = _sceneModel->rootPath();
  using namespace SceneInfoManager;
  int deleteCnt = ScnMgr::ClearScnFiles(workPath);
  LOG_OK_P("Delete scn file", "cnt: %d under[%s]", deleteCnt, qPrintable(workPath));
  _sceneModel->setRootPath(workPath, true);
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

bool SceneListView::IsPathAtShallowDepth(const QString& path) {
#ifdef _WIN32
  static constexpr int NEAR_ROOT_PATH_LIMIT = 2;  // windows path start with disk letter
#else
  static constexpr int NEAR_ROOT_PATH_LIMIT = 2;  // linux path start with '/'
#endif
  return path.count('/') < NEAR_ROOT_PATH_LIMIT;
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
    const QModelIndex proIndex = indexAt(pos);  // here no need use mapToSource
    if (mLastClickedIndex != proIndex) {
      onClickEvent(proIndex);
    }
    mLastClickedIndex = proIndex;
    const QRect imageRect{mAlignDelegate->GetRealImageVisualRect(proIndex, visualRect(proIndex))};
    emit sceneGridClicked(proIndex, imageRect, pos);
  }
  QListView::mousePressEvent(event);
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
  const QStringList& filesNeedRecycle = BatchRenameBy::GetFilesNeedRename(jsonLocatedInPath, jsonFileNames);
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
