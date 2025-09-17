#ifndef FILESYSTEMMODEL_H
#define FILESYSTEMMODEL_H

#include "CustomStatusBar.h"
#include "SelectionsRangeHelper.h"
#include "Logger.h"
#include <QFileSystemModel>
#include <QObject>

class FileSystemModel : public QFileSystemModel {
public:
  explicit FileSystemModel(QObject* parent = nullptr);
  QModelIndex setRootPath(const QString &path) {
    // return index(rootPath());
    return mRootIndex = QFileSystemModel::setRootPath(path);
  }

  void BindLogger(CustomStatusBar* logger) const;

  QString fullInfo(const QModelIndex& curIndex) const;

  bool canDrop(const QModelIndex& targetDropIndex) const;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
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
    EmitBeforeDecorationRoleRevert(beRngLst);
    EmitAfterDecorationRoleChange(mCutIndexes.GetTopBottomRange());
  }
  void CopiedSomething(const QModelIndexList& copiedIndexes) {
    auto beRngLst = mCopyIndexes.GetTopBottomRange();
    ClearCopyAndCutDict();
    mCopyIndexes.Set(rootPath(), copiedIndexes);
    EmitBeforeDecorationRoleRevert(beRngLst);
    EmitAfterDecorationRoleChange(mCopyIndexes.GetTopBottomRange());
  }

  void DragHover(const QModelIndex index) {
    if (m_draggedHoverIndex.row() == index.row() && m_draggedHoverIndex.column() == index.column()) {
      return;
    }
    const int beforeRow = m_draggedHoverIndex.row();
    m_draggedHoverIndex = index;
    emit headerDataChanged(Qt::Orientation::Vertical, beforeRow, beforeRow);
    emit headerDataChanged(Qt::Orientation::Vertical, m_draggedHoverIndex.row(), m_draggedHoverIndex.row());
  }

  void DragRelease() {
    if (!m_draggedHoverIndex.isValid()) {
      return; // already invalid index
    }
    const int beforeRow = m_draggedHoverIndex.row();
    m_draggedHoverIndex = QModelIndex{};
    emit headerDataChanged(Qt::Orientation::Vertical, beforeRow, beforeRow);
  }

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

public slots:
  void whenDirectoryLoaded(const QString& path);

protected:
  mutable CustomStatusBar* _mPLogger{nullptr};

private:
  void EmitBeforeDecorationRoleRevert(const SelectionsRangeHelper::ROW_RANGES_LST& beRngLst) {
    foreach (auto beRng, beRngLst) {
      if (checkIndex(beRng.first, CheckIndexOption::DoNotUseParent) || checkIndex(beRng.second, CheckIndexOption::DoNotUseParent)) {
        LOG_D("parent model of indexes[(%d,%d), (%d,%d)] is invalid",//
               beRng.first.row(), beRng.first.column(),//
               beRng.second.row(), beRng.second.column());//
        break;
      }
      emit dataChanged(beRng.first, beRng.second, {Qt::DecorationRole});
    }
  }
  void EmitAfterDecorationRoleChange(const SelectionsRangeHelper::ROW_RANGES_LST& beRngLst) {
    foreach (auto beRng, beRngLst) {
      emit dataChanged(beRng.first, beRng.second, {Qt::DecorationRole});
    }
  }
  SelectionsRangeHelper mCutIndexes, mCopyIndexes;
  QModelIndex m_draggedHoverIndex;
  QModelIndex mRootIndex;
};

#endif // FILESYSTEMMODEL_H
