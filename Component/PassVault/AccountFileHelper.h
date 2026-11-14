#ifndef ACCOUNTFILEHELPER_H
#define ACCOUNTFILEHELPER_H

#include "AccountInfo.h"
#include <QList>

class QIcon;
namespace AccountFileHelper {
struct LoadResult {
  bool operator==(const LoadResult& rhs) const {
    return valid == rhs.valid && accounts == rhs.accounts;
  }
  bool valid;
  QList<AccountInfo> accounts;
};
LoadResult ValidateAndLoad(const QString& encCsvFileFullPath, const QString& aesKey);

// plain Utf8 Utf8 -> list<Account>
QList<AccountInfo> GetAccountsFromPlainString(const QString& contents, QList<int>& incompleteLines);
// encrypted base64 (.csv) -> encrypted binary -> plain binary -> plain Utf8 (.csv)
std::pair<bool, QString> ParseEncryptCsvFile(const QString& encryptCsvFilePath, const QString& aesKey);

// list<Account> -> plain Utf8
QString GetExportCSVRecords(const QList<AccountInfo>& accounts);

bool ExportAccountsToPlainCsvFile(const QList<AccountInfo>& accounts, const QString& savedToFileFullPath);
bool CommitChangesToEncryptedCsvFile(QList<AccountInfo>& accounts, const QString& savedToFileFullPath, const QString& aesKey);

QString GetDefaultEncFileFullPath();
constexpr const char* ENCRYPTED_CSV_FILE_NAME{"accounts.enc.csv"};

const QIcon& IconForType(const QString& typeStr);
}

#endif // ACCOUNTFILEHELPER_H
