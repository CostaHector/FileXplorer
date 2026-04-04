#ifndef BOOL2QTENUM_H
#define BOOL2QTENUM_H

#include <QtCore>
#include <QListView>

namespace Bool2QtEnum {
inline Qt::SortOrder toSortOrder(bool bReverseOrder) {
  return bReverseOrder ? Qt::SortOrder::DescendingOrder : Qt::SortOrder::AscendingOrder;
}
inline QListView::Flow toFlow(bool bLeft2Right) {
  return bLeft2Right ? QListView::Flow::LeftToRight : QListView::Flow::TopToBottom;
}
inline QListView::ViewMode toViewMode(bool bIconMode) {
  return bIconMode ? QListView::ViewMode::IconMode : QListView::ViewMode::ListMode;
}
inline QListView::ResizeMode toResizeMode(bool bAdjust) {
  return bAdjust ? QListView::ResizeMode::Adjust : QListView::ResizeMode::Fixed;
}
}  // namespace Bool2QtEnum

namespace QtEnum2Bool {
inline bool fromSortOrder(Qt::SortOrder bReverseOrder) {
  return bReverseOrder == Qt::SortOrder::DescendingOrder;
}
inline bool fromFlow(QListView::Flow bLeft2Right) {
  return bLeft2Right == QListView::Flow::LeftToRight;
}
inline bool fromViewMode(QListView::ViewMode bIconMode) {
  return bIconMode == QListView::ViewMode::IconMode;
}
inline bool fromResizeMode(QListView::ResizeMode bAdjust) {
  return bAdjust == QListView::ResizeMode::Adjust;
}
}  // namespace Bool2QtEnum

#endif  // BOOL2QTENUM_H
