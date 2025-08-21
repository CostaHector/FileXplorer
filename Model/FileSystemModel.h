#ifndef FILESYSTEMMODEL_H
#define FILESYSTEMMODEL_H

#include <QFileSystemModel>
#include <QObject>
#include "CustomStatusBar.h"
#include "SelectionsRangeHelper.h"

class FileSystemModel : public QFileSystemModel {
public:
  explicit FileSystemModel(QObject* parent = nullptr);

  void BindLogger(CustomStatusBar* logger) const;

  QString fullInfo(const QModelIndex& curIndex) const;

  Qt::ItemFlags flags(const QModelIndex& index) const override;
  bool canItemsBeDragged(const QModelIndex& index) const;

  bool canItemsDroppedHere(const QModelIndex& index) const;

  bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const override;
  bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
  Qt::DropActions supportedDropActions() const override;
  Qt::DropActions supportedDragActions() const override;

  // cut/paste made file inexist so index not exist but still valid. Don't emit dataChanged for these index or crash down.
  // check if DoNotUseParent is true
  // if (!checkIndex(ind, CheckIndexOption::DoNotUseParent)) {
  //  emit dataChanged(ind, ind, {Qt::ItemDataRole::DecorationRole});
  // }

  void ClearCopyAndCutDict() {
    ClearCutDict();
    ClearCopiedDict();
  }
  void ClearCutDict() { mCutIndexes.clear(); }
  void ClearCopiedDict() { mCopyIndexes.clear(); }
  void CutSomething(const QModelIndexList& cutIndexes) {
    auto beRngLst = mCutIndexes.GetTopBottomRange();
    ClearCopyAndCutDict();
    mCutIndexes.Set(rootPath(), cutIndexes);
    beRngLst += mCutIndexes.GetTopBottomRange();
    foreach (auto beRng, beRngLst) {
      if (beRng.first.isValid() && beRng.second.isValid()) {
        emit dataChanged(beRng.first, beRng.second, {Qt::ItemDataRole::DecorationRole});
      }
    }
  }
  void CopiedSomething(const QModelIndexList& copiedIndexes) {
    auto beRngLst = mCutIndexes.GetTopBottomRange();
    ClearCopyAndCutDict();
    mCopyIndexes.Set(rootPath(), copiedIndexes);
    beRngLst += mCutIndexes.GetTopBottomRange();
    foreach (auto beRng, beRngLst) {
      if (beRng.first.isValid() && beRng.second.isValid()) {
        emit dataChanged(beRng.first, beRng.second, {Qt::ItemDataRole::DecorationRole});
      }
    }
  }

  void DragHover(const QModelIndex index) {
    if (m_draggedHoverIndex.row() == index.row() && m_draggedHoverIndex.column() == index.column()) {
      return;
    }
    emit dataChanged(m_draggedHoverIndex, m_draggedHoverIndex, {Qt::ItemDataRole::ForegroundRole});
    emit dataChanged(index, index, {Qt::ItemDataRole::ForegroundRole});
    m_draggedHoverIndex = index;
  }

  void DragRelease() {
    if (!m_draggedHoverIndex.isValid()) {
      return; // already invalid index
    }
    QModelIndex tmpIndex = m_draggedHoverIndex;
    m_draggedHoverIndex = QModelIndex{};
    emit dataChanged(tmpIndex, tmpIndex, {Qt::ItemDataRole::ForegroundRole});
  }

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
    if (orientation == Qt::Vertical) {
      if (role == Qt::TextAlignmentRole) {
        return Qt::AlignRight;
      }
    }
    return QFileSystemModel::headerData(section, orientation, role);
  }

public slots:
  void whenDirectoryLoaded(const QString& path);

protected:
  mutable CustomStatusBar* _mPLogger{nullptr};

private:
  SelectionsRangeHelper mCutIndexes, mCopyIndexes;
  QModelIndex m_draggedHoverIndex;
};

#endif // FILESYSTEMMODEL_H
