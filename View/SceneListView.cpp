#include "SceneListView.h"
#include "Actions/ViewActions.h"
#include "Model/ScenesListModel.h"
#include "Tools/PlayVideo.h"
#include "public/PublicMacro.h"
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

SceneListView::SceneListView(ScenesListModel* sceneModel, QWidget* parent)  //
    : CustomListView{"SCENES_TABLE", parent},                               //
      _sceneModel{sceneModel}                                               //
{
  if (_sceneModel == nullptr) {
    qCritical("sceneModel is nullptr");
    return;
  } else {
    setModel(_sceneModel);
  }
  setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectItems);
  setViewMode(QListView::ViewMode::ListMode);
  setTextElideMode(Qt::TextElideMode::ElideMiddle);
  setUniformItemSizes(false);

  setResizeMode(QListView::ResizeMode::Adjust);
  setMovement(QListView::Movement::Free);
  setWrapping(true);
  setFlow(QListView::Flow::LeftToRight);

  mAlignDelegate = new (std::nothrow) AlignDelegate;
  CHECK_NULLPTR_RETURN_VOID(mAlignDelegate)
  setItemDelegate(mAlignDelegate);

  m_fPrev = new (std::nothrow) FloatingPreview{"FloatingList", this};
  CHECK_NULLPTR_RETURN_VOID(m_fPrev)

  QMenu* m_menu = new (std::nothrow) QMenu{"scene table view menu", this};
  CHECK_NULLPTR_RETURN_VOID(m_menu)
  COPY_BASENAME_FROM_SCENE = new (std::nothrow) QAction("copy basename", m_menu);
  CHECK_NULLPTR_RETURN_VOID(COPY_BASENAME_FROM_SCENE)
  OPEN_CORRESPONDING_FOLDER = new (std::nothrow) QAction("play this folder", m_menu);
  CHECK_NULLPTR_RETURN_VOID(OPEN_CORRESPONDING_FOLDER)

  m_menu->addAction(COPY_BASENAME_FROM_SCENE);
  m_menu->addAction(OPEN_CORRESPONDING_FOLDER);
  m_menu->addSeparator();
  m_menu->addActions(_ORIENTATION_GRP->actions());
  BindMenu(m_menu);
  subscribe();

  setMouseTracking(g_viewActions()._FLOATING_PREVIEW->isChecked());
}

void SceneListView::onCopyBaseName() {
  const QModelIndex& curInd = currentIndex();
  const QString& copiedStr = _sceneModel->baseName(curInd);
  auto* cb = QApplication::clipboard();
  cb->setText(copiedStr, QClipboard::Mode::Clipboard);
  qDebug("user copied str: [%s]", qPrintable(copiedStr));
}

void SceneListView::onOpenCorrespondingFolder() {
  const QModelIndex& curInd = currentIndex();
  const QString& scenePath = _sceneModel->absolutePath(curInd);
  on_ShiftEnterPlayVideo(scenePath);
  qDebug("Play path: [%s]", qPrintable(scenePath));
}

void SceneListView::subscribe() {
  connect(COPY_BASENAME_FROM_SCENE, &QAction::triggered, this, &SceneListView::onCopyBaseName);
  connect(OPEN_CORRESPONDING_FOLDER, &QAction::triggered, this, &SceneListView::onOpenCorrespondingFolder);
  connect(g_viewActions()._FLOATING_PREVIEW, &QAction::triggered, this, &SceneListView::setMouseTracking);
}

void SceneListView::setRootPath(const QString& rootPath) {
  if (rootPath.count('/') < 2) {  // large folder
    qDebug("rootPath[%s] may contains a large item(s)", qPrintable(rootPath));
    const auto ret = QMessageBox::warning(this, "Large folder alert(May cause LAG)", rootPath, QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No, QMessageBox::StandardButton::No);
    if (ret != QMessageBox::StandardButton::Yes) {
      qDebug("User cancel setRootPath on a large path[%s]", qPrintable(rootPath));
      return;
    }
  }
  _sceneModel->setRootPath(rootPath);
}

void SceneListView::mouseMoveEvent(QMouseEvent* event) {
  if (m_fPrev == nullptr) {
    return;
  }
  const QPoint& pnt = event->pos();
  const QModelIndex& idx = indexAt(pnt);
  if (!idx.isValid()) {
    m_fPrev->SaveSettings();
    m_fPrev->hide();
    return;
  }
  const QString& name = _sceneModel->data(idx, Qt::ItemDataRole::DisplayRole).toString();
  if (!m_fPrev->NeedUpdate(name)) {
    return;
  }
  m_fPrev->move(event->globalPos() + QPoint{20, 20});
  if (m_fPrev->NeedUpdateImgs()) {
    m_fPrev->UpdateImgs(name, _sceneModel->GetImgs(idx));
  }
  if (m_fPrev->NeedUpdateVids()) {
    m_fPrev->UpdateVids(_sceneModel->GetVids(idx));
  }
  if (m_fPrev->NeedUpdateOthers()) {
    // todo:
  }
  if (m_fPrev->isHidden()) {
    m_fPrev->show();
  }
  m_fPrev->raise();
}
