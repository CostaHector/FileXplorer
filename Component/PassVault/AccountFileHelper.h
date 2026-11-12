#ifndef ACCOUNTFILEHELPER_H
#define ACCOUNTFILEHELPER_H

#include "AccountInfo.h"
#include <QList>

class QIcon;
namespace AccountFileHelper {
struct LoadResult {
  bool valid;
  QList<AccountInfo> accounts;
};
LoadResult ValidateAndLoad(const QString& encCsvFileFullPath, const QString& aesKey);

// plain Utf8 Utf8 -> list<Account>
QList<AccountInfo> GetAccountsFromPlainString(const QString& contents, QList<int>& incompleteLines);
// encrypted base64 (.csv) -> encrypted binary -> plain binary -> plain Utf8 (.csv)
bool ParseEncryptCsvFile(const QString& encryptCsvFilePath, QString& plainCsvFileContent, const QString& aesKey);

// list<Account> -> plain Utf8
QString GetExportCSVRecords(const QList<AccountInfo>& accounts);

bool ExportAccountsToPlainCsvFile(const QList<AccountInfo>& accounts, const QString& savedToFileFullPath);
bool CommitChangesToEncryptedCsvFile(QList<AccountInfo>& accounts, const QString& savedToFileFullPath, const QString& aesKey);

QString GetSaveToFullPath(bool bEncrypt, const QString& saveTo);
constexpr const char* ENCRYPTED_CSV_FILE_NAME{"accounts.enc.csv"};
constexpr const char* PLAIN_CSV_FILE_NAME{"accounts_export.csv"};

const QIcon& IconForType(const QString& typeStr);
}

#endif // ACCOUNTFILEHELPER_H
