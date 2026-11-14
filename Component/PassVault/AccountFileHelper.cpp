#include "AccountFileHelper.h"
#include "FileTool.h"
#include "Logger.h"
#include "SimpleAES.h"
#include "PathTool.h"
#include "SystemPath.h"
#include <QFileInfo>

namespace AccountFileHelper {
LoadResult ValidateAndLoad(const QString& encCsvFileFullPath, const QString& aesKey) {
  if (encCsvFileFullPath.isEmpty() || aesKey.isEmpty()) {
    return {false, {}};
  }
  const QFileInfo encFile{encCsvFileFullPath};
  if (encFile.isDir()) { // exist and directory
    LOG_W("encCsvFileFullPath points to a directory: %s", qPrintable(encCsvFileFullPath));
    return {false, {}};
  }
  // not exist or not directory
  const bool bExist{encFile.exists()};
  if (!bExist) {
    const QFileInfo parentPathFi{encFile.absolutePath()};
    const bool isParentPathExist{parentPathFi.isDir()};
    if (!isParentPathExist) {
      LOG_W("Parent directory does not exist: encCsvFileFullPath=%s, parentDir=%s",
            qPrintable(encCsvFileFullPath),
            qPrintable(parentPathFi.absoluteFilePath()));
    }
    return {isParentPathExist, {}};
  }
  // must be file
  if (encFile.size() == 0) { // empty file
    return {true, {}};
  }
  // Parse result
  bool parseResult{false};
  QString plainCsvFileContent;
  std::tie(parseResult, plainCsvFileContent) = ParseEncryptCsvFile(encCsvFileFullPath, aesKey);
  if (!parseResult) {
    // aes invalid or corrupted encrypted csv file
    LOG_W("Failed to parse encrypted CSV (wrong AES key or corrupted file): %s", qPrintable(encCsvFileFullPath));
    return {false, {}};
  }
  QList<int> incompleteLines;
  QList<AccountInfo> accounts = GetAccountsFromPlainString(plainCsvFileContent, incompleteLines);
  return {true, accounts};
}

std::pair<bool, QString> ParseEncryptCsvFile(const QString& encryptCsvFilePath, const QString& aesKey){
  bool bReadOk{false};
  const QString encryptedContents = FileTool::StringTextReader(encryptCsvFilePath, &bReadOk);
  if (!bReadOk) {
    LOG_W("Open file[%s] to read failed", qPrintable(encryptCsvFilePath));
    return {false, ""};
  }
  QString plainCsvFileContent;
  const bool decryptResult = SimpleAES(aesKey).decrypt_GCM(encryptedContents, plainCsvFileContent);
  if (!decryptResult) {
    LOG_C("Decrypt file[%s] failed! Skip load", qPrintable(encryptCsvFilePath));
    return {false, ""};
  }
  return {true, plainCsvFileContent};
}

QList<AccountInfo> GetAccountsFromPlainString(const QString& contents, QList<int>& incompleteLines) {
  QList<AccountInfo> tempAccounts;
  AccountInfo acc;
  const QStringList lines = contents.split('\n', Qt::KeepEmptyParts);
  for (int i = 0; i < lines.size(); ++i) {
    const QString& line = lines[i];
    if (i == lines.size() - 1 && line.isEmpty()) {
      continue; // 允许最后一行是空行
    }
    if (!AccountInfo::FromCsvLine(line, acc)) {
      incompleteLines.push_back(i);
      continue;
    }
    tempAccounts.push_back(acc);
  }
  return tempAccounts;
}

QString GetExportCSVRecords(const QList<AccountInfo>& accounts) {
  QString plainCSVContents;
  plainCSVContents.reserve(2048);
  for (const AccountInfo& acc : accounts) {
    if (acc.IsDeleted()) {
      continue; // 已删除的元素写入文件时忽略
    }
    plainCSVContents += acc.toCsvLine();
    plainCSVContents += '\n';
  }
  return plainCSVContents;
}

// bEncrypt=false: list<Account> -> plain Utf8                                                     -----> local file
// bEncrypt=true:  list<Account> -> plain Utf8 -> plain binary -> encrypted binary -> plain base64 -----> local file
bool ExportAccountsToPlainCsvFile(const QList<AccountInfo>& accounts, const QString& savedToFileFullPath) {
  QString fullPlainCSVContents = GetExportCSVRecords(accounts);
  const bool saveRes = FileTool::StringTextWriter(savedToFileFullPath, fullPlainCSVContents, QIODevice::WriteOnly);
  if (!saveRes) {
    LOG_C("Open file[%s] to write failed", qPrintable(savedToFileFullPath));
    return false;
  }
  LOG_D("%d byte(s) of %d record(s) dumped into %s succced", fullPlainCSVContents.size(), accounts.size(), qPrintable(savedToFileFullPath));
  return true;
}

bool CommitChangesToEncryptedCsvFile(QList<AccountInfo>& accounts, const QString& savedToFileFullPath, const QString& aesKey) {
  QString fullPlainCSVContents = GetExportCSVRecords(accounts);
  QString contentNeedDumped;
  const bool encryptedResult = SimpleAES(aesKey).encrypt_GCM(fullPlainCSVContents, contentNeedDumped);
  if (!encryptedResult) {
    LOG_C("Encrypt file[%s] failed! Skip write into file!", qPrintable(savedToFileFullPath));
    return false;
  }
  const bool backupRes = FileTool::CreateABackupFile(savedToFileFullPath);
  if (!backupRes) { // 备份原文件
    LOG_W("backup file[%s] failed", qPrintable(savedToFileFullPath));
  }

  const bool saveRes = FileTool::AtomicTextWriter(savedToFileFullPath, contentNeedDumped);
  if (!saveRes) {
    LOG_C("Open file[%s] to write failed", qPrintable(savedToFileFullPath));
    return false;
  }
  std::for_each(accounts.begin(), accounts.end(), [](AccountInfo& ele) { ele.SetModified(false); });
  LOG_D("%d byte(s) of %d record(s) dumped into %s succced", fullPlainCSVContents.size(), accounts.size(), qPrintable(savedToFileFullPath));
  return true;
}

QString GetDefaultEncFileFullPath() {
  return PathTool::Path2Join(SystemPath::RoamingPath(), ENCRYPTED_CSV_FILE_NAME);
}

const QIcon& IconForType(const QString& typeStr) {
  static const std::array<std::pair<QString, QIcon>, 8> kIcons
      {
       std::pair<QString, QIcon>{"unionpay",   QIcon(":/pwd/types/UNIONPAY")},
       std::pair<QString, QIcon>{"mastercard", QIcon(":/pwd/types/MASTERCARD")},
       std::pair<QString, QIcon>{"visa",       QIcon(":/pwd/types/VISA")},
       std::pair<QString, QIcon>{"bank",       QIcon(":/pwd/types/BANK")},
       std::pair<QString, QIcon>{"social",     QIcon(":/pwd/types/SOCIAL_MEDIA")},
       std::pair<QString, QIcon>{"email",      QIcon(":/pwd/types/EMAIL")},
       std::pair<QString, QIcon>{"game",       QIcon(":/pwd/types/GAME")},
       std::pair<QString, QIcon>{"tool",       QIcon(":/pwd/types/TOOL")},
       };
  for (int i = 0; i < kIcons.size(); ++i) {
    if (typeStr == kIcons[i].first) {
      return kIcons[i].second;
    }
  }
  static const QIcon kEmpty;
  return kEmpty;
}

}
