#ifndef FAVORITESTREEMODEL_H
#define FAVORITESTREEMODEL_H

#include <QStandardItemModel>
#include "FavoriteItemData.h"

class FavoritesTreeModel : public QStandardItemModel {
  Q_OBJECT
 public:
  enum Role {
    IS_GROUP_ROLE = Qt::UserRole + 1,
    FULL_PATH_ROLE,
    LAST_ACCESS_ROLE,
    ACCESS_COUNT_ROLE,
  };
  explicit FavoritesTreeModel(const QString& belongToName, QObject* parent = nullptr, bool bInitialCollectionsWhenEmpty = true);
  ~FavoritesTreeModel();

  bool setDatas(const QVector<FavoriteItemData>& topLevelItems);
  bool setDatas(const QByteArray& dataByteArray);

  Qt::ItemFlags flags(const QModelIndex& index) const override;
  bool canDropOn(const QModelIndex& index) const;
  Qt::DropActions supportedDropActions() const override;
  Qt::DropActions supportedDragActions() const override;
  QStringList mimeTypes() const override;
  QMimeData* mimeData(const QModelIndexList& indexes) const override;
  bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& dstParent) const override;
  bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& dstParent) override;

  static bool fromByteArray(const QByteArray& data, QVector<FavoriteItemData>& topLevelItems);
  QByteArray toByteArray() const;

  static FavoriteItemData convertItemToData(QStandardItem* item);
  static QStandardItem* convertDataToItem(const FavoriteItemData& data);

  QStandardItem* addGroup(const QString& grpName, const QModelIndex& parentIndex = {});
  QStandardItem* addGroup(const QString& grpName, QStandardItem* parentItem = nullptr);
  QStandardItem* addPath(const QString& name, const QString& path, const QModelIndex& parentIndex = {});
  QStandardItem* addPath(const QString& name, const QString& path, QStandardItem* parent = nullptr);

  QString filePath(const QModelIndex& parentIndex) const;

  bool isRoot(const QModelIndex& parentIndex) const { return !parentIndex.isValid(); }
  bool isGroup(const QModelIndex& parentIndex) const;
  QString groupName(const QModelIndex& parentIndex) const;

  int moveParentIndexesTo(const QModelIndexList& parentIndexes, const QModelIndex& dest);
  int removeParentIndexes(const QModelIndexList& parentIndexes);
  bool onRename(const QModelIndex& parentIndex, const QString& newName);
  void setThisTimeNotSave(bool bWillNotSaveDataThisTime) { mNotSaveDatasThisTimeBeforeDestruct = bWillNotSaveDataThisTime; }
  void saveToSettings();

  static constexpr const char* MIME_TYPE = "application/x-favoritetreeitemdata";

 private:
  QList<QStandardItem*> GetItemsNeedProcess(const QModelIndexList& parentIndexes, QStandardItem* destItem) const;
  static bool isIndexValidAndDescendantOfValidAncestor(const QModelIndex& descendant, const QModelIndex& ancestor);
  int handleExternalDrop(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& dstParent);
  int handleInternalDrop(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& dstParent);

  QString GetBelongToName() const { return m_belongToName; }
  QString GetDataKeyInQSetting() const { return GetBelongToName() + "_DATAS"; }
  const QString m_belongToName;
  bool mNotSaveDatasThisTimeBeforeDestruct{false};
  static constexpr quint16 VERSION = 1;
};

#endif  // FAVORITESTREEMODEL_H
