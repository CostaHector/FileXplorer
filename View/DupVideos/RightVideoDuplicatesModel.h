#ifndef RIGHTVIDEODUPLICATESMODEL_H
#define RIGHTVIDEODUPLICATESMODEL_H

#include "QAbstractTableModelPub.h"
#include "DuplicateVideosHelper.h"

class RightVideoDuplicatesModel : public QAbstractTableModelPub {
 public:
  using QAbstractTableModelPub::QAbstractTableModelPub;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  int rowCount(const QModelIndex& parent = {}) const override;
  int columnCount(const QModelIndex& /*parent*/ = {}) const override { return DuplicateVideoMetaInfo::DV_TABLE_HEADERS_COUNT; }

  auto headerData(int section, Qt::Orientation orientation, int role) const -> QVariant override;

  QString fileNameUsedForToolEverything(const QModelIndex& index) const;
  QString filePath(const QModelIndex& index) const;
  QString fileName(const QModelIndex& index) const;

  bool SetSharedMemberSrc(const GroupedDupVidListArr* _groupedVidsListArr, const DuplicateVideoDetectionCriteria::DVCriteriaE* _currentDiffer);

  int onChangeDetailIndex(int newLeftSelectedRow);
  void onInvalidateLeftSelection();

 private:
  int rowCountHelper(int leftSelectedRow) const;
  int getLeftSelectedRow() const { return m_leftSelectedRow; }
  bool isLeftSelectedRowValid() const {
    return _pGroupedVidsList != nullptr && _pCurrentDiffer != nullptr && 0 <= m_leftSelectedRow &&
           m_leftSelectedRow < (*_pGroupedVidsList)[(int)*_pCurrentDiffer].size();
  }
  void invalidDataLeftSelectedRows() { m_leftSelectedRow = INVALID_LEFT_SELECTED_ROW; }
  int m_leftSelectedRow = INVALID_LEFT_SELECTED_ROW;  // -1: nothing selected in left

  const DuplicateVideoDetectionCriteria::DVCriteriaE* _pCurrentDiffer{nullptr};
  const GroupedDupVidListArr* _pGroupedVidsList{nullptr};
};

#endif  // RIGHTVIDEODUPLICATESMODEL_H
