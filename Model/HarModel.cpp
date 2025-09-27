#include "HarModel.h"
#include "DataFormatter.h"

const QStringList HarModel::HAR_VERTICAL_HEAD {"Name", "Size(Byte)", "Type", "Url"};

int HarModel::setRootPath(const QString& harFileAbsPath) {
  int before = rowCount();

#ifdef RUNNING_UNIT_TESTS
  HarFiles rhs = HarFilesMocker::mockHarFiles();
#else
  HarFiles rhs;
  rhs(harFileAbsPath);
#endif
  int after = rhs.size();

  RowsCountBeginChange(before, after);
  mHarParser.swap(rhs);
  RowsCountEndChange();

  return after;
}


QVariant HarModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }
  if (!(0 <= index.row() && index.row() < rowCount())) { return {}; }
  if (role == Qt::DisplayRole) {
    const int section = index.column();
    switch (section) {
      case 0:
        return mHarParser[index.row()].name;
      case 1:
        return DataFormatter::formatFileSizeGMKB(mHarParser[index.row()].content.size());
      case 2:
        return mHarParser[index.row()].type;
      case 3:
        return mHarParser[index.row()].url;
      default:
        return QVariant();
    }
  } else if (role == Qt::TextAlignmentRole) {
    return Qt::AlignLeft;
  } else if (role == Qt::DecorationRole) {
    if (index.column() == 0) {
      return m_iconProvider.icon(QFileInfo{mHarParser[index.row()].type});
    }
  }
  return QVariant();
}

int HarModel::SaveToLocal(QString dstRootpath, const QList<int>& selectedRows) {
  return mHarParser.SaveToLocal(dstRootpath, selectedRows);
}

const HAR_FILE_ITEM& HarModel::GetHarEntryItem(const int rowIndex) const {
  return mHarParser[rowIndex];
}
