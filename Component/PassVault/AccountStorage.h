#ifndef ACCOUNTSTORAGE_H
#define ACCOUNTSTORAGE_H

#include <QStringList>
#include "AccountFileHelper.h"

struct AccountStorage {
public:
  AccountStorage() = default;
  explicit AccountStorage(const QString& encCsvFileFullPath, const QString& aesKey);
  void swap(AccountStorage& rhs) {
    std::swap(mIsValid, rhs.mIsValid);
    mAccounts.swap(rhs.mAccounts);
    mEncCsvFileFullPath.swap(rhs.mEncCsvFileFullPath);
    mAesKey.swap(rhs.mAesKey);
  }

  void SetEncFileFullPath(const QString& encFileFullPath);
  void SetAesKey(const QString& aesKey);

  QString toPlainCsv() const { return AccountFileHelper::GetExportCSVRecords(mAccounts); }
  // one can save to file to plain text or encrypted and then save
  bool CommitChangesToEncryptedCsvFile();
  bool ExportToPlainCsvFile() const;

  bool clearRowRangeModified(int beginRow, int endRow);
  bool markRowRangeDeleted(int beginRow, int endRow, bool bDeleted);
  bool InsertNRows(int indexBefore, int cnt);
  bool AppendNRows(int cnt);

  AccountInfo& operator[](int i);
  const AccountInfo& operator[](int i) const;
  AccountStorage& operator+=(const QList<AccountInfo>& tempAccounts) {
    mAccounts += tempAccounts;
    return *this;
  }

  inline int size() const { return mAccounts.size(); }
  inline bool empty() const { return mAccounts.empty(); }
  inline void append(const AccountInfo& ele) { mAccounts.push_back(ele); }
  bool IsValid() const {
    return mIsValid;
  }

private:
  AccountStorage(AccountFileHelper::LoadResult&& result, const QString& encFileFullPath, const QString& aesKey);

  bool mIsValid{false}; // 要求"mEncCsvFileFullPath和AES二者均非空 && (文件存在且非空且解析成功 || 文件不存在 || 文件为空)"
  QList<AccountInfo> mAccounts;
  QString mEncCsvFileFullPath;
  QString mAesKey;
};
#endif  // ACCOUNTSTORAGE_H
