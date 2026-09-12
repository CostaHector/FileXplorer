#ifndef FAVORITESTREEMODEL_H
#define FAVORITESTREEMODEL_H

#include "FavoriteItemData.h"
#include "QAbstractTreeModelPub.h"

extern template class QAbstractTreeModelPub<FavTreeNode>;

class FavoritesTreeModel : public QAbstractTreeModelPub<FavTreeNode> {
  Q_OBJECT
 public:
  explicit FavoritesTreeModel(const QString& belongToName, QObject* parent = nullptr, bool bInitialCollectionsWhenEmpty = true);
  ~FavoritesTreeModel();

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

  Qt::ItemFlags flags(const QModelIndex& index) const override;
  bool canDropOn(const QModelIndex& index) const;
  Qt::DropActions supportedDropActions() const override;
  Qt::DropActions supportedDragActions() const override;
  QStringList mimeTypes() const override;
  QMimeData* mimeData(const QModelIndexList& indexes) const override;
  Qt::DropAction GetDropAction(const QMimeData* data, const QModelIndex& hoverIndex) const;
  bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& dstParent) const override;
  bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& dstParent) override;

  FavTreeNode* addPath(const QString& name, const QString& path, const QModelIndex& parentIndex = {}, int insertAt = -1);
  FavTreeNode* addPath(const QString& name, const QString& path, FavTreeNode* parent = nullptr, int insertAt = -1);

  QString filePath(const QModelIndex& parentIndex) const;

  void addInitialFavoritesGroup();
  void setThisTimeNotSave(bool bWillNotSaveDataThisTime) { mNotSaveDatasThisTimeBeforeDestruct = bWillNotSaveDataThisTime; }
  void saveToSettings();

  static constexpr const char* MIME_TYPE = "application/x-favoritetreeitemdata";

 private:
  int handleExternalDrop(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& dstParent);
  int handleInternalDrop(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& dstParent);
  bool isFolderAccessable(const QModelIndex& index) const;
  // "NFLF" = "Navigation Favorites Links File"

  QString GetBelongToName() const { return m_belongToName; }
  QString GetDataKeyInQSetting() const { return GetBelongToName() + "/DATAS"; }
  const QString m_belongToName;
  bool mNotSaveDatasThisTimeBeforeDestruct{false};
};

#endif  // FAVORITESTREEMODEL_H
