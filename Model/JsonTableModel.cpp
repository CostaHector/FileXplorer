#include "JsonTableModel.h"
#include "NameTool.h"
#include "StudiosManager.h"
#include "PublicVariable.h"
#include "DataFormatter.h"
#include "CastManager.h"
#include "PathTool.h"
#include "GeneralDataType.h"
#include "BatchRenameBy.h"
#include "SceneMixed.h"
#include "JsonUpdater.h"
#include <QIcon>
#include <QBrush>
#include <QDir>
#include <QDirIterator>
#include <QTextCharFormat>

constexpr int JsonTableModel::DATA_TYPE_ROLE;

QVariant JsonTableModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return {};
  }
  const auto& item = mCachedJsons[index.row()];
  const int col = index.column();
  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    switch (col) {
#define JSON_KEY_ITEM(enu, val, def, enhanceDef, generalDataType, format, writer, initer, jsonWriter) \
      case JsonModelField::FIELD_E::enu: return format(item.m_##enu); //
      JSON_MODEL_FIELD_MAPPING   //
#undef JSON_KEY_ITEM             //
      default: return {};
    }
  } else if (role == Qt::DecorationRole && col == JsonModelField::FIELD_E::ContentFixed) {
    if (item.m_ContentFixed) {
      static const QIcon CONTENTS_FIXED_IMG{":/JsonEditor/ANCHOR_DROP"};
      return CONTENTS_FIXED_IMG;
    }
  } else if (role == Qt::ForegroundRole) {
    switch (col) {
      case JsonModelField::FIELD_E::Cast: {
        if (!item.hintCast.isEmpty()) {
          return QColor{Qt::GlobalColor::red};
        }
        break;
      }
      case JsonModelField::FIELD_E::Studio: {
        if (!item.hintStudio.isEmpty()) {
          return QColor{Qt::GlobalColor::red};
        }
        static const auto& studiosTable = StudiosManager::getInst().StdStudiosSet();
        if (!item.m_Studio.isEmpty() && !studiosTable.contains(item.m_Studio)) {
          return QColor{Qt::GlobalColor::darkRed};
        }
      }
      default:
        break;
    }
  } else if (role == JsonTableModel::DATA_TYPE_ROLE) {
    switch (col) {
#define JSON_KEY_ITEM(enu, val, def, enhanceDef, generalDataType, format, writer, initer, jsonWriter) \
      case JsonModelField::FIELD_E::enu: return generalDataType; //
        JSON_MODEL_FIELD_MAPPING    //
#undef JSON_KEY_ITEM                //
      default: return GeneralDataType::Type::ERROR_TYPE;
    }
  }
  return {};
}

QVariant JsonTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
  switch (role) {
    case Qt::DisplayRole: {
      if (0 <= section && section < columnCount() && orientation == Qt::Orientation::Horizontal) {
        return JsonModelField::JSON_TABLE_HEADERS[section];
      }
      return section + 1;
    }
    case Qt::TextAlignmentRole: {
      if (orientation == Qt::Vertical) {
        return Qt::AlignRight;
      }
      break;
    }
    case Qt::ForegroundRole: {
      if (orientation == Qt::Vertical && (0 <= section && section < rowCount()) && mCachedJsons[section].bModified) {
        return QBrush(Qt::GlobalColor::red);
      }
      break;
    }
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

bool JsonTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (index.column() == JsonModelField::FIELD_E::Prepath) {  // ignore it
    return false;
  }
  if (role == Qt::EditRole) {
    auto& item = mCachedJsons[index.row()];
    switch (index.column()) {
#define JSON_KEY_ITEM(enu, val, def, enhanceDef, generalDataType, format, writer, initer, jsonWriter) \
  case JsonModelField::FIELD_E::enu: {                                                                        \
    if (!writer(item.m_##enu, value)) {                                              \
      return false;                                                                  \
    }                                                                                \
    break;                                                                           \
  }
      JSON_MODEL_FIELD_MAPPING  //
#undef JSON_KEY_ITEM            //
          default : return false;
    }
    setModified(index.row(), true);
    // what you see is what you get, no need emit change signal; avoid cursor selection be cleared
    if (index.column() != JsonModelField::FIELD_E::Detail) {
      emit dataChanged(index, index, {Qt::DisplayRole});
    }
    return true;
  }
  return QAbstractItemModel::setData(index, value, role);
}

int JsonTableModel::setRootPath(const QString& path, bool isForce) {
  if (mRootPath == path && !isForce) {
    LOG_D("Path[%s] unchange", qPrintable(path));
    return 0;
  }

  mRootPath = path;
  if (!QFileInfo(path).isDir()) {
    LOG_D("path[%s] is not a dir", qPrintable(path));
    return -1;
  }

  QVector<JsonPr> tempCachedJsons;
  QDirIterator it{path, TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  while (it.hasNext()) {
    it.next();
    tempCachedJsons.append(JsonPr::fromJsonFile(it.filePath()));
  }
  std::sort(tempCachedJsons.begin(), tempCachedJsons.end());

  const int befRowCnt = mCachedJsons.size();
  const int afterRowCnt = tempCachedJsons.size();
  RowsCountBeginChange(befRowCnt, afterRowCnt);
  mCachedJsons.swap(tempCachedJsons);
  RowsCountEndChange();
  return afterRowCnt;
}

int JsonTableModel::forceReloadPath() {
  return setRootPath(mRootPath, true);
}

QFileInfo JsonTableModel::fileInfo(const QModelIndex& index) const {
  int row = index.row();
  if (row < 0 || row >= rowCount()) {
    LOG_W("row: %d out of range", row);
    return {};
  }
  return QFileInfo{mCachedJsons[row].GetJsonFileAbsPath()};
}

QString JsonTableModel::filePath(const QModelIndex& index) const {
  int row = index.row();
  if (row < 0 || row >= rowCount()) {
    LOG_W("row: %d out of range", row);
    return {};
  }
  return mCachedJsons[row].GetJsonFileAbsPath();
}

QString JsonTableModel::fileName(const QModelIndex& index) const {
  int row = index.row();
  if (row < 0 || row >= rowCount()) {
    LOG_W("row: %d out of range", row);
    return {};
  }
  return mCachedJsons[row].jsonFileName;
}

QString JsonTableModel::fileBaseName(const QModelIndex& index) const {
  return PathTool::GetBaseName(fileName(index));
}

QString JsonTableModel::absolutePath(const QModelIndex& index) const {
  int row = index.row();
  if (row < 0 || row >= rowCount()) {
    LOG_W("row: %d out of range", row);
    return {};
  }
  return mCachedJsons[row].m_Prepath;
}

QString JsonTableModel::fullInfo(const QModelIndex& index) const {
  int row = index.row();
  if (row < 0 || row >= rowCount()) {
    LOG_W("row: %d out of range", row);
    return {};
  }
  return mCachedJsons[row].GetJsonBA();
}

QStringList JsonTableModel::rel2fileNames(const QModelIndexList& indexes) const {
  // full: "/home/to/a.json"
  // root: "/home"
  // rel2fileNames: "to/a.json"
  QStringList relativePaths2FileName;
  relativePaths2FileName.reserve(indexes.size());
  const int N = rootPath().size();
  for (const QModelIndex& index : indexes) {
    int row = index.row();
    if (row < 0 || row >= rowCount()) {
      LOG_W("row: %d out of range", row);
      return {};
    }
    const QString& fullPath = mCachedJsons[row].GetJsonFileAbsPath();
    relativePaths2FileName.push_back(fullPath.mid(N + 1));
  }
  return relativePaths2FileName;
}

QStringList JsonTableModel::relativePath2RelatedFiles(const QModelIndexList& indexes) const {
  const QStringList& jsonFileNames{rel2fileNames(indexes)};
  return BatchRenameBy::GetFilesNeedProcess(rootPath(), jsonFileNames);
}

bool JsonTableModel::setModified(int row, bool modified) {
  if (!setModifiedNoEmit(row, modified)) {
    return false;
  }
  emit headerDataChanged(Qt::Vertical, row, row);
  return true;
}

bool JsonTableModel::setModifiedNoEmit(int row, bool modified) {
  if (row < 0 || row >= rowCount()) {
    return false;
  }
  mCachedJsons[row].bModified = modified;
  return true;
}

int JsonTableModel::SetStudio(const QModelIndexList& rowIndexes, const QString& studio) {
  int affectedRows{0};
  int row{-1};
  int minRow{INT_MAX}, maxRow{-1};
  for (const QModelIndex& ind : rowIndexes) {
    row = ind.row();
    if (row < 0 || row >= rowCount()) {
      LOG_W("row: %d out of range [0,%d)", row, rowCount());
      return affectedRows;
    }
    if (mCachedJsons[row].m_Studio == studio) {
      continue;
    }
    ++affectedRows;
    mCachedJsons[row].m_Studio = studio;
    setModifiedNoEmit(row, true);
    if (row > maxRow) {
      maxRow = row;
    }
    if (row < minRow) {
      minRow = row;
    }
  }
  if (maxRow < 0 || minRow > maxRow) {
    LOG_W("Studio Field of %d row(s) NO change at all", rowIndexes.size());
    return 0;
  }
  const QModelIndex& frontInd = sibling(minRow, JsonModelField::FIELD_E::Studio, {});
  const QModelIndex& backInd = sibling(maxRow, JsonModelField::FIELD_E::Studio, {});
  emit dataChanged(frontInd, backInd, {Qt::DisplayRole});
  emit headerDataChanged(Qt::Vertical, minRow, maxRow);
  LOG_D("Studio Field of %d/%d row(s) range [%d, %d) changed to [%s]", affectedRows, rowIndexes.size(), minRow, maxRow, qPrintable(studio));
  return affectedRows;
}

int JsonTableModel::SetCastOrTags(const QModelIndexList& rowIndexes, JsonModelField::FIELD_E keyEnum, const QString& sentence) {
  if (keyEnum != JsonModelField::FIELD_E::Cast && keyEnum != JsonModelField::FIELD_E::Tags) {
    LOG_W("Field[%d] not support", (int)keyEnum);
    return -1;
  }

  const SortedUniqStrLst newLst{sentence};
  int affectedRows{0};
  int row{-1};
  int minRow{INT_MAX}, maxRow{-1};
  for (const QModelIndex& ind : rowIndexes) {
    row = ind.row();
    if (row < 0 || row >= rowCount()) {
      LOG_W("row: %d out of range [0,%d)", row, rowCount());
      return affectedRows;
    }
    auto& targetField = (keyEnum == JsonModelField::FIELD_E::Cast) ? mCachedJsons[row].m_Cast : mCachedJsons[row].m_Tags;
    if (targetField == newLst) {
      continue;
    }
    targetField = newLst;
    setModifiedNoEmit(row, true);
    if (row > maxRow) {
      maxRow = row;
    }
    if (row < minRow) {
      minRow = row;
    }
    ++affectedRows;
  }
  if (maxRow < 0 || minRow > maxRow) {
    LOG_W("Cast or Tags Field[%d] of %d row(s) NO change at all", keyEnum, rowIndexes.size());
    return 0;
  }
  const QModelIndex& frontInd = sibling(minRow, keyEnum, {});
  const QModelIndex& backInd = sibling(maxRow, keyEnum, {});
  emit dataChanged(frontInd, backInd, {Qt::DisplayRole});
  emit headerDataChanged(Qt::Vertical, minRow, maxRow);
  LOG_D("Cast or Tags Field[%d] of %d/%d row(s) range [%d, %d) set [%s]", keyEnum, affectedRows, rowIndexes.size(), minRow, maxRow,
        qPrintable(sentence));
  return affectedRows;
}

int JsonTableModel::AddCastOrTags(const QModelIndexList& rowIndexes, const JsonModelField::FIELD_E keyEnum, const QString& sentence) {
  if (sentence.isEmpty()) {
    LOG_D("No need add empty to cast or tags field[%d]", keyEnum);
    return 0;
  }
  if (keyEnum != JsonModelField::FIELD_E::Cast && keyEnum != JsonModelField::FIELD_E::Tags) {
    LOG_W("Field[%d] not support", (int)keyEnum);
    return -1;
  }

  const SortedUniqStrLst appendContainer{sentence};

  int affectedRows{0};
  int row{-1};
  int minRow{INT_MAX}, maxRow{-1};
  for (const QModelIndex& ind : rowIndexes) {
    row = ind.row();
    if (row < 0 || row >= rowCount()) {
      LOG_W("row: %d out of range [0,%d)", row, rowCount());
      return affectedRows;
    }
    auto& targetField = (keyEnum == JsonModelField::FIELD_E::Cast) ? mCachedJsons[row].m_Cast : mCachedJsons[row].m_Tags;
    if (targetField == appendContainer) {
      continue;
    }
    targetField += appendContainer;
    setModifiedNoEmit(row, true);
    if (row > maxRow) {
      maxRow = row;
    }
    if (row < minRow) {
      minRow = row;
    }
    ++affectedRows;
  }
  if (maxRow < 0 || minRow > maxRow) {
    LOG_W("Cast or Tags Field[%d] of %d row(s) NO change at all", keyEnum, rowIndexes.size());
    return 0;
  }
  const QModelIndex& frontInd = sibling(minRow, keyEnum, {});
  const QModelIndex& backInd = sibling(maxRow, keyEnum, {});
  emit dataChanged(frontInd, backInd, {Qt::DisplayRole});
  emit headerDataChanged(Qt::Vertical, minRow, maxRow);
  LOG_D("Cast or Tags Field[%d] of %d/%d row(s) range [%d, %d) Add [%s]", keyEnum, affectedRows, rowIndexes.size(), minRow, maxRow,
        qPrintable(sentence));
  return affectedRows;
}

int JsonTableModel::RmvCastOrTags(const QModelIndexList& rowIndexes, const JsonModelField::FIELD_E keyEnum, const QString& oneElement) {
  if (oneElement.isEmpty()) {
    LOG_D("No need remove empty from cast or tags field[%d]", keyEnum);
    return 0;
  }

  if (keyEnum != JsonModelField::FIELD_E::Cast && keyEnum != JsonModelField::FIELD_E::Tags) {
    LOG_W("Field[%d] not support", (int)keyEnum);
    return -1;
  }

  int affectedRows{0};
  int row{-1};
  int minRow{INT_MAX}, maxRow{-1};
  for (const QModelIndex& ind : rowIndexes) {
    row = ind.row();
    if (row < 0 || row >= rowCount()) {
      LOG_W("row: %d out of range [0,%d)", row, rowCount());
      return affectedRows;
    }
    auto& targetField = (keyEnum == JsonModelField::FIELD_E::Cast) ? mCachedJsons[row].m_Cast : mCachedJsons[row].m_Tags;
    if (!targetField.remove(oneElement)) {
      continue;
    }
    setModifiedNoEmit(row, true);
    ++affectedRows;
    if (row > maxRow) {
      maxRow = row;
    }
    if (row < minRow) {
      minRow = row;
    }
  }
  if (maxRow < 0 || minRow > maxRow) {
    LOG_W("Cast or Tags Field[%d] of %d row(s) NO change at all", keyEnum, rowIndexes.size());
    return 0;
  }
  const QModelIndex& frontInd = sibling(minRow, keyEnum, {});
  const QModelIndex& backInd = sibling(maxRow, keyEnum, {});
  emit dataChanged(frontInd, backInd, {Qt::DisplayRole});
  emit headerDataChanged(Qt::Vertical, minRow, maxRow);
  LOG_D("Cast or Tags Field[%d] of %d/%d row(s) range [%d, %d) remove element[%s]", keyEnum, affectedRows, rowIndexes.size(), minRow, maxRow,
        qPrintable(oneElement));
  return affectedRows;
}

int JsonTableModel::InitCastAndStudio(const QModelIndexList& rowIndexes) {
  int affecteRows{0};
  int row{-1};
  int minRow{INT_MAX}, maxRow{-1};

  for (const QModelIndex& ind : rowIndexes) {
    row = ind.row();
    if (row < 0 || row >= rowCount()) {
      LOG_W("row: %d out of range [0,%d)", row, rowCount());
      return affecteRows;
    }
    if (mCachedJsons[row].m_ContentFixed) {  // will not be influenced
      continue;
    }
    if (!mCachedJsons[row].ConstructCastStudioValue()) {
      continue;
    }

    setModifiedNoEmit(row, true);
    ++affecteRows;
    if (row > maxRow) {
      maxRow = row;
    }
    if (row < minRow) {
      minRow = row;
    }
  }
  if (maxRow < 0 || minRow > maxRow) {
    LOG_W("Cast and Tags Field of %d row(s) NO init at all", rowIndexes.size());
    return 0;
  }
  const QModelIndex& castFrontInd = sibling(minRow, JsonModelField::FIELD_E::Cast, {});
  const QModelIndex& castBackInd = sibling(maxRow, JsonModelField::FIELD_E::Cast, {});
  emit dataChanged(castFrontInd, castBackInd, {Qt::DisplayRole});
  emit headerDataChanged(Qt::Vertical, minRow, maxRow);

  const QModelIndex& studioFrontInd = sibling(minRow, JsonModelField::FIELD_E::Studio, {});
  const QModelIndex& studioBackInd = sibling(maxRow, JsonModelField::FIELD_E::Studio, {});
  emit dataChanged(studioFrontInd, studioBackInd, {Qt::DisplayRole});
  emit headerDataChanged(Qt::Vertical, minRow, maxRow);
  LOG_D("Cast and Tags Field of %d/%d row(s) range [%d, %d) init ok", affecteRows, rowIndexes.size(), minRow, maxRow);
  return affecteRows;
}

int JsonTableModel::HintCastAndStudio(const QModelIndexList& rowIndexes, const QString& sentence) {
  int studioCnt{0}, castCnt{0};
  int row{-1};
  int studioMinRow{INT_MAX}, studioMaxRow{-1};
  int castMinRow{INT_MAX}, castMaxRow{-1};

  bool studioChanged{false}, castChanged{false};
  for (const QModelIndex& ind : rowIndexes) {
    row = ind.row();
    if (row < 0 || row >= rowCount()) {
      LOG_W("row: %d out of range [0,%d)", row, rowCount());
      return studioCnt;
    }
    auto& item = mCachedJsons[row];
    if (item.m_ContentFixed) {  // will not be influenced
      continue;
    }
    item.HintForCastStudio(sentence, studioChanged, castChanged);
    if (!studioChanged && !castChanged) {
      continue;
    }
    // hintCast or hintStudio changed
    setModifiedNoEmit(row, true);
    // studio update in-place;
    if (studioChanged) {
      item.m_Studio = item.hintStudio;
      if (row > studioMaxRow) {
        studioMaxRow = row;
      }
      if (row < studioMinRow) {
        studioMinRow = row;
      }
      ++studioCnt;
    }
    // hintcast append back
    if (castChanged) {
      item.m_Cast.insertBatchFromSentence(item.hintCast);
      if (row > castMaxRow) {
        castMaxRow = row;
      }
      if (row < castMinRow) {
        castMinRow = row;
      }
      ++castCnt;
    }
  }
  if (studioMaxRow < 0 || studioMinRow > studioMaxRow) {
    LOG_W("Studio Field of %d row(s) NO hint at all", rowIndexes.size());
  } else {
    const QModelIndex& studioFrontInd = sibling(studioMinRow, JsonModelField::FIELD_E::Studio, {});
    const QModelIndex& studioBackInd = sibling(studioMaxRow, JsonModelField::FIELD_E::Studio, {});
    emit dataChanged(studioFrontInd, studioBackInd, {Qt::DisplayRole});
    emit headerDataChanged(Qt::Vertical, studioMinRow, studioMaxRow);
    LOG_D("Studio Field of %d/%d row(s) range [%d, %d) hint ok", studioCnt, rowIndexes.size(), studioMinRow, studioMaxRow);
  }

  if (castMaxRow < 0 || castMinRow > castMaxRow) {
    LOG_W("Cast Field of %d row(s) NO hint at all", rowIndexes.size());
  } else {
    const QModelIndex& castFrontInd = sibling(castMinRow, JsonModelField::FIELD_E::Cast, {});
    const QModelIndex& castBackInd = sibling(castMaxRow, JsonModelField::FIELD_E::Cast, {});
    emit dataChanged(castFrontInd, castBackInd, {Qt::ForegroundRole | Qt::DisplayRole});
    emit headerDataChanged(Qt::Vertical, castMinRow, castMaxRow);
    LOG_D("Cast Field of %d/%d row(s) range [%d, %d) hint ok", castCnt, rowIndexes.size(), castMinRow, castMaxRow);
  }

  return studioCnt + castCnt;
}

int JsonTableModel::FormatCast(const QModelIndexList& rowIndexes) {
  int affectedRows{0};
  int row{-1};
  int minRow{INT_MAX}, maxRow{-1};
  for (const QModelIndex& ind : rowIndexes) {
    row = ind.row();
    if (row < 0 || row >= rowCount()) {
      LOG_W("row: %d out of range [0,%d)", row, rowCount());
      return affectedRows;
    }
    mCachedJsons[row].m_Cast.format();
    setModifiedNoEmit(row, true);
    if (row > maxRow) {
      maxRow = row;
    }
    if (row < minRow) {
      minRow = row;
    }
    ++affectedRows;
  }
  if (maxRow < 0 || minRow > maxRow) {
    LOG_W("Cast Field of %d row(s) NO format at all", rowIndexes.size());
    return 0;
  }
  const QModelIndex& frontInd = sibling(minRow, JsonModelField::FIELD_E::Cast, {});
  const QModelIndex& backInd = sibling(maxRow, JsonModelField::FIELD_E::Cast, {});
  emit dataChanged(frontInd, backInd, {Qt::DisplayRole});
  emit headerDataChanged(Qt::Vertical, minRow, maxRow);
  LOG_D("Cast Field of %d/%d row(s) range [%d, %d) format ok", affectedRows, rowIndexes.size(), minRow, maxRow);
  return affectedRows;
}

QHash<QString, QString> JsonTableModel::GetVidBaseName2FullPath() const {
  QHash<QString, QString> vidBaseName2FullPath;
  QDirIterator it{mRootPath, TYPE_FILTER::VIDEO_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  while (it.hasNext()) {
    QString vidFullPath = it.next();
    QString vidBaseName = PathTool::GetFileNameExtRemoved(vidFullPath);
    vidBaseName2FullPath[vidBaseName] = vidFullPath;
  }
  return vidBaseName2FullPath;
}

int JsonTableModel::JsonFieldValueUpdateCore(const QModelIndexList& rowIndexes, JsonModelField::FIELD_E field, const int ITERATE_FOLDER_FIRST_LIMIT) {
  JsonPr::UPDATER_FUNC func{nullptr};
  switch (field) {
    case JsonModelField::FIELD_E::Size:
      func = &JsonPr::UpdateVideoSizeField;
      break;
    case JsonModelField::FIELD_E::Duration:
      func = &JsonPr::UpdateDurationField;
      break;
    case JsonModelField::FIELD_E::MD5:
      func = &JsonPr::UpdateVideoMD5Field;
      break;
    default:
      LOG_W("The field[%d] no support update", field);
      return -1;
  }
  if (rowIndexes.isEmpty()) {
    LOG_D("no row need update at all");
    return 0;
  }

  QHash<QString, QString> vidBaseName2FullPath;
  if (rowIndexes.size() >= ITERATE_FOLDER_FIRST_LIMIT) {
    vidBaseName2FullPath = GetVidBaseName2FullPath();
  }

  int affectedRows{0};
  int row{-1};
  int minRow{INT_MAX}, maxRow{-1};
  QString vidFullPath;
  for (const QModelIndex& ind : rowIndexes) {
    row = ind.row();
    if (row < 0 || row >= rowCount()) {
      LOG_W("row: %d out of range [0,%d)", row, rowCount());
      return affectedRows;
    }
    if (!vidBaseName2FullPath.isEmpty()) {
      const QString& jsonFullPath = mCachedJsons[row].GetJsonFileAbsPath();
      const QString& jsonBaseName = PathTool::GetFileNameExtRemoved(jsonFullPath);
      vidFullPath = vidBaseName2FullPath.value(jsonBaseName, "");
    } else {
      vidFullPath.clear();
    }
    affectedRows += (int)(mCachedJsons[row].*func)(vidFullPath);
    setModifiedNoEmit(row, true);
    if (row > maxRow) {
      maxRow = row;
    }
    if (row < minRow) {
      minRow = row;
    }
  }
  if (maxRow < 0 || minRow > maxRow) {
    LOG_W("Cast Field of %d row(s) NO format at all", rowIndexes.size());
    return 0;
  }
  const QModelIndex& frontInd = sibling(minRow, field, {});
  const QModelIndex& backInd = sibling(maxRow, field, {});
  emit dataChanged(frontInd, backInd, {Qt::DisplayRole});
  emit headerDataChanged(Qt::Vertical, minRow, maxRow);
  return affectedRows;
}

int JsonTableModel::UpdateFizeSizeField(const QModelIndexList& rowIndexes, const int ITERATE_FOLDER_FIRST_LIMIT) {
  return JsonFieldValueUpdateCore(rowIndexes, JsonModelField::FIELD_E::Size, ITERATE_FOLDER_FIRST_LIMIT);
}

int JsonTableModel::UpdateDurationField(const QModelIndexList& rowIndexes, const int ITERATE_FOLDER_FIRST_LIMIT) {
  return JsonFieldValueUpdateCore(rowIndexes, JsonModelField::FIELD_E::Duration, ITERATE_FOLDER_FIRST_LIMIT);
}

int JsonTableModel::UpdateMD5Field(const QModelIndexList& rowIndexes, const int ITERATE_FOLDER_FIRST_LIMIT) {
  return JsonFieldValueUpdateCore(rowIndexes, JsonModelField::FIELD_E::MD5, ITERATE_FOLDER_FIRST_LIMIT);
}

int JsonTableModel::SyncFieldNameByJsonBaseName(const QModelIndexList& rowIndexes) {
  int cnt{0};
  int row{-1};
  int minRow{INT_MAX}, maxRow{-1};
  for (const QModelIndex& ind : rowIndexes) {
    row = ind.row();
    if (row < 0 || row >= rowCount()) {
      LOG_W("row: %d out of range [0,%d)", row, rowCount());
      return cnt;
    }
    if (!mCachedJsons[row].SyncNameValueFromFileBaseName()) {
      continue;
    }
    setModifiedNoEmit(row, true);
    if (row > maxRow) {
      maxRow = row;
    }
    if (row < minRow) {
      minRow = row;
    }
    ++cnt;
  }
  if (maxRow < 0 || minRow > maxRow) {
    LOG_W("Name Field of %d row(s) NO sync at all", rowIndexes.size());
    return 0;
  }
  const QModelIndex& frontInd = sibling(minRow, JsonModelField::FIELD_E::Name, {});
  const QModelIndex& backInd = sibling(maxRow, JsonModelField::FIELD_E::Name, {});
  emit dataChanged(frontInd, backInd, {Qt::DisplayRole});
  emit headerDataChanged(Qt::Vertical, minRow, maxRow);
  LOG_D("Name Field of %d/%d row(s) range [%d, %d) sync ok", cnt, rowIndexes.size(), minRow, maxRow);
  return cnt;
}

// after call it reload and sync Name field to file Name needed
int JsonTableModel::AfterJsonFilesNameRenamed(const QModelIndexList& indexes) {
  const auto rowElementsRmv = [this](int beg, int end) { mCachedJsons.erase(mCachedJsons.begin() + beg, mCachedJsons.begin() + end); };
  return onRowsRemoved(indexes, rowElementsRmv);
}

int JsonTableModel::SaveCurrentChanges(const QModelIndexList& rowIndexes) {
  int cnt{0};
  int row{-1};
  int minRow{INT_MAX}, maxRow{-1};
  for (const QModelIndex& ind : rowIndexes) {
    row = ind.row();
    if (row < 0 || row >= rowCount()) {
      LOG_W("row: %d out of range [0,%d)", row, rowCount());
      return cnt;
    }
    if (!mCachedJsons[row].bModified) {
      continue;
    }
    if (!mCachedJsons[row].WriteIntoFiles()) {
      LOG_W("Write into local file[%s] failed", qPrintable(mCachedJsons[row].GetJsonFileAbsPath()));
      return -1;
    }
    if (row > maxRow) {
      maxRow = row;
    }
    if (row < minRow) {
      minRow = row;
    }
    ++cnt;
  }
  if (maxRow < 0 || minRow > maxRow) {
    LOG_W("Name Field of %d row(s) NO sync at all", rowIndexes.size());
    return 0;
  }
  const QModelIndex& frontInd = sibling(minRow, JsonModelField::FIELD_E::Cast, {});
  const QModelIndex& backInd = sibling(maxRow, JsonModelField::FIELD_E::Tags, {});
  emit dataChanged(frontInd, backInd, {Qt::DisplayRole | Qt::ForegroundRole});
  emit headerDataChanged(Qt::Vertical, minRow, maxRow);
  LOG_D("Changes of %d/%d row(s) range [%d, %d) saved ok", cnt, rowIndexes.size(), minRow, maxRow);
  return cnt;
}

std::pair<int, int> JsonTableModel::ExportCastStudioToLocalDictionaryFile(const QModelIndexList& rowIndexes) const {
  CastManager& castMgr = CastManager::getInst();
  CAST_MGR_DATA_T actorsFromSelection, singleWordActorsFromSelection;

  StudiosManager& stdMgr = StudiosManager::getInst();
  STUDIO_MGR_DATA_T studioIncrements;

  int rowCnt{0};
  int row{-1};
  for (const QModelIndex& ind : rowIndexes) {
    row = ind.row();
    if (row < 0 || row >= rowCount()) {
      LOG_W("row: %d out of range [0,%d)", row, rowCount());
      return {-1, -1};
    }

    const QString& stdStudioName = mCachedJsons[row].m_Studio;
    stdMgr.StudioIncrement(studioIncrements, stdStudioName);

    const CAST_MGR_DATA_T& actors = mCachedJsons[row].m_Cast.toLowerSets();
    if (stdMgr.isStudioWithSingleWord(stdStudioName)) {
      singleWordActorsFromSelection += actors;
    }
    actorsFromSelection += actors;
    ++rowCnt;
  }
  const int studioRet = stdMgr.WriteIntoLocalDictionaryFiles(studioIncrements);
  CAST_MGR_DATA_T actorIncrements = castMgr.ActorIncrement(actorsFromSelection);
  CAST_MGR_DATA_T singleWordActorIncrements = castMgr.SingleWordActorIncrement(singleWordActorsFromSelection);
  const int actorRet = castMgr.WriteIntoLocalDictionaryFiles(actorIncrements, false);
  const int singleWordActorRet = castMgr.WriteIntoLocalDictionaryFiles(singleWordActorIncrements, true);
  LOG_D("Increment of Actors:%d(writeRet:%d), SingleWordActors:%d(writeRet:%d), Studios:%d(writeRet:%d) from %d row(s)",  //
        actorIncrements.size(), actorRet,                                                                                 //
        singleWordActorIncrements.size(), singleWordActorRet,                                                             //
        studioIncrements.size(), studioRet,                                                                               //
        rowCnt);
  return {actorRet, studioRet};
}

QStringList JsonTableModel::CheckMd5AndVidNameConsistency() const {
  QStringList inconsistentFiles;
  for (const JsonPr& jpr: mCachedJsons) {
    if (!jpr.checkMd5AndVidNameConsistency()) {
      inconsistentFiles.push_back(jpr.m_Name);
    }
  }
  return inconsistentFiles;
}

JsonPr JsonTableModel::GetJsonPr(const QModelIndex& ind) const {
  const int row = ind.row();
  if (row < 0 || row >= rowCount()) {
    LOG_W("row: %d out of range [0,%d)", row, rowCount());
    return {};
  }
  return mCachedJsons[row];
}

int JsonTableModel::AppendCastFromSentence(const QModelIndex& ind, const QString& sentence, bool isUpperCaseSentence) {
  int row = ind.row();
  if (row < 0 || row >= rowCount()) {
    LOG_W("row: %d out of range [0,%d)", row, rowCount());
    return -1;
  }
  static const NameTool nt;
  const QStringList& newLst{isUpperCaseSentence ? nt.castFromUpperCaseSentence(sentence) : nt.castFromSentence(sentence)};

  auto& stCast = mCachedJsons[row].m_Cast;
  int beforeCastCnt = stCast.count();
  stCast.insertBatch(newLst);
  int afterCastCnt = stCast.count();
  if (afterCastCnt <= beforeCastCnt) {
    LOG_D("nothing cast increased by selected sentence[%s]", qPrintable(sentence));
    return 0;
  }

  setModifiedNoEmit(row, true);
  const QModelIndex& frontAndBackInd = ind.siblingAtColumn(JsonModelField::FIELD_E::Cast);
  emit dataChanged(frontAndBackInd, frontAndBackInd, {Qt::DisplayRole | Qt::ForegroundRole});
  emit headerDataChanged(Qt::Vertical, row, row);
  LOG_D("%d cast increased by selected sentence[%s]", afterCastCnt - beforeCastCnt, qPrintable(sentence));
  return afterCastCnt - beforeCastCnt;
}

int JsonTableModel::SetRecordContentsFixed(const QModelIndexList& rowIndexes, bool bFixed) {
  int affectedRows{0};
  int row{-1};
  int minRow{INT_MAX}, maxRow{-1};
  for (const QModelIndex& ind : rowIndexes) {
    row = ind.row();
    if (row < 0 || row >= rowCount()) {
      LOG_W("row: %d out of range [0,%d)", row, rowCount());
      return affectedRows;
    }
    if (mCachedJsons[row].m_ContentFixed == bFixed) {
      continue;
    }
    ++affectedRows;
    mCachedJsons[row].m_ContentFixed = bFixed;
    setModifiedNoEmit(row, true);
    if (row > maxRow) {
      maxRow = row;
    }
    if (row < minRow) {
      minRow = row;
    }
  }
  if (maxRow < 0 || minRow > maxRow) {
    LOG_W("ContentFixed Field of %d row(s) NO change at all", rowIndexes.size());
    return 0;
  }
  const QModelIndex& frontInd = sibling(minRow, JsonModelField::FIELD_E::ContentFixed, {});
  const QModelIndex& backInd = sibling(maxRow, JsonModelField::FIELD_E::ContentFixed, {});
  emit dataChanged(frontInd, backInd, {Qt::DisplayRole, Qt::DecorationRole});
  emit headerDataChanged(Qt::Vertical, minRow, maxRow);
  LOG_D("ContentFixed Field of %d/%d row(s) range [%d, %d) changed to bool[%d]", affectedRows, rowIndexes.size(), minRow, maxRow, bFixed);
  return affectedRows;
}

JsonOp::Counter JsonTableModel::UpdateJsonKeyValuePair() {
  JsonOp::Counter counter;
  ScenesMixed sMixed;
  sMixed(rootPath());

  int minRow{INT_MAX}, maxRow{-1};
  for (int row = 0; row < rowCount(); ++row) {
    JsonOp::Counter currentCounter = JsonUpdater::UpdateJsonKeyValuePair(sMixed, mCachedJsons[row]);
    if (currentCounter.m_jsonUsedCnt == 0) { // useless json found
      continue;
    }
    counter += currentCounter;
    if (currentCounter.m_jsonUpdatedCnt == 0) {
      continue;
    }
    setModifiedNoEmit(row, true);
    if (row > maxRow) {
      maxRow = row;
    }
    if (row < minRow) {
      minRow = row;
    }
  }
  if (maxRow < 0 || minRow > maxRow) {
    LOG_W("No Field(s) of %d row(s) updated", rowCount());
    return counter;
  }
  const QModelIndex& frontInd = sibling(minRow, JsonModelField::FIELD_E::Name, {});
  const QModelIndex& backInd = sibling(maxRow, JsonModelField::FIELD_E::Detail, {});
  emit dataChanged(frontInd, backInd, {Qt::DisplayRole | Qt::ForegroundRole});
  emit headerDataChanged(Qt::Vertical, minRow, maxRow);
  return counter;
}
