#ifndef MYQFILESYSTEMMODEL_H
#define MYQFILESYSTEMMODEL_H

#include <QFileSystemModel>
#include <QHash>
#include <QMap>
#include <QObject>
#include <QSettings>
#include "Component/CustomStatusBar.h"

class MyQFileSystemModel : public QFileSystemModel {
public:
    explicit MyQFileSystemModel(CustomStatusBar* _statusBar = nullptr, QObject* parent = nullptr);

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const override;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
    Qt::DropActions supportedDropActions() const override;
    Qt::DropActions supportedDragActions() const override;

    void CutSomething(const QModelIndexList& cutIndexes, bool appendMode = false) {
        if (not appendMode) {
            m_cutMap.clear();
        }
    }
    void CopiedSomething(const QModelIndexList& copiedIndexes, bool appendMode = false) {
        if (not appendMode) {
            m_copiedMap.clear();
        }
        if (not m_copiedMap.contains(rootPath())) {
            m_copiedMap[rootPath()] = {};
        }
        m_copiedMap[rootPath()] += copiedIndexes;
    }

    void DragHover(const QModelIndex index){
        if (m_draggedHoverIndex.row() == index.row() and m_draggedHoverIndex.column() == index.column()){
            return;
        }
        emit dataChanged(m_draggedHoverIndex, m_draggedHoverIndex, {Qt::ItemDataRole::BackgroundRole});
        emit dataChanged(index, index, {Qt::ItemDataRole::BackgroundRole});
        m_draggedHoverIndex = index;
    }

    void DragRelease(const QModelIndex index=QModelIndex()){
        emit dataChanged(m_draggedHoverIndex, m_draggedHoverIndex, {Qt::ItemDataRole::BackgroundRole});
        m_draggedHoverIndex = QModelIndex();
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override{
        if (role == Qt::BackgroundRole){
            if (m_draggedHoverIndex == index){
                return QBrush(Qt::green);
            }
            if (m_cutMap.contains(rootPath()) and m_cutMap[rootPath()].contains(index)){
                return QBrush(Qt::GlobalColor::yellow);
            }
            if (m_copiedMap.contains(rootPath()) and m_copiedMap[rootPath()].contains(index)){
                return QBrush(Qt::GlobalColor::magenta);
            }
            return QBrush(Qt::transparent);
        }
        return QFileSystemModel::data(index, role);
    }

public slots:
    void whenRootPathChanged(const QString& newpath);
    void whenDirectoryLoaded(const QString& path);

protected:
    CustomStatusBar* logger;
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
