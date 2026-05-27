#include "ImgReorderListView.h"
#include "PublicMacro.h"
#include "NotificatorMacro.h"
#include "InputDialogHelper.h"
#include "FileTool.h"
#include <QDropEvent>
#include <QMimeData>

ImgReorderListView::ImgReorderListView(QWidget* parent)
  : CustomListView{"IMG_REORDER_LISTVIEW", parent} {
  mImgReorderListModel = new ImgReorderListModel{"IMG_REORDER_LISTVIEW", this};
  setModel(mImgReorderListModel);
  setAcceptDrops(true);
  setDragEnabled(true);
  setDropIndicatorShown(true);
  setDragDropMode(QAbstractItemView::DragDrop);

  mBatchShiftRight100 = new (std::nothrow) QAction{QIcon{":img/SHIFT_RIGHT_BY_STEP"}, tr("Move Right by 100"), this};
  mBatchShiftLeft100 = new (std::nothrow) QAction{QIcon{":img/SHIFT_LEFT_BY_STEP"}, tr("Move Left by 100"), this};
  mBatchShiftCustomUnit = new (std::nothrow) QAction{tr("Move by Custom Step"), this};
  mNormalizeKeepRelativeOrder = new (std::nothrow) QAction{QIcon{":img/NOMARLIZE_KEEP_RELATIVE_ORDER"}, tr("Normalize keep relative order"), this};
  mOpenInSystemApplication = new (std::nothrow) QAction{QIcon{":img/SYSTEM_APPLICATION"}, tr("Open in System Application"), this};

  QList<QAction*> acts{mBatchShiftRight100, mBatchShiftLeft100, mBatchShiftCustomUnit, mNormalizeKeepRelativeOrder, mOpenInSystemApplication};
  PushFrontExclusiveActions(acts);
  PushBackExclusiveActions(mImgReorderListModel->GetExcusiveActions());

  subscribe();
}

bool ImgReorderListView::setImagesToReorder(const QStringList& imgs, const QString& baseName, int startIndex, const QString& namePattern) {
  return mImgReorderListModel->setImagesToReorder(imgs, baseName, startIndex, namePattern);
}

QStringList ImgReorderListView::getOrderedNames() const {
  return mImgReorderListModel->getOrderedNames();
}

void ImgReorderListView::subscribe() {
  connect(this, &QListView::doubleClicked, this, &ImgReorderListView::onOpenCurrentIndexInSystemApplication);
  connect(mOpenInSystemApplication, &QAction::triggered, this, &ImgReorderListView::onOpenCurrentIndexInSystemApplication);
  connect(this, &QListView::iconSizeChanged, mImgReorderListModel, &QAbstractListModelPub::onIconSizeChange);
  connect(mBatchShiftRight100, &QAction::triggered, this, [this]() { onBatchShiftSelectedRowsByStep(100); });
  connect(mBatchShiftLeft100, &QAction::triggered, this, [this]() { onBatchShiftSelectedRowsByStep(-100); });
  connect(mBatchShiftCustomUnit, &QAction::triggered, this, &ImgReorderListView::onBatchShiftCustomUnit);
  connect(mNormalizeKeepRelativeOrder, &QAction::triggered, this, &ImgReorderListView::onNormalizeKeepRelativeOrder);
}

void ImgReorderListView::dropEvent(QDropEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  const QMimeData* mimeData = event->mimeData();
  if (!mimeData || !mimeData->hasFormat(ImgReorderListModel::MIME_TYPE)) {
    QListView::dropEvent(event);
    return;
  }

  // 计算释放位置
  const QPoint pos{event->pos()};
  QModelIndex index{indexAt(pos)};

  int row = -1;
  int column = -1;

  if (index.isValid()) {
    // 在列表项上释放
    row = index.row();
    column = index.column();
  } else {
    // 在空白区域释放，计算应该插入的位置
    // 在IconMode下，我们可以根据位置计算应该插入到哪个位置
    row = calculateInsertionRow(pos);
  }

  // 调用模型的dropMimeData
  if (mImgReorderListModel->dropMimeData(mimeData, event->dropAction(), row, column, {})) {
    event->acceptProposedAction();
  } else {
    event->ignore();
  }
}

int ImgReorderListView::calculateInsertionRow(const QPoint& pos) const {
  const int N{model()->rowCount()};
  // 如果没有项，直接插入到最后
  if (N == 0) {
    return N;
  }
  const QModelIndex& ind{indexAt(pos)};
  if (!ind.isValid()) {
    return N;
  }
  return ind.row();
}

void ImgReorderListView::initExclusivePreferenceSetting() {
  CustomListView::m_defaultFlowLeft2Right = true;
  CustomListView::m_defaultViewModeIcon = true;
  CustomListView::m_defaultWrapping = true;
}

bool ImgReorderListView::onBatchShiftSelectedRowsByStep(int step) {
  const QModelIndexList& indexes = selectedIndexes();
  if (indexes.isEmpty()) {
    return false;
  }
  return mImgReorderListModel->onBatchShiftSelectedRowsByStep(indexes, step);
}

bool ImgReorderListView::onBatchShiftCustomUnit() {
  bool bUserAccept{false};
  int shiftValue{0};
  const QString labelText{"Enter a non‑zero step:\n"
                          " Positive: move right\n"
                          " Negative: move left"};
  std::tie(bUserAccept, shiftValue) = InputDialogHelper::GetIntWithInitial(this, "Move Images", labelText, 0, -10000, 10000, 1);

  if (!bUserAccept) {
    LOG_INFO_NP("Skip", "User cancelled move operation");
    return false;
  }

  return onBatchShiftSelectedRowsByStep(shiftValue);
}

bool ImgReorderListView::onNormalizeKeepRelativeOrder() {
  return mImgReorderListModel->onNormalizeKeepRelativeOrder();
}

bool ImgReorderListView::onOpenCurrentIndexInSystemApplication() const {
  const QModelIndex& ind{currentIndex()};
  if (!ind.isValid()) {
    return false;
  }
  const QString& filePath = mImgReorderListModel->filePath(ind);
  return FileTool::OpenLocalFileUsingDesktopService(filePath);
}
