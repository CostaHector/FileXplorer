#ifndef ACCOUNTTABLEMODEL_H
#define ACCOUNTTABLEMODEL_H

#include "QAbstractTableModelPub.h"
#include "AccountStorage.h"

class AccountTableModel : public QAbstractTableModelPub {
public:
  explicit AccountTableModel(const QString& encFileFullPath, const QString& aesKey, QObject* parent = nullptr);

  int rowCount(const QModelIndex& parent = {}) const override {  //
    return mAccountsList.size();                                 //
  }
  int columnCount(const QModelIndex& parent = {}) const override {  //
    return AccountInfo::COLUMN_COUNT;              //
  }

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  Qt::ItemFlags flags(const QModelIndex& index) const override;

  int MarkDeletedIndexes(const QModelIndexList& srcIndexes, bool bDeleted);
  int InsertNRows(int indexBefore, int cnt);
  int AppendAccountRecords(const QList<AccountInfo>& tempAccounts);

  bool onSave();
  QString GetExportCSVRecords() const;
  bool ExportToPlainCSVFile() const;

  AccountInfo* rowDataAt(int index);
  const AccountInfo* rowDataAt(int index) const;

  void SetEncFileFullPath(const QString& encFileFullPath);
  void SetAesKey(const QString& aesKey);

private:
  AccountStorage mAccountsList;
};

#endif  // ACCOUNTTABLEMODEL_H
