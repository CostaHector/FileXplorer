#include "ImgReorderListView.h"
#include "PublicMacro.h"
#include <QDropEvent>
#include <QMimeData>

ImgReorderListView::ImgReorderListView(QWidget* parent) : CustomListView{"IMG_REORDER_LISTVIEW", parent} {
  mImgReorderListModel = new ImgReorderListModel{this};
  setModel(mImgReorderListModel);
  setAcceptDrops(true);
  setDragEnabled(true);
  setDropIndicatorShown(true);
  setDragDropMode(QAbstractItemView::DragDrop);

  _FLOW_ORIENTATION_TTB->setChecked(false);
  _VIEW_MODE_LIST_ICON->setChecked(true);
  _WRAPING_ACTIONS->setChecked(true);
  _UNIFORM_ITEM_SIZES->setChecked(false);

  connect(this, &QListView::iconSizeChanged, mImgReorderListModel, &QAbstractListModelPub::onIconSizeChange);
  setWindowIcon(QIcon{":img/RENAME_REORDER_LISTVIEW"});
  setWindowTitle("Drag to reorder images names");
}

bool ImgReorderListView::setImagesToReorder(const QStringList& imgs, const QString& baseName, int startIndex, const QString& namePattern) {
  return mImgReorderListModel->setImagesToReorder(imgs, baseName, startIndex, namePattern);
}

QStringList ImgReorderListView::getOrderedNames() const {
  return mImgReorderListModel->getOrderedNames();
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
