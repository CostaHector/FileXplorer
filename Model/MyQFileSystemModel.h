﻿#ifndef MYQFILESYSTEMMODEL_H
#define MYQFILESYSTEMMODEL_H

#include "Component/CustomStatusBar.h"

#include <QDebug>
#include <QFileSystemModel>
#include <QHash>
#include <QMap>
#include <QObject>
#include <QSettings>

class MyQFileSystemModel : public QFileSystemModel {
 public:
  explicit MyQFileSystemModel(QObject* parent = nullptr);

  void BindLogger(CustomStatusBar* logger);

  auto fullInfo(const QModelIndex& curIndex) const -> QString;

  Qt::ItemFlags flags(const QModelIndex& index) const override;
  bool canItemsBeDragged(const QModelIndex& index) const;

  bool canItemsDroppedHere(const QModelIndex& index) const;

  bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const override;
  bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
  Qt::DropActions supportedDropActions() const override;
  Qt::DropActions supportedDragActions() const override;

  void ClearCopyAndCutDict() {
    ClearCutDict();
    ClearCopiedDict();
  }

  void ClearCutDict() {
    decltype(m_cutMap) tmp;
    tmp.swap(m_cutMap);
    // continous cut made index not exist but valid can emit not exist make it crash down.
    // so we need remove index before emit dataChanged
    // bugs here. so we need check if DoNotUseParent is true
    for (auto it = tmp.cbegin(); it != tmp.cend(); ++it) {
      for (auto ind : it.value()) {
        if (checkIndex(ind, CheckIndexOption::DoNotUseParent))
          continue;
        emit dataChanged(ind, ind, {Qt::ItemDataRole::BackgroundRole});
      }
    }
  }

  void ClearCopiedDict() {
    decltype(m_copiedMap) tmp;
    tmp.swap(m_copiedMap);
    for (auto it = tmp.cbegin(); it != tmp.cend(); ++it) {
      for (auto ind : it.value()) {
         if (checkIndex(ind, CheckIndexOption::DoNotUseParent))
          continue;
        emit dataChanged(ind, ind, {Qt::ItemDataRole::BackgroundRole});
      }
    }
  }

  void CutSomething(const QModelIndexList& cutIndexes, bool appendMode = false) {
    if (!appendMode) {
      ClearCutDict();
    }
    ClearCopiedDict();
    if (!m_cutMap.contains(rootPath())) {
      m_cutMap[rootPath()] = {};
    }
    m_cutMap[rootPath()] += cutIndexes;
  }

  void CopiedSomething(const QModelIndexList& copiedIndexes, bool appendMode = false) {
    if (!appendMode) {
      ClearCopiedDict();
    }
    ClearCutDict();
    if (not m_copiedMap.contains(rootPath())) {
      m_copiedMap[rootPath()] = {};
    }
    m_copiedMap[rootPath()] += copiedIndexes;
  }

  void DragHover(const QModelIndex index) {
    if (m_draggedHoverIndex.row() == index.row() and m_draggedHoverIndex.column() == index.column()) {
      return;
    }
    emit dataChanged(m_draggedHoverIndex, m_draggedHoverIndex, {Qt::ItemDataRole::BackgroundRole});
    emit dataChanged(index, index, {Qt::ItemDataRole::BackgroundRole});
    m_draggedHoverIndex = index;
  }

  void DragRelease() {
    if (not m_draggedHoverIndex.isValid()) {
      return;  // already invalid index
    }
    QModelIndex tmpIndex = m_draggedHoverIndex;
    m_draggedHoverIndex = QModelIndex();
    emit dataChanged(tmpIndex, tmpIndex, {Qt::ItemDataRole::BackgroundRole});
  }

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
    if (role == Qt::BackgroundRole) {
      if (m_draggedHoverIndex == index) {
        return QBrush(Qt::green);
      }
      if (m_cutMap.contains(rootPath()) && m_cutMap[rootPath()].contains(index)) {
        return QBrush(Qt::GlobalColor::darkGray, Qt::BrushStyle::Dense4Pattern);
      }
      if (m_copiedMap.contains(rootPath()) && m_copiedMap[rootPath()].contains(index)) {
        return QBrush(Qt::GlobalColor::yellow, Qt::BrushStyle::CrossPattern);
      }
      return QBrush(Qt::transparent);
    }
    return QFileSystemModel::data(index, role);
  }

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
    if (orientation == Qt::Vertical) {
      if (role == Qt::TextAlignmentRole) {
        return Qt::AlignRight;
      }
    }
    return QFileSystemModel::headerData(section, orientation, role);
  }

 public slots:
  void whenRootPathChanged(const QString& newpath);
  void whenDirectoryLoaded(const QString& path);

 protected:
  CustomStatusBar* _logger;
  int m_imagesSizeLoaded = 0;

  static int previewsCnt;
  static constexpr int cacheWidth = 256;
  static constexpr int cacheHeight = 256;

  static constexpr int IMAGES_COUNT_LOAD_ONCE_MAX = 3;  // 10 pics
 private:
  QHash<QString, QModelIndexList> m_copiedMap;
  QHash<QString, QModelIndexList> m_cutMap;
  QModelIndex m_draggedHoverIndex;
};

#endif  // MYQFILESYSTEMMODEL_H
