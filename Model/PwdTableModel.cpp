#include "PwdTableModel.h"
#include <QIcon>

bool AccountSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  const QString keyword = filterRegExp().pattern(); // Qt 5
#else
  const QString keyword = filterRegularExpression().pattern(); // Qt 6
#endif
  if (pAccountsList == nullptr || keyword.isEmpty()) {
    return true;
  }
  return (*pAccountsList)[sourceRow].IsContainsKeyWords(keyword);
}

const QMap<QString, QIcon>& LoadTypeIcons() {
  static const QMap<QString, QIcon> type2Icons {
      {"unionpay", QIcon(":/bankcard/UNIONPAY")},
      {"mastercard", QIcon(":/bankcard/MASTERCARD")},
      {"visa", QIcon(":/bankcard/VISA")},
      {"bank", QIcon(":/type/BANK")},
      {"society", QIcon(":/type/SOCIETY")},
      {"email", QIcon(":/type/EMAIL")},
      {"game", QIcon(":/type/GAME")},
      {"tool", QIcon(":/type/TOOL")},
  };
  return type2Icons;
}

PwdTableModel::PwdTableModel(QObject* parent)
  : QAbstractTableModelPub{parent} {
  mType2Icon = LoadTypeIcons();
  mLoadResult = mAccountsList.LoadAccounts();
}

QVariant PwdTableModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }
  if (!(0 <= index.row() && index.row() < rowCount())) {
    return QVariant();
  }
  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    const int section = index.column();
    switch (section) {
      case PwdPublicVariable::PwdTypeE::INDEX:
        return index.row();
      case PwdPublicVariable::PwdTypeE::TYPE:
        return mAccountsList[index.row()].typeStr;
      case PwdPublicVariable::PwdTypeE::NAME:
        return mAccountsList[index.row()].nameStr;
      case PwdPublicVariable::PwdTypeE::ACCOUNT:
        return mAccountsList[index.row()].accountStr;
      default:
        return QVariant();
    }
  } else if (role == Qt::TextAlignmentRole) {
    return int(Qt::AlignLeft);
  } else if (role == Qt::DecorationRole) {
    if (index.column() == 1) {
      return mType2Icon.value(mAccountsList[index.row()].typeStr.toLower(), {});
    }
  }
  return {};
}

bool PwdTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (role == Qt::EditRole) { // 0: value
    const int section = index.column();
    switch (section) {
      case PwdPublicVariable::PwdTypeE::TYPE:
        mAccountsList[index.row()].typeStr = value.toString();
        break;
      case PwdPublicVariable::PwdTypeE::NAME:
        mAccountsList[index.row()].nameStr = value.toString();
        break;
      case PwdPublicVariable::PwdTypeE::ACCOUNT:
        mAccountsList[index.row()].accountStr = value.toString();
        break;
      default:
        return false;
    }
    mAccountsList[index.row()].SetDetailModified();
    emit dataChanged(index, index, {Qt::DisplayRole});
  }
  return true;
}

Qt::ItemFlags PwdTableModel::flags(const QModelIndex& index) const {
  if (index.column() == PwdPublicVariable::PwdTypeE::INDEX) {
    return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
  }
  return Qt::ItemFlag::ItemIsEditable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
}


int PwdTableModel::RemoveIndexes(const std::set<int>& rows) {
  if (rows.empty()) {
    LOG_D("no row need to be delete");
    return 0;
  }
  const int cnt = rows.size();
  const int before = rowCount();
  if (cnt > before) {
    LOG_W("delete row(s) count %d > total count %d", cnt, before);
    return -1;
  }
  const int after = before - cnt;
  RowsCountBeginChange(before, after);
  int rowsDeleted = mAccountsList.RemoveIndexes(rows);
  LOG_D("%d/%d row(s) were deleted", rowsDeleted, cnt);
  RowsCountEndChange();
  return rowsDeleted;
}

int PwdTableModel::InsertNRows(int indexBefore, int cnt) {
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

int PwdTableModel::AppendAccountRecords(const QVector<AccountInfo>& tempAccounts) {
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

bool PwdTableModel::ExportToPlainCSV() const {
  return mAccountsList.SaveAccounts(false);
}

PwdPublicVariable::SAVE_RESULT PwdTableModel::onSave(QString* detailMessage) {
  using namespace PwdPublicVariable;
  if (!IsDirty()) {
    LOG_D("Nothing changed. No need to save at all");
    return SAVE_RESULT::SKIP;
  }
  QString detailMsg = mAccountsList.GetRowChangeDetailMessage();
  bool saveResult = mAccountsList.SaveAccounts(true);
  if (!saveResult) {
    LOG_W("Save record(s) failed");
    return SAVE_RESULT::FAILED;
  }
  if (detailMessage != nullptr) {
    *detailMessage = detailMsg;
  }
  LOG_D("Save [%s] succeed.", qPrintable(detailMsg));
  return SAVE_RESULT::OK;
}

AccountInfo* PwdTableModel::rowDataAt(int index) {
  if (index < 0 || index > rowCount()) {
    return nullptr;
  }
  return &mAccountsList[index];
}
