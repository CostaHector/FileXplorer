#include "JsonTableModel.h"
#include "public/PublicVariable.h"
#include "Tools/NameTool.h"
#include "Tools/Json/DataFormatter.h"
#include <QBrush>
#include <QDir>
#include <QDirIterator>

JsonTableModel::JsonTableModel(QObject* object)  //
    : QAbstractTableModelPub{object}             //
{
  return;
}

QVariant JsonTableModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return {};
  }
  const auto& item = mCachedJsons[index.row()];
  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    switch (index.column()) {
#define JSON_KEY_ITEM(enu, val, def, enhanceDef, format, writer) \
  case enu:                                                      \
    return format(item.m_##enu);  //
      JSON_MODEL_FIELD_MAPPING    //
#undef JSON_KEY_ITEM              //
          default : return {};
    }
  }
  return {};
}

QVariant JsonTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
  switch (role) {
    case Qt::DisplayRole: {
      if (orientation == Qt::Orientation::Vertical) {
        return section + 1;
      }
      return JsonKey::JSON_KEY_IND_2_NAME[section];
    }
    case Qt::TextAlignmentRole: {
      if (orientation == Qt::Vertical) {
        return Qt::AlignRight;
      }
      break;
    }
    case Qt::ForegroundRole: {
      if (orientation == Qt::Vertical && m_modifiedRows.contains(section)) {
        return QBrush(Qt::GlobalColor::red);
      }
      break;
    }
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

bool JsonTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (index.column() == JsonKey::Prepath) {  // ignore it
    return false;
  }
  if (role == Qt::EditRole) {
    auto& item = mCachedJsons[index.row()];
    switch (index.column()) {
#define JSON_KEY_ITEM(enu, val, def, enhanceDef, format, writer) \
  case enu: {                                                    \
    if (!writer(item.m_##enu, value)) {                          \
      return false;                                              \
    }                                                            \
    break;                                                       \
  }
      JSON_MODEL_FIELD_MAPPING  //
#undef JSON_KEY_ITEM            //
          default : return false;
    }
    setModified(index.row(), true);
    emit dataChanged(index, index, {Qt::DisplayRole});
  }
  return QAbstractItemModel::setData(index, value, role);
}

int JsonTableModel::ReadADirectory(const QString& path) {
  if (!QFileInfo(path).isDir()) {
    qDebug("path[%s] is not a dir", qPrintable(path));
    return -1;
  }
  decltype(mCachedJsons) tempCachedJsons;
  QDirIterator it{path, TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  while (it.hasNext()) {
    it.next();
    tempCachedJsons.append(JsonPr{it.filePath()});
  }

  const int befRowCnt = mCachedJsons.size();
  const int afterRowCnt = tempCachedJsons.size();
  RowsCountBeginChange(befRowCnt, afterRowCnt);
  mCachedJsons.swap(tempCachedJsons);
  RowsCountEndChange();
  return afterRowCnt;
}

int JsonTableModel::AppendADirectory(const QString& path) {
  if (!QFileInfo(path).isDir()) {
    qDebug("path[%s] is not a dir", qPrintable(path));
    return -1;
  }
  QSet<QString> alreadyExistedSet;
  for (const auto& item : mCachedJsons) {
    alreadyExistedSet.insert(item.m_Prepath + '/' + item.jsonFileName);
  }
  decltype(mCachedJsons) appendCachedJsons;
  QDirIterator it{path, TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  QString jsonPth;
  while (it.hasNext()) {
    it.next();
    jsonPth = it.filePath();
    if (alreadyExistedSet.contains(jsonPth)) {
      continue;
    }
    appendCachedJsons.append(JsonPr{jsonPth});
  }
  const int befRowCnt = mCachedJsons.size();
  const int afterRowCnt = mCachedJsons.size() + appendCachedJsons.size();
  RowsCountBeginChange(befRowCnt, afterRowCnt);
  mCachedJsons.reserve(afterRowCnt);
  std::move(appendCachedJsons.begin(), appendCachedJsons.end(), std::back_inserter(mCachedJsons));
  RowsCountEndChange();
  return afterRowCnt;
}

void JsonTableModel::setModified(int row, bool modified) {
  if (modified) {
    m_modifiedRows.insert(row);
  } else {
    m_modifiedRows.remove(row);
  }
  emit headerDataChanged(Qt::Vertical, row, row);
}
