#ifndef DUPLICATEVIDEOSMETAINFOMODEL_H
#define DUPLICATEVIDEOSMETAINFOMODEL_H

#include "QAbstractTableModelPub.h"
#include "DupVidsManager.h"
#include "DupVideosHelper.h"

class DuplicateVideosMetaInfoModel : public QAbstractTableModelPub {
 public:
  using QAbstractTableModelPub::QAbstractTableModelPub;
  auto headerData(int section, Qt::Orientation orientation, int role) const -> QVariant override;
  auto rowCount(const QModelIndex& /*parent*/ = {}) const -> int override { return m_data.size(); }
  auto columnCount(const QModelIndex& /*parent*/ = {}) const -> int override { return DUP_VIDEOS_METAINFO_HOR_HEADER.size(); }
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QString fileName(const QModelIndex& index) const;
  QStringList fileNames(const QModelIndexList& indx) const;
  void UpdateDupVideoTableMetaInfoList(DupVidTableName2RecordCountList& newData);

 private:
  DupVidTableName2RecordCountList m_data;
  static const QStringList DUP_VIDEOS_METAINFO_HOR_HEADER;
};

#endif  // DUPLICATEVIDEOSMETAINFOMODEL_H
