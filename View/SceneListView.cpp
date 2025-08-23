#include "SceneListView.h"
#include "ScenesListModel.h"
#include "PlayVideo.h"
#include "PublicMacro.h"
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

  QMenu* m_menu = new (std::nothrow) QMenu{"scene table view menu", this};
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

void SceneListView::setFloatingPreview(FloatingPreview* floatingPreview) {
  mPrev_ = floatingPreview;
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
  connect(this, &QListView::clicked, this, &SceneListView::onClickEvent);
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

void SceneListView::onClickEvent(const QModelIndex& idx) {
  CHECK_NULLPTR_RETURN_VOID(mPrev_)
  if (!idx.isValid()) {
    return;
  }
  const QString& name = _sceneModel->baseName(idx);
  if (!mPrev_->NeedUpdate(name)) {
    return;
  }
  mPrev_->BeforeDisplayAFolder();
  mPrev_->UpdateImgs(name, _sceneModel->GetImgs(idx));
  mPrev_->UpdateVids(_sceneModel->GetVids(idx));
}
