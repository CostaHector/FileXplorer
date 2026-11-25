#include "CurrentIndexGuard.h"
#include <QAbstractItemView>
#include <QItemSelection>

CurrentIndexGuard::CurrentIndexGuard(QAbstractItemView &view_): _view{view_} {
  QModelIndex oldCurInd = _view.currentIndex();
  if (oldCurInd.isValid()) {
    m_oldCurrentIndexRow = oldCurInd.row();
    m_oldCurrentIndexColumn = oldCurInd.column();
  }
  if (auto* pSelectionModel = _view.selectionModel()) {
    const QItemSelection& selectRangeList = pSelectionModel->selection();
    for (const auto& range: selectRangeList) {
      m_oldRangeList.push_back(CellRange{range.left(), range.top(), range.right(), range.bottom()});
    }
  }
}

CurrentIndexGuard::~CurrentIndexGuard() {
  if (!mEnabled) {
    return;
  }
  if (const auto* pModel = _view.model()) {
    const QModelIndex rootIndex = _view.rootIndex();
    if (auto* pSelectionModel = _view.selectionModel()) {
      QItemSelection rectSelList;
      for (const CellRange& rect: m_oldRangeList) {
        QModelIndex leftTop = pModel->index(rect.top, rect.left, rootIndex);
        QModelIndex rightBottom = pModel->index(rect.bottom, rect.right, rootIndex);
        rectSelList.select(leftTop, rightBottom);
      }
      pSelectionModel->select(rectSelList, QItemSelectionModel::ClearAndSelect);
    }
    if (m_oldCurrentIndexRow >= 0 && m_oldCurrentIndexColumn >= 0) {
      QModelIndex newCurIndex = pModel->index(m_oldCurrentIndexRow, m_oldCurrentIndexColumn, rootIndex);
      if (newCurIndex.isValid()) {
        _view.setCurrentIndex(newCurIndex);
        _view.scrollTo(newCurIndex, QAbstractItemView::EnsureVisible);
      }
    }
  }
}
