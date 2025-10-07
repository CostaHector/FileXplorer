#include "AccountStorage.h"
#include "SimpleAES.h"
#include "PublicVariable.h"
#include "PublicTool.h"
#include "Logger.h"
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>
#include <QDir>

const QString& AccountStorage::GetFullEncCsvFilePath() {
  static constexpr char ENC_CSV_FILE[]{"accounts.csv"};
  static const QString absEncFilePath = QDir::toNativeSeparators(SystemPath::WORK_PATH() + "/" + ENC_CSV_FILE);
#ifdef RUNNING_UNIT_TESTS
  return AccountStorageMock::GetFullEncCsvFilePathMock();
#endif
  return absEncFilePath;
}

const QString& AccountStorage::GetFullPlainCsvFilePath() {
  static constexpr char EXPORTED_PLAIN_CSV_FILE[]{"exportedPlainAccounts.csv"};
  static const QString absPlainFilePath = QDir::toNativeSeparators(SystemPath::WORK_PATH() + "/" + EXPORTED_PLAIN_CSV_FILE);
#ifdef RUNNING_UNIT_TESTS
  return AccountStorageMock::GetFullPlainCsvFilePathMock();
#endif
  return absPlainFilePath;
}

const bool AccountStorage::IsAccountCSVFileInexistOrEmpty() {
  QFile csvFile{GetFullEncCsvFilePath()};
  return !csvFile.exists() || csvFile.size() == 0;
}

QString AccountStorage::GetExportCSVRecords() const {
  QString fullPlainCSVContents;
  fullPlainCSVContents.reserve(2048);
  for (const AccountInfo& acc : mAccounts) {
    fullPlainCSVContents += acc.toCsvLine();
    fullPlainCSVContents += '\n';
  }
  if (!fullPlainCSVContents.isEmpty()) {
    fullPlainCSVContents.chop(1); // remove extra `\n` at the back
  }
  return fullPlainCSVContents;
}

// one can save to file to plain text or encrypted by their willing
bool AccountStorage::SaveAccounts(bool bEncrypt) const {
  QString fullPlainCSVContents = GetExportCSVRecords();
  QString contentNeedDumped;
  QString writeIntoPath;

  if (bEncrypt) {
    writeIntoPath = GetFullEncCsvFilePath();
    bool encryptedResult = SimpleAES::encrypt_GCM(fullPlainCSVContents, contentNeedDumped);
    if (!encryptedResult) {
      qCritical("Encrypt file[%s] failed! Skip write into file!", qPrintable(writeIntoPath));
      return false;
    }
  } else {
    writeIntoPath = GetFullPlainCsvFilePath();
    contentNeedDumped.swap(fullPlainCSVContents);
  }

  bool saveRes = FileTool::TextWriter(writeIntoPath, contentNeedDumped, QIODevice::WriteOnly | QIODevice::Text);
  if (!saveRes) {
    qCritical("Open file[%s] to write failed", qPrintable(writeIntoPath));
    return false;
  }

  AfterSave();
  LOG_D("%d byte(s) of %d record(s) dumped into %s succced", fullPlainCSVContents.size(), size(), qPrintable(writeIntoPath));
  return true;
}

// when start on, data is from plain or encrypted is determined
bool AccountStorage::LoadAccounts() {
  if (IsAccountCSVFileInexistOrEmpty()) {
    return true; // first time used
  }
  const QString encCsvFilePath = GetFullEncCsvFilePath();
  bool bReadOk{false};
  QString encryptedContents = FileTool::TextReader(encCsvFilePath, &bReadOk);
  if (!bReadOk) {
    LOG_W("Open file to read failed", qPrintable(encCsvFilePath));
    return false;
  }
  QString plainContents;
  bool decryptResult = SimpleAES::decrypt_GCM(encryptedContents, plainContents);
  if (!decryptResult) {
    LOG_C("Decrypt file[%s] failed! Skip load", qPrintable(encCsvFilePath));
    return false;
  }
  int nonEmptyLine{0};
  decltype(mAccounts) tempAccounts = GetAccountsFromPlainString(plainContents, &nonEmptyLine);
  mAccounts.swap(tempAccounts);
  // SetListModified(); here mAccounts is same as file contents, we consider it has no modification
  LOG_D("%d account record(s) was loaded from %d non empty lines", mAccounts.size(), nonEmptyLine);
  return true;
}

QVector<AccountInfo> AccountStorage::GetAccountsFromPlainString(const QString& contents, int* pNonEmptyLine) {
  decltype(mAccounts) tempAccounts;
  AccountInfo acc;
  int nonEmptyLine{0};
  for (const QString& line : contents.split('\n', Qt::SkipEmptyParts)) {
    if (line.isEmpty()) {
      continue;
    }
    ++nonEmptyLine;
    if (!AccountInfo::FromCsvLine(line, acc)) {
      continue;
    }
    tempAccounts.push_back(acc);
  }
  if (pNonEmptyLine != nullptr) {
    *pNonEmptyLine = nonEmptyLine;
  }
  return tempAccounts;
}

int AccountStorage::RemoveIndexes(const std::set<int>& rows) {
  // rows is ascending
  int beforeRowCnt = size();
  for (auto rit = rows.crbegin(); rit != rows.crend(); ++rit) {
    if (*rit < 0 || *rit >= beforeRowCnt) {
      continue;
    }
    mAccounts.removeAt(*rit);
  }
  LOG_D("row size changed from %d->%d(given %lu row indexes)", beforeRowCnt, size(), rows.size());
  const int rmvedRowCnt{beforeRowCnt - size()};
  SetListModified(rmvedRowCnt);
  return rmvedRowCnt;
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
  mAccounts.insert(indexBefore, cnt, AccountInfo{});
  SetListModified(cnt);
  return true;
}

bool AccountStorage::AppendNRows(int cnt) {
  return InsertNRows(size(), cnt);
}

AccountInfo& AccountStorage::operator[](int i) {
  if (i < 0 || i >= size()) {
    static AccountInfo INVALID_ACC_FILE_ITEM;
    return INVALID_ACC_FILE_ITEM;
  }
  return mAccounts[i];
}

const AccountInfo& AccountStorage::operator[](int i) const {
  if (i < 0 || i >= size()) {
    static AccountInfo INVALID_ACC_FILE_ITEM;
    return INVALID_ACC_FILE_ITEM;
  }
  return mAccounts[i];
}
