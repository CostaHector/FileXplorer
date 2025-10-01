#include "SceneListView.h"
#include "ScenesListModel.h"
#include "PlayVideo.h"
#include "PublicMacro.h"
#include "SceneInPageActions.h"
#include <QStyledItemDelegate>
#include <QHeaderView>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QMouseEvent>

class AlignDelegate : public QStyledItemDelegate {
 public:
  void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override {
    option->decorationPosition = QStyleOptionViewItem::Position::Top;
    option->decorationAlignment = Qt::AlignmentFlag::AlignHCenter;
    option->textElideMode = Qt::TextElideMode::ElideLeft;
    option->displayAlignment = Qt::AlignmentFlag::AlignVCenter;
    QStyledItemDelegate::initStyleOption(option, index);
  }

  QString displayText(const QVariant& value, const QLocale& /**/) const override {
    const QString& text = value.toString();
    static constexpr int CHAR_LETTER_CNT = 40;
    if (text.size() <= CHAR_LETTER_CNT) {
      return text;
    }
    return text.left(CHAR_LETTER_CNT / 2) + "\n" + text.right(CHAR_LETTER_CNT / 2);
  }
};

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
}

void SceneListView::setRootPath(const QString& rootPath) {
  if (rootPath.count('/') < 2) {  // large folder
    LOG_D("rootPath[%s] may contains a large item(s)", qPrintable(rootPath));
    const auto ret = QMessageBox::warning(this, "Large folder alert(May cause LAG)", rootPath,
                                          QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No, QMessageBox::StandardButton::No);
    if (ret != QMessageBox::StandardButton::Yes) {
      LOG_D("User cancel setRootPath on a large path[%s]", qPrintable(rootPath));
      return;
    }
  }
  _sceneModel->setRootPath(rootPath);
}

void SceneListView::onClickEvent(const QModelIndex& current, const QModelIndex& previous) {
  if (!current.isValid()) {
    return;
  }
  const QString& name = _sceneModel->baseName(current);
  emit currentSceneChanged(name, _sceneModel->GetImgs(current), _sceneModel->GetVids(current));
}
