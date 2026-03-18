#ifndef IMGREORDERLISTMODEL_H
#define IMGREORDERLISTMODEL_H

#include "QAbstractListModelPub.h"
#include "ImgReorderDataType.h"
#include <QSet>

extern template std::pair<bool, ImgReorderDataLst> MoveItemsBase<ImgReorderDataLst>(const ImgReorderDataLst&, const QList<int>&, int);

class ImgReorderListModel : public QAbstractListModelPub {
 public:
  using QAbstractListModelPub::QAbstractListModelPub;
  bool setImagesToReorder(const QStringList& imgs, const QString& baseName, int startIndex = 0, const QString& namePattern = " %1");

  int rowCount(const QModelIndex& parent = QModelIndex()) const override { return m_imgs.size(); }
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
  Qt::ItemFlags flags(const QModelIndex& index) const override {
    return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsDragEnabled | Qt::ItemFlag::ItemIsDropEnabled | Qt::ItemFlag::ItemIsEditable;
  }

  QStringList mimeTypes() const override { return {MIME_TYPE}; }
  QMimeData* mimeData(const QModelIndexList& indexes) const override;
  bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
  Qt::DropActions supportedDropActions() const override;

  QStringList getOrderedNames() const;
  static constexpr const char* MIME_TYPE = "application/x-imgreorder-rows";

  bool onBatchShiftSelectedRowsByStep(const QModelIndexList& indexes, int step=100);
  bool onNormalizeKeepRelativeOrder();

 private:
  void initOccupiedRows(int n) const;
  void updateOccupiedRows() const;

  ImgReorderDataLst m_imgs;
  mutable QSet<int> m_occupiedRows;
  QString m_baseName;
  int m_startNo{0};
  QString m_namePattern;
};

std::pair<bool, ImgReorderDataLst> BatchShiftSelectedRowsByStep(const ImgReorderDataLst& datas, const QList<int>& selectedRows, int step);
std::pair<bool, ImgReorderDataLst> NormalizeKeepRelativeOrder(const ImgReorderDataLst& datas);

#endif  // IMGREORDERLISTMODEL_H
