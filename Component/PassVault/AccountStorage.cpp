#include "AccountStorage.h"
#include "Logger.h"
#include "PathTool.h"

AccountStorage::AccountStorage(const QString& encFileFullPath, const QString& aesKey)
  : AccountStorage{AccountFileHelper::ValidateAndLoad(encFileFullPath, aesKey), encFileFullPath, aesKey} {
}

AccountStorage::AccountStorage(AccountFileHelper::LoadResult&& result, const QString& encFileFullPath, const QString& aesKey)
  : mIsValid{result.valid}, mAccounts{std::move(result.accounts)}, mEncCsvFileFullPath{encFileFullPath}, mAesKey{aesKey} {
}

void AccountStorage::SetEncFileFullPath(const QString& encFileFullPath) {
  AccountStorage temp{encFileFullPath, mAesKey};
  swap(temp);
}

void AccountStorage::SetAesKey(const QString& aesKey) {
  AccountStorage temp{mEncCsvFileFullPath, aesKey};
  swap(temp);
}

bool AccountStorage::CommitChangesToEncryptedCsvFile() {
  if (!IsValid()) {
    return false; // 拒绝写入
  }
  return AccountFileHelper::CommitChangesToEncryptedCsvFile(mAccounts, mEncCsvFileFullPath, mAesKey);
}

bool AccountStorage::ExportToPlainCsvFile() const {
  if (!IsValid()) {
    return false; // 拒绝写入
  }
  QString pre = PathTool::absolutePath(mEncCsvFileFullPath);
  const QString pth{PathTool::Path2Join(pre, "accounts_export.csv")};
  return AccountFileHelper::ExportAccountsToPlainCsvFile(mAccounts, pth);
}

AccountInfo& AccountStorage::operator[](int i) {
  return mAccounts[i];
}

const AccountInfo& AccountStorage::operator[](int i) const {
  return mAccounts[i];
}

bool AccountStorage::clearRowRangeModified(int beginRow, int endRow) {
  if (!(0 <= beginRow && beginRow < endRow && endRow <= size())) {
    LOG_W("range[%d, %d) invalid", beginRow, endRow);
    return false;
  }
  std::for_each(mAccounts.begin() + beginRow, mAccounts.begin() + endRow, [](AccountInfo& ele) { ele.SetModified(false); });
  return true;
}

bool AccountStorage::markRowRangeDeleted(int beginRow, int endRow, bool bDeleted) {
  if (!(0 <= beginRow && beginRow < endRow && endRow <= size())) {
    LOG_W("range[%d, %d) invalid", beginRow, endRow);
    return false;
  }
  std::for_each(mAccounts.begin() + beginRow, mAccounts.begin() + endRow, [bDeleted](AccountInfo& ele) { ele.SetDeleted(bDeleted); });
  return true;
}

bool AccountStorage::InsertNRows(int indexBefore, int cnt) {
  if (cnt <= 0) {
    LOG_D("no row need to insert into");
    return false;
  }
  if (indexBefore < 0) {
    indexBefore = 0;
  } else if (indexBefore > size()) {
    indexBefore = size();
  }

  const int oldSize = mAccounts.size();
  mAccounts.reserve(oldSize + cnt);
  for (int i = 0; i < cnt; ++i) {
    mAccounts.push_back(AccountInfo{});
  }
  std::rotate(mAccounts.begin() + indexBefore, mAccounts.begin() + oldSize, mAccounts.end());
  return true;
}

bool AccountStorage::AppendNRows(int cnt) {
  return InsertNRows(size(), cnt);
}