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
  QString plainCsvFileContent;
  if (!ParseEncryptCsvFile(encCsvFileFullPath, plainCsvFileContent, aesKey)) {
    // aes invalid or corrupted encrypted csv file
    LOG_W("Failed to parse encrypted CSV (wrong AES key or corrupted file): %s", qPrintable(encCsvFileFullPath));
    return {false, {}};
  }
  QList<int> incompleteLines;
  QList<AccountInfo> accounts = GetAccountsFromPlainString(plainCsvFileContent, incompleteLines);
  return {true, accounts};
}

bool ParseEncryptCsvFile(const QString& encryptCsvFilePath, QString& plainCsvFileContent, const QString& aesKey){
  bool bReadOk{false};
  QString encryptedContents = FileTool::StringTextReader(encryptCsvFilePath, &bReadOk);
  if (!bReadOk) {
    LOG_W("Open file[%s] to read failed", qPrintable(encryptCsvFilePath));
    return false;
  }
  bool decryptResult = SimpleAES(aesKey).decrypt_GCM(encryptedContents, plainCsvFileContent);
  if (!decryptResult) {
    LOG_C("Decrypt file[%s] failed! Skip load", qPrintable(encryptCsvFilePath));
    return false;
  }
  return true;
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
  const bool saveRes = FileTool::AtomicTextWriter(savedToFileFullPath, fullPlainCSVContents);
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

QString GetSaveToFullPath(bool bEncrypt, const QString& saveTo) {
  return PathTool::Path2Join(saveTo.isEmpty() ? SystemPath::RoamingPath() : saveTo,
                             bEncrypt ? ENCRYPTED_CSV_FILE_NAME : PLAIN_CSV_FILE_NAME);
}

const QIcon& IconForType(const QString& typeStr) {
  static const std::array<std::pair<QString, QIcon>, 8> kIcons
      {
       std::pair<QString, QIcon>{"unionpay",   QIcon(":/bankcard/UNIONPAY")},
       std::pair<QString, QIcon>{"mastercard", QIcon(":/bankcard/MASTERCARD")},
       std::pair<QString, QIcon>{"visa",       QIcon(":/bankcard/VISA")},
       std::pair<QString, QIcon>{"bank",       QIcon(":/type/BANK")},
       std::pair<QString, QIcon>{"society",    QIcon(":/type/SOCIETY")},
       std::pair<QString, QIcon>{"email",      QIcon(":/type/EMAIL")},
       std::pair<QString, QIcon>{"game",       QIcon(":/type/GAME")},
       std::pair<QString, QIcon>{"tool",       QIcon(":/type/TOOL")},
       };
  static const QIcon kEmpty;
  for (int i = 0; i < kIcons.size(); ++i) {
    if (typeStr == kIcons[i].first) {
      return kIcons[i].second;
    }
  }
  return kEmpty;
}

}
