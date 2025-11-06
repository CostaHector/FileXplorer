#include "SceneListView.h"
#include "NotificatorMacro.h"
#include "ScenesListModel.h"
#include "PlayVideo.h"
#include "PublicMacro.h"
#include "PathTool.h"
#include "SceneInPageActions.h"
#include "SceneInfoManager.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
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

  _sceneSortProxyModel->setSourceModel(_sceneModel);
  const SceneInPageActions& sceneActInst = g_SceneInPageActions();
  const auto& sortPr = sceneActInst.GetSortSetting();
  _sceneSortProxyModel->sortByFieldDimension(sortPr.first, sortPr.second);

  setModel(_sceneSortProxyModel);
  setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectItems);
  setViewMode(QListView::ViewMode::IconMode);
  setTextElideMode(Qt::TextElideMode::ElideLeft);
  setResizeMode(QListView::ResizeMode::Fixed);
  setWrapping(true);

  mAlignDelegate = new (std::nothrow) SceneStyleDelegate{this};
  CHECK_NULLPTR_RETURN_VOID(mAlignDelegate)
  setItemDelegate(mAlignDelegate);

  QMenu* m_menu = new (std::nothrow) QMenu{"Scene list view menu", this};
  CHECK_NULLPTR_RETURN_VOID(m_menu)
  COPY_BASENAME_FROM_SCENE = new (std::nothrow) QAction{QIcon(":img/COPY_TEXT"), "Copy basename", m_menu};
  CHECK_NULLPTR_RETURN_VOID(COPY_BASENAME_FROM_SCENE)
  OPEN_CORRESPONDING_FOLDER = new (std::nothrow) QAction{QIcon(":img/PLAY_BUTTON_ROUND"), "Play this folder", m_menu};
  CHECK_NULLPTR_RETURN_VOID(OPEN_CORRESPONDING_FOLDER)

  m_menu->addAction(COPY_BASENAME_FROM_SCENE);
  m_menu->addAction(OPEN_CORRESPONDING_FOLDER);
  BindMenu(m_menu);
  subscribe();

  // setMouseTracking(true);
  // setAttribute(Qt::WA_Hover, true);
  // viewport()->setMouseTracking(true);
}

bool SceneListView::onCopyBaseName() {
  const QModelIndex& curInd = currentIndex();
  if (!curInd.isValid()) {
    return false;
  }
  const QModelIndex& srcInd = _sceneSortProxyModel->mapToSource(curInd);
  const QString& copiedStr = _sceneModel->baseName(srcInd);
  auto* cb = QApplication::clipboard();
  cb->setText(copiedStr, QClipboard::Mode::Clipboard);
  LOG_D("user copied str: [%s]", qPrintable(copiedStr));
  return true;
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
  connect(COPY_BASENAME_FROM_SCENE, &QAction::triggered, this, &SceneListView::onCopyBaseName);
  connect(OPEN_CORRESPONDING_FOLDER, &QAction::triggered, this, &SceneListView::onOpenCorrespondingFolder);
  connect(selectionModel(), &QItemSelectionModel::currentRowChanged, this, &SceneListView::onClickEvent);
  connect(this, &CustomListView::iconSizeChanged, _sceneModel, &ScenesListModel::onIconSizeChange);

  connect(_scenePageControl, &ScenePageControl::currentPageIndexChanged, _sceneModel, &ScenesListModel::onPageIndexChanged);
  connect(_scenePageControl, &ScenePageControl::maxScenesCountPerPageChanged, _sceneModel, &ScenesListModel::onScenesCountsPerPageChanged);
  connect(_sceneModel, &ScenesListModel::pagesCountChanged, _scenePageControl, &ScenePageControl::onPagesCountChanged);

  SceneInPageActions& sceneActInst = g_SceneInPageActions();
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
  LOG_OK_P("Json file K-V updated", "updated:%d, used:%d\nimgUpdate:%d, vidUpdate:%d\nunder path[%s]",  //
           cnt.m_jsonUpdatedCnt, cnt.m_jsonUsedCnt,                                                     //
           cnt.m_ImgNameKeyFieldUpdatedCnt, cnt.m_VidNameKeyFieldUpdatedCnt,                            //
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

bool SceneListView::onClickEvent(const QModelIndex& current, const QModelIndex& previous) {
  if (!current.isValid()) {
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

void SceneListView::mousePressEvent(QMouseEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  const QPoint pos = event->pos();
  const QModelIndex proIndex = indexAt(pos);
  const QModelIndex srcIndex = _sceneSortProxyModel->mapToSource(proIndex);

  const QRect imageRect{mAlignDelegate->GetRealImageVisualRect(proIndex, visualRect(proIndex))};

  emit sceneGridClicked(proIndex, imageRect, pos);
  QListView::mousePressEvent(event);
}

void SceneListView::onCellVisualUpdateRequested(const QModelIndex& ind) {
  if (!ind.isValid()) {
    LOG_W("index invalid, no need visual update at all");
    return;
  }
  update(ind);
}
