#include "SceneListView.h"
#include "NotificatorMacro.h"
#include "ScenesListModel.h"
#include "PlayVideo.h"
#include "PublicMacro.h"
#include "SceneInPageActions.h"
#include "SceneInfoManager.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QMouseEvent>

void AlignDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const {
  option->decorationPosition = QStyleOptionViewItem::Position::Top;
  option->decorationAlignment = Qt::AlignmentFlag::AlignHCenter;
  option->textElideMode = Qt::TextElideMode::ElideLeft;
  option->displayAlignment = Qt::AlignmentFlag::AlignVCenter;
  QStyledItemDelegate::initStyleOption(option, index);
}

QString AlignDelegate::displayText(const QVariant& value, const QLocale& /**/) const {
  const QString& text = value.toString();
  static constexpr int CHAR_LETTER_CNT = 40;
  if (text.size() <= CHAR_LETTER_CNT) {
    return text;
  }
  return text.left(CHAR_LETTER_CNT / 2) + "\n" + text.right(CHAR_LETTER_CNT / 2);
}

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

  setModel(_sceneSortProxyModel);
  setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectItems);
  setViewMode(QListView::ViewMode::IconMode);
  setTextElideMode(Qt::TextElideMode::ElideMiddle);
  setResizeMode(QListView::ResizeMode::Fixed);
  setWrapping(true);

  mAlignDelegate = new (std::nothrow) AlignDelegate;
  CHECK_NULLPTR_RETURN_VOID(mAlignDelegate)
  setItemDelegate(mAlignDelegate);

  QMenu* m_menu = new (std::nothrow) QMenu{"Scene list view menu", this};
  CHECK_NULLPTR_RETURN_VOID(m_menu)
  COPY_BASENAME_FROM_SCENE = new (std::nothrow) QAction{"copy basename", m_menu};
  CHECK_NULLPTR_RETURN_VOID(COPY_BASENAME_FROM_SCENE)
  OPEN_CORRESPONDING_FOLDER = new (std::nothrow) QAction{"play this folder", m_menu};
  CHECK_NULLPTR_RETURN_VOID(OPEN_CORRESPONDING_FOLDER)

  m_menu->addAction(COPY_BASENAME_FROM_SCENE);
  m_menu->addAction(OPEN_CORRESPONDING_FOLDER);
  BindMenu(m_menu);
  subscribe();
}

void SceneListView::onCopyBaseName() {
  const QModelIndex& curInd = currentIndex();
  const QString& copiedStr = _sceneModel->baseName(curInd);
  auto* cb = QApplication::clipboard();
  cb->setText(copiedStr, QClipboard::Mode::Clipboard);
  LOG_D("user copied str: [%s]", qPrintable(copiedStr));
}

void SceneListView::onOpenCorrespondingFolder() {
  const QModelIndex& curInd = currentIndex();
  const QString& scenePath = _sceneModel->absolutePath(curInd);
  on_ShiftEnterPlayVideo(scenePath);
  LOG_D("Play path: [%s]", qPrintable(scenePath));
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
  connect(sceneActInst._COMBINE_MEDIAINFOS_JSON, &QAction::triggered, this, &SceneListView::onUpdateScnFiles);
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
  ScnMgr scnMgr;
  Counter cnter = scnMgr(workPath);
  if (cnter.m_jsonUpdatedCnt == 0) {
    LOG_INFO_NP("No scene file need updated", "0 json(s) updated");
    return 0;
  }
  int scnFileCnt = scnMgr.WriteDictIntoScnFiles();
  LOG_OE_P(scnFileCnt >= 0, "Scn file updated", "count: %d, workPath[%s]", scnFileCnt, qPrintable(workPath));
  _sceneModel->setRootPath(workPath, true);
  return scnFileCnt;
}

void SceneListView::onClickEvent(const QModelIndex& current, const QModelIndex& previous) {
  if (!current.isValid()) {
    return;
  }
  const QString& name = _sceneModel->baseName(current);
  emit currentSceneChanged(name, _sceneModel->GetImgs(current), _sceneModel->GetVids(current));
}

bool SceneListView::IsPathAtShallowDepth(const QString& path) {
#ifdef _WIN32
  static constexpr int NEAR_ROOT_PATH_LIMIT = 2;  // windows path start with disk letter
#else
  static constexpr int NEAR_ROOT_PATH_LIMIT = 2;  // linux path start with '/'
#endif
  return path.count('/') < NEAR_ROOT_PATH_LIMIT;
}
