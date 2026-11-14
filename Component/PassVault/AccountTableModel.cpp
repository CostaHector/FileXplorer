#include "AccountTableModel.h"
#include "GeneralDataType.h"
#include "Logger.h"
#include <QIcon>

AccountTableModel::AccountTableModel(const QString& encFileFullPath, const QString& aesKey, QObject* parent)
  : QAbstractTableModelPub{parent}, mAccountsList{encFileFullPath, aesKey} {
}

QVariant AccountTableModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }
  if (!(0 <= index.row() && index.row() < rowCount())) {
    return QVariant();
  }
  const AccountInfo& item = mAccountsList[index.row()];
  const int section = index.column();
  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    switch (section) {
      case AccountInfo::Role2Column(AccountInfo::Role::TYPE_STR_ROLE):
        return item.typeStr;
      case AccountInfo::Role2Column(AccountInfo::Role::NAME_STR_ROLE):
        return item.nameStr;
      case AccountInfo::Role2Column(AccountInfo::Role::ACCOUNT_STR_ROLE):
        return item.accountStr;
      case AccountInfo::Role2Column(AccountInfo::Role::PWD_STR_ROLE):
        return item.pwdStr;
      case AccountInfo::Role2Column(AccountInfo::Role::OTHERS_STR_ROLE):
        return item.othersStr;
      case AccountInfo::Role2Column(AccountInfo::Role::IS_MODIFIED_ROLE):
        return item.IsModified();
      case AccountInfo::Role2Column(AccountInfo::Role::IS_DELETED_ROLE):
        return item.IsDeleted();
      default:
        return QVariant();
    }
  } else if (role == Qt::DecorationRole) {
    if (section == 0) {
      return AccountFileHelper::IconForType(item.typeStr.toLower());
    }
    return {};
  } else {
    switch (role) {
      case AccountInfo::Role::TYPE_STR_ROLE:
        return item.typeStr;
      case AccountInfo::Role::NAME_STR_ROLE:
        return item.nameStr;
      case AccountInfo::Role::ACCOUNT_STR_ROLE:
        return item.accountStr;
      case AccountInfo::Role::PWD_STR_ROLE:
        return item.pwdStr;
      case AccountInfo::Role::OTHERS_STR_ROLE:
        return item.othersStr;
      case AccountInfo::Role::IS_MODIFIED_ROLE:
        return item.IsModified();
      case AccountInfo::Role::IS_DELETED_ROLE:
        return item.IsDeleted();
      case AccountInfo::DATA_TYPE_ROLE:
        return section == AccountInfo::Role::OTHERS_STR_ROLE - AccountInfo::Role::DEF_BEGIN_ROLE ? GeneralDataType::Type::MULTI_LINE_STR : GeneralDataType::Type::PLAIN_STR;
      default:
        return QVariant();
    }
  }
  return {};
}

bool AccountTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (!index.isValid() || index.row() < 0 || index.row() >= mAccountsList.size()) {
    return false;
  }
  if (value == data(index, role)) { // 新值==旧值
    return true;
  }

  AccountInfo& item = mAccountsList[index.row()];
  switch (role) {
    case Qt::EditRole: {
      // 原来的按 column 逻辑保持不变（QTableView 场景仍可用）
      switch (index.column()) {
        case AccountInfo::Role2Column(AccountInfo::Role::TYPE_STR_ROLE):
          item.typeStr = value.toString();
          break;
        case AccountInfo::Role2Column(AccountInfo::Role::NAME_STR_ROLE):
          item.nameStr = value.toString();
          break;
        case AccountInfo::Role2Column(AccountInfo::Role::ACCOUNT_STR_ROLE):
          item.accountStr = value.toString();
          break;
        case AccountInfo::Role2Column(AccountInfo::Role::PWD_STR_ROLE):
          item.pwdStr = value.toString();
          break;
        case AccountInfo::Role2Column(AccountInfo::Role::OTHERS_STR_ROLE):
          item.othersStr = value.toString();
          break;
        default:
          return false;
      }
      break;
    }

    // QListView 单列场景：按自定义 role 直接写字段
    case AccountInfo::Role::TYPE_STR_ROLE:
      item.typeStr = value.toString();
      break;
    case AccountInfo::Role::NAME_STR_ROLE:
      item.nameStr = value.toString();
      break;
    case AccountInfo::Role::ACCOUNT_STR_ROLE:
      item.accountStr = value.toString();
      break;
    case AccountInfo::Role::PWD_STR_ROLE:
      item.pwdStr = value.toString();
      break;
    case AccountInfo::Role::OTHERS_STR_ROLE:
      item.othersStr = value.toString();
      break;
    default:
      return false;
  }

  item.SetModified(true);
  emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
  return true;
}

QVariant AccountTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role == Qt::TextAlignmentRole) {
    if (orientation == Qt::Vertical) {
      return Qt::AlignRight;
    }
  }
  if (role == Qt::DisplayRole) {
    if (0 <= section && section < columnCount() && orientation == Qt::Orientation::Horizontal) {
      return AccountInfo::HOR_HEADER_TITLES[section];
    }
    return section + 1;
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags AccountTableModel::flags(const QModelIndex& index) const {
  if (!AccountInfo::IsColumnEditableByUser(index.column())) {
    return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
  }
  if (const AccountInfo* p = rowDataAt(index.row())) {
    if (p->IsDeleted()) {
      return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
    }
  }
  return Qt::ItemFlag::ItemIsEditable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
}


int AccountTableModel::MarkDeletedIndexes(const QModelIndexList& srcIndexes, bool bDeleted) {
  if (srcIndexes.empty()) {
    LOG_D("no row need to be delete");
    return 0;
  }
  const QList<int> rowsList = ModelTools::GetSortedUniqueRowsFromIndexes(srcIndexes);
  const auto fbPairList = ModelTools::MergeList2SectionsRange(rowsList);
  for (const auto& fbPair: fbPairList) {
    const int frontRow = fbPair.first;
    const int backRow = fbPair.second;
    if (mAccountsList.markRowRangeDeleted(frontRow, backRow + 1, bDeleted)) {
      emit dataChanged(index(frontRow, 0), index(backRow, columnCount() - 1), {Qt::ItemDataRole::ForegroundRole});
    }
  }
  return rowsList.size();
}

int AccountTableModel::InsertNRows(int indexBefore, int cnt) {
  if (cnt == 0) {
    LOG_D("No row to insert");
    return 0;
  }
  if (indexBefore < 0) {
    indexBefore = 0;
  }
  if (indexBefore >= rowCount()) {
    indexBefore = rowCount();
  }
  const int before = rowCount();
  const int after = before + cnt;
  RowsCountBeginChange(before, after);
  if (!mAccountsList.InsertNRows(indexBefore, cnt)) {
    LOG_D("insert before index[%d] %d rows failed", indexBefore, cnt);
    return -1;
  }
  RowsCountEndChange();
  return cnt;
}

int AccountTableModel::AppendAccountRecords(const QList<AccountInfo>& tempAccounts) {
  int cnt = tempAccounts.size();
  if (cnt == 0) {
    LOG_D("No records to insert");
    return 0;
  }

  int before = rowCount();
  int after = before + tempAccounts.size();

  RowsCountBeginChange(before, after);
  mAccountsList += tempAccounts;
  RowsCountEndChange();
  return tempAccounts.size();
}

QString AccountTableModel::GetExportCSVRecords() const {
  return mAccountsList.toPlainCsv();
}

bool AccountTableModel::ExportToPlainCSVFile() const {
  return mAccountsList.ExportToPlainCsvFile();
}

bool AccountTableModel::onSave() {
  if (rowCount() <= 0 || columnCount() <= 0) {
    return false;
  }
  const bool isAllSucceed{mAccountsList.CommitChangesToEncryptedCsvFile()};
  if (isAllSucceed) {
    emit dataChanged(index(0, columnCount() - 1), index(rowCount() - 1, columnCount() - 1), {AccountInfo::Role::IS_MODIFIED_ROLE});
  }
  return isAllSucceed;
}

AccountInfo* AccountTableModel::rowDataAt(int index) {
  return const_cast<AccountInfo*>(
      static_cast<const AccountTableModel*>(this)->rowDataAt(index)
      );
}

const AccountInfo* AccountTableModel::rowDataAt(int index) const {
  if (index < 0 || index >= rowCount()) {
    return nullptr;
  }
  return &mAccountsList[index];
}

void AccountTableModel::SetEncFileFullPath(const QString& encFileFullPath) {
  ResetWithDecryptedNotify([this, encFileFullPath](){
    mAccountsList.SetEncFileFullPath(encFileFullPath);
  });
}

void AccountTableModel::SetAesKey(const QString& aesKey) {
  ResetWithDecryptedNotify([this, aesKey](){
    mAccountsList.SetAesKey(aesKey);
  });
}

void AccountTableModel::SetEncFileFullPathAndAesKey(const QString& encFileFullPath, const QString& aesKey) {
  ResetWithDecryptedNotify([this, encFileFullPath, aesKey](){
    AccountStorage temp{encFileFullPath, aesKey};
    mAccountsList.swap(temp);
  });
}
