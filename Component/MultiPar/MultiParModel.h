#ifndef MULTIPARMODEL_H
#define MULTIPARMODEL_H

#include "QAbstractTableModelPub.h"
#include "ParVerifyInfomation.h"

class MultiParModel : public QAbstractTableModelPub {
public:
  explicit MultiParModel(ParVerifyInfomationList&& resultList, QObject* parent = nullptr);

  int rowCount(const QModelIndex& /*parent*/ = {}) const override { return mVerifyInfoList.size(); }
  int columnCount(const QModelIndex& /*parent*/ = {}) const override { return MultiParKey::COLUMNS_CNT_BUTT; }
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  const QByteArray* GetCliOutput(const QModelIndex& srcIndex) const;
  QString GetFrontSourceFile(const QModelIndex& srcIndex) const;
  QString GetPar2FileAbsPath(const QModelIndex& srcIndex) const;

  int repairBrokenFile(const QModelIndexList& srcIndexes, const ParVerifyInfomation::Par2StatusE par2Status = ParVerifyInfomation::Par2StatusE::READY_TO_REPAIR);
  int syncBuiltInSrcFileListInPar2(const QModelIndexList& srcIndexes);
  QModelIndexList ProcessOldNewPar2Names(const QModelIndexList& srcIndexes) const;
  int ReverifyPar2File(const QModelIndexList& needReverifyIndexes);

private:
  int GetLastColumnIndex() const { return columnCount() - 1; }
  void EmitInfoChanged(const QModelIndex& ind);
  ParVerifyInfomationList mVerifyInfoList;
};

#endif // MULTIPARMODEL_H
