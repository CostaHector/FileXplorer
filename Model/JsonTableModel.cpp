#include "JsonTableModel.h"
#include "NameTool.h"
#include "StudiosManager.h"
#include "PublicVariable.h"
#include "DataFormatter.h"
#include "CastManager.h"
#include "PathTool.h"
#include <QBrush>
#include <QDir>
#include <QDirIterator>
#include <QTextCharFormat>

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
  const int col = index.column();
  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    switch (col) {
#define JSON_KEY_ITEM(enu, val, def, enhanceDef, format, writer, initer, jsonWriter) \
  case enu:                                                                          \
    return format(item.m_##enu);  //
      JSON_MODEL_FIELD_MAPPING    //
#undef JSON_KEY_ITEM              //
          default : return {};
    }
  } else if (role == Qt::ForegroundRole) {
    switch (col) {
      case JSON_KEY_E::Cast: {
        if (!item.hintCast.isEmpty()) {
          return QColor{Qt::GlobalColor::red};
        }
        break;
      }
      case JSON_KEY_E::Studio: {
        if (!item.hintStudio.isEmpty()) {
          return QColor{Qt::GlobalColor::red};
        }
        break;
      }
      default:
        break;
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
      if (orientation == Qt::Vertical && m_modifiedRows.test(section)) {
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
#define JSON_KEY_ITEM(enu, val, def, enhanceDef, format, writer, initer, jsonWriter) \
  case enu: {                                                                        \
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
    if (index.column() != JsonKey::Detail) {
      emit dataChanged(index, index, {Qt::DisplayRole});
    }
  }
  return QAbstractItemModel::setData(index, value, role);
}

int JsonTableModel::setRootPath(const QString& path, bool isForce) {
  if (mRootPath == path && !isForce) {
    qDebug("Path[%s] unchange", qPrintable(path));
    return 0;
  }

  mRootPath = path;
  if (!QFileInfo(path).isDir()) {
    qDebug("path[%s] is not a dir", qPrintable(path));
    return -1;
  }

  decltype(mCachedJsons) tempCachedJsons;
  QDirIterator it{path, TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  while (it.hasNext()) {
    it.next();
    tempCachedJsons.append(JsonPr::fromJsonFile(it.filePath()));
  }

  const int befRowCnt = mCachedJsons.size();
  const int afterRowCnt = tempCachedJsons.size();
  RowsCountBeginChange(befRowCnt, afterRowCnt);
  mCachedJsons.swap(tempCachedJsons);
  m_modifiedRows.reset();
  RowsCountEndChange();
  return afterRowCnt;
}

int JsonTableModel::forceReloadPath() {
  return setRootPath(mRootPath, true);
}

QFileInfo JsonTableModel::fileInfo(const QModelIndex& index) const {
  int row = index.row();
  if (row < 0 || row >= rowCount()) {
    qWarning("row: %d out of range", row);
    return {};
  }
  return QFileInfo{mCachedJsons[row].GetAbsPath()};
}

QString JsonTableModel::filePath(const QModelIndex& index) const {
  int row = index.row();
  if (row < 0 || row >= rowCount()) {
    qWarning("row: %d out of range", row);
    return {};
  }
  return mCachedJsons[row].GetAbsPath();
}

QString JsonTableModel::fileName(const QModelIndex& index) const {
  int row = index.row();
  if (row < 0 || row >= rowCount()) {
    qWarning("row: %d out of range", row);
    return {};
  }
  return mCachedJsons[row].jsonFileName;
}

QString JsonTableModel::fileBaseName(const QModelIndex& index) const {
  return PathTool::GetBaseName(fileName(index));
}

QString JsonTableModel::absolutePath(const QModelIndex &index) const {
  int row = index.row();
  if (row < 0 || row >= rowCount()) {
    qWarning("row: %d out of range", row);
    return {};
  }
  return mCachedJsons[row].m_Prepath;
}

QString JsonTableModel::fullInfo(const QModelIndex &index) const {
  int row = index.row();
  if (row < 0 || row >= rowCount()) {
    qWarning("row: %d out of range", row);
    return {};
  }
  return mCachedJsons[row].GetJsonBA();
}

void JsonTableModel::setModified(int row, bool modified) {
  setModifiedNoEmit(row, modified);
  emit headerDataChanged(Qt::Vertical, row, row);
}

void JsonTableModel::setModifiedNoEmit(int row, bool modified) {
  m_modifiedRows.set(row, modified);
}

int JsonTableModel::SetStudio(const QModelIndexList& rowIndexes, const QString& studio) {
  int cnt{0};
  int row{-1};
  int minRow{INT_MAX}, maxRow{-1};
  for (const QModelIndex& ind : rowIndexes) {
    row = ind.row();
    if (row < 0 || row >= rowCount()) {
      qWarning("row: %d out of range [0,%d)", row, rowCount());
      return cnt;
    }
    if (mCachedJsons[row].m_Studio == studio) {
      continue;
    }
    ++cnt;
    mCachedJsons[row].m_Studio = studio;
    setModifiedNoEmit(row);
    if (row > maxRow) {
      maxRow = row;
    }
    if (row < minRow) {
      minRow = row;
    }
  }
  if (maxRow < 0 || minRow > maxRow) {
    qWarning("Studio Field of %d row(s) NO change at all", rowIndexes.size());
    return 0;
  }
  const QModelIndex& frontInd = sibling(minRow, JSON_KEY_E::Studio, {});
  const QModelIndex& backInd = sibling(maxRow, JSON_KEY_E::Studio, {});
  emit dataChanged(frontInd, backInd, {Qt::DisplayRole});
  emit headerDataChanged(Qt::Vertical, minRow, maxRow);
  qDebug("Studio Field of %d/%d row(s) range [%d, %d) changed to [%s]", cnt, rowIndexes.size(), minRow, maxRow, qPrintable(studio));
  return cnt;
}

int JsonTableModel::SetCastOrTags(const QModelIndexList& rowIndexes, JSON_KEY_E keyEnum, const QString& sentence) {
  if (keyEnum != JSON_KEY_E::Cast && keyEnum != JSON_KEY_E::Tags) {
    qWarning("Field[%d] not support", (int)keyEnum);
    return -1;
  }

  const SortedUniqStrLst newLst{sentence};
  int cnt{0};
  int row{-1};
  int minRow{INT_MAX}, maxRow{-1};
  for (const QModelIndex& ind : rowIndexes) {
    row = ind.row();
    if (row < 0 || row >= rowCount()) {
      qWarning("row: %d out of range [0,%d)", row, rowCount());
      return cnt;
    }
    auto& targetField = (keyEnum == JSON_KEY_E::Cast) ? mCachedJsons[row].m_Cast : mCachedJsons[row].m_Tags;
    if (targetField == newLst) {
      continue;
    }
    targetField = newLst;
    setModifiedNoEmit(row);
    if (row > maxRow) {
      maxRow = row;
    }
    if (row < minRow) {
      minRow = row;
    }
    ++cnt;
  }
  if (maxRow < 0 || minRow > maxRow) {
    qWarning("Cast or Tags Field[%d] of %d row(s) NO change at all", keyEnum, rowIndexes.size());
    return 0;
  }
  const QModelIndex& frontInd = sibling(minRow, keyEnum, {});
  const QModelIndex& backInd = sibling(maxRow, keyEnum, {});
  emit dataChanged(frontInd, backInd, {Qt::DisplayRole});
  emit headerDataChanged(Qt::Vertical, minRow, maxRow);
  qDebug("Cast or Tags Field[%d] of %d/%d row(s) range [%d, %d) set [%s]", keyEnum, cnt, rowIndexes.size(), minRow, maxRow, qPrintable(sentence));
  return cnt;
}

int JsonTableModel::AddCastOrTags(const QModelIndexList& rowIndexes, const JSON_KEY_E keyEnum, const QString& sentence) {
  if (sentence.isEmpty()) {
    qDebug("No need add empty to cast or tags field[%d]", keyEnum);
    return 0;
  }
  if (keyEnum != JSON_KEY_E::Cast && keyEnum != JSON_KEY_E::Tags) {
    qWarning("Field[%d] not support", (int)keyEnum);
    return -1;
  }

  const SortedUniqStrLst appendContainer{sentence};

  int cnt{0};
  int row{-1};
  int minRow{INT_MAX}, maxRow{-1};
  for (const QModelIndex& ind : rowIndexes) {
    row = ind.row();
    if (row < 0 || row >= rowCount()) {
      qWarning("row: %d out of range [0,%d)", row, rowCount());
      return cnt;
    }
    auto& targetField = (keyEnum == JSON_KEY_E::Cast) ? mCachedJsons[row].m_Cast : mCachedJsons[row].m_Tags;
    if (targetField == appendContainer) {
      continue;
    }
    targetField += appendContainer;
    setModifiedNoEmit(row);
    if (row > maxRow) {
      maxRow = row;
    }
    if (row < minRow) {
      minRow = row;
    }
    ++cnt;
  }
  if (maxRow < 0 || minRow > maxRow) {
    qWarning("Cast or Tags Field[%d] of %d row(s) NO change at all", keyEnum, rowIndexes.size());
    return 0;
  }
  const QModelIndex& frontInd = sibling(minRow, keyEnum, {});
  const QModelIndex& backInd = sibling(maxRow, keyEnum, {});
  emit dataChanged(frontInd, backInd, {Qt::DisplayRole});
  emit headerDataChanged(Qt::Vertical, minRow, maxRow);
  qDebug("Cast or Tags Field[%d] of %d/%d row(s) range [%d, %d) Add [%s]", keyEnum, cnt, rowIndexes.size(), minRow, maxRow, qPrintable(sentence));
  return cnt;
}

int JsonTableModel::RmvCastOrTags(const QModelIndexList& rowIndexes, const JSON_KEY_E keyEnum, const QString& oneElement) {
  if (oneElement.isEmpty()) {
    qDebug("No need remove empty from cast or tags field[%d]", keyEnum);
    return 0;
  }

  if (keyEnum != JSON_KEY_E::Cast && keyEnum != JSON_KEY_E::Tags) {
    qWarning("Field[%d] not support", (int)keyEnum);
    return -1;
  }

  int cnt{0};
  int row{-1};
  int minRow{INT_MAX}, maxRow{-1};
  for (const QModelIndex& ind : rowIndexes) {
    row = ind.row();
    if (row < 0 || row >= rowCount()) {
      qWarning("row: %d out of range [0,%d)", row, rowCount());
      return cnt;
    }
    auto& targetField = (keyEnum == JSON_KEY_E::Cast) ? mCachedJsons[row].m_Cast : mCachedJsons[row].m_Tags;
    if (!targetField.remove(oneElement)) {
      continue;
    }
    setModifiedNoEmit(row);
    ++cnt;
    if (row > maxRow) {
      maxRow = row;
    }
    if (row < minRow) {
      minRow = row;
    }
  }
  if (maxRow < 0 || minRow > maxRow) {
    qWarning("Cast or Tags Field[%d] of %d row(s) NO change at all", keyEnum, rowIndexes.size());
    return 0;
  }
  const QModelIndex& frontInd = sibling(minRow, keyEnum, {});
  const QModelIndex& backInd = sibling(maxRow, keyEnum, {});
  emit dataChanged(frontInd, backInd, {Qt::DisplayRole});
  emit headerDataChanged(Qt::Vertical, minRow, maxRow);
  qDebug("Cast or Tags Field[%d] of %d/%d row(s) range [%d, %d) remove element[%s]", keyEnum, cnt, rowIndexes.size(), minRow, maxRow, qPrintable(oneElement));
  return cnt;
}

int JsonTableModel::InitCastAndStudio(const QModelIndexList& rowIndexes) {
  int cnt{0};
  int row{-1};
  int minRow{INT_MAX}, maxRow{-1};

  for (const QModelIndex& ind : rowIndexes) {
    row = ind.row();
    if (row < 0 || row >= rowCount()) {
      qWarning("row: %d out of range [0,%d)", row, rowCount());
      return cnt;
    }
    if (!mCachedJsons[row].ConstructCastStudioValue()) {
      continue;
    }

    setModifiedNoEmit(row);
    ++cnt;
    if (row > maxRow) {
      maxRow = row;
    }
    if (row < minRow) {
      minRow = row;
    }
  }
  if (maxRow < 0 || minRow > maxRow) {
    qWarning("Cast and Tags Field of %d row(s) NO init at all", rowIndexes.size());
    return 0;
  }
  const QModelIndex& castFrontInd = sibling(minRow, JSON_KEY_E::Cast, {});
  const QModelIndex& castBackInd = sibling(maxRow, JSON_KEY_E::Cast, {});
  emit dataChanged(castFrontInd, castBackInd, {Qt::DisplayRole});
  emit headerDataChanged(Qt::Vertical, minRow, maxRow);

  const QModelIndex& studioFrontInd = sibling(minRow, JSON_KEY_E::Studio, {});
  const QModelIndex& studioBackInd = sibling(maxRow, JSON_KEY_E::Studio, {});
  emit dataChanged(studioFrontInd, studioBackInd, {Qt::DisplayRole});
  emit headerDataChanged(Qt::Vertical, minRow, maxRow);
  qDebug("Cast and Tags Field of %d/%d row(s) range [%d, %d) init ok", cnt, rowIndexes.size(), minRow, maxRow);
  return cnt;
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
      qWarning("row: %d out of range [0,%d)", row, rowCount());
      return studioCnt;
    }
    auto& item = mCachedJsons[row];
    item.HintForCastStudio(sentence, studioChanged, castChanged);
    if (!studioChanged && !castChanged) {
      continue;
    }
    // hintCast or hintStudio changed
    setModifiedNoEmit(row);
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
    qWarning("Studio Field of %d row(s) NO hint at all", rowIndexes.size());
  } else {
    const QModelIndex& studioFrontInd = sibling(studioMinRow, JSON_KEY_E::Studio, {});
    const QModelIndex& studioBackInd = sibling(studioMaxRow, JSON_KEY_E::Studio, {});
    emit dataChanged(studioFrontInd, studioBackInd, {Qt::DisplayRole});
    emit headerDataChanged(Qt::Vertical, studioMinRow, studioMaxRow);
    qDebug("Studio Field of %d/%d row(s) range [%d, %d) hint ok", studioCnt, rowIndexes.size(), studioMinRow, studioMaxRow);
  }

  if (castMaxRow < 0 || castMinRow > castMaxRow) {
    qWarning("Cast Field of %d row(s) NO hint at all", rowIndexes.size());
  } else {
    const QModelIndex& castFrontInd = sibling(castMinRow, JSON_KEY_E::Cast, {});
    const QModelIndex& castBackInd = sibling(castMaxRow, JSON_KEY_E::Cast, {});
    emit dataChanged(castFrontInd, castBackInd, {Qt::ForegroundRole | Qt::DisplayRole});
    emit headerDataChanged(Qt::Vertical, castMinRow, castMaxRow);
    qDebug("Cast Field of %d/%d row(s) range [%d, %d) hint ok", castCnt, rowIndexes.size(), castMinRow, castMaxRow);
  }

  return studioCnt + castCnt;
}

int JsonTableModel::FormatCast(const QModelIndexList& rowIndexes) {
  int cnt{0};
  int row{-1};
  int minRow{INT_MAX}, maxRow{-1};
  for (const QModelIndex& ind : rowIndexes) {
    row = ind.row();
    if (row < 0 || row >= rowCount()) {
      qWarning("row: %d out of range [0,%d)", row, rowCount());
      return cnt;
    }
    mCachedJsons[row].m_Cast.format();
    setModifiedNoEmit(row);
    if (row > maxRow) {
      maxRow = row;
    }
    if (row < minRow) {
      minRow = row;
    }
    ++cnt;
  }
  if (maxRow < 0 || minRow > maxRow) {
    qWarning("Cast Field of %d row(s) NO format at all", rowIndexes.size());
    return 0;
  }
  const QModelIndex& frontInd = sibling(minRow, JSON_KEY_E::Cast, {});
  const QModelIndex& backInd = sibling(maxRow, JSON_KEY_E::Cast, {});
  emit dataChanged(frontInd, backInd, {Qt::DisplayRole});
  emit headerDataChanged(Qt::Vertical, minRow, maxRow);
  qDebug("Cast Field of %d/%d row(s) range [%d, %d) format ok", cnt, rowIndexes.size(), minRow, maxRow);
  return cnt;
}

int JsonTableModel::SyncFieldNameByJsonBaseName(const QModelIndexList& rowIndexes) {
  int cnt{0};
  int row{-1};
  int minRow{INT_MAX}, maxRow{-1};
  for (const QModelIndex& ind : rowIndexes) {
    row = ind.row();
    if (row < 0 || row >= rowCount()) {
      qWarning("row: %d out of range [0,%d)", row, rowCount());
      return cnt;
    }
    if (!mCachedJsons[row].SyncNameValueFromFileBaseName()) {
      continue;
    }
    setModifiedNoEmit(row);
    if (row > maxRow) {
      maxRow = row;
    }
    if (row < minRow) {
      minRow = row;
    }
    ++cnt;
  }
  if (maxRow < 0 || minRow > maxRow) {
    qWarning("Name Field of %d row(s) NO sync at all", rowIndexes.size());
    return 0;
  }
  const QModelIndex& frontInd = sibling(minRow, JSON_KEY_E::Name, {});
  const QModelIndex& backInd = sibling(maxRow, JSON_KEY_E::Name, {});
  emit dataChanged(frontInd, backInd, {Qt::DisplayRole});
  emit headerDataChanged(Qt::Vertical, minRow, maxRow);
  qDebug("Name Field of %d/%d row(s) range [%d, %d) sync ok", cnt, rowIndexes.size(), minRow, maxRow);
  return cnt;
}

int JsonTableModel::RenameJsonAndItsRelated(const QModelIndex& ind, const QString& newJsonBaseName) {
  if (newJsonBaseName.isEmpty()) {
    qWarning("new json basename cannot be empty");
    return false;
  }
  int row = ind.row();
  if (row < 0 || row >= rowCount()) {
    qWarning("row: %d out of range [0,%d)", row, rowCount());
    return 0;
  }
  auto cnt = mCachedJsons[row].RenameJsonAndRelated(newJsonBaseName, true);
  // no need send data change signal, because rename json will not change Name Field automatically
  return cnt;
}

int JsonTableModel::SaveCurrentChanges(const QModelIndexList& rowIndexes) {
  int cnt{0};
  int row{-1};
  int minRow{INT_MAX}, maxRow{-1};
  for (const QModelIndex& ind : rowIndexes) {
    row = ind.row();
    if (row < 0 || row >= rowCount()) {
      qWarning("row: %d out of range [0,%d)", row, rowCount());
      return cnt;
    }
    if (!m_modifiedRows.test(row)) {
      continue;
    }
    if (!mCachedJsons[row].WriteIntoFiles()) {
      qWarning("Write into local file[%s] failed", qPrintable(mCachedJsons[row].GetAbsPath()));
      return -1;
    }
    setModifiedNoEmit(row, false);
    if (row > maxRow) {
      maxRow = row;
    }
    if (row < minRow) {
      minRow = row;
    }
    ++cnt;
  }
  if (maxRow < 0 || minRow > maxRow) {
    qWarning("Name Field of %d row(s) NO sync at all", rowIndexes.size());
    return 0;
  }
  const QModelIndex& frontInd = sibling(minRow, JSON_KEY_E::Cast, {});
  const QModelIndex& backInd = sibling(maxRow, JSON_KEY_E::Tags, {});
  emit dataChanged(frontInd, backInd, {Qt::DisplayRole | Qt::ForegroundRole});
  emit headerDataChanged(Qt::Vertical, minRow, maxRow);
  qDebug("Changes of %d/%d row(s) range [%d, %d) saved ok", cnt, rowIndexes.size(), minRow, maxRow);
  return cnt;
}

std::pair<int, int> JsonTableModel::ExportCastStudioToLocalDictionaryFile(const QModelIndexList& rowIndexes) const {
  int row{-1};
  int cnt{0};
  static CastManager& pm = CastManager::getIns();
  decltype(pm.m_casts) castIncrements;

  static StudiosManager& psm = StudiosManager::getIns();
  decltype(psm.m_prodStudioMap) studioIncrements;

  for (const QModelIndex& ind : rowIndexes) {
    row = ind.row();
    if (row < 0 || row >= rowCount()) {
      qWarning("row: %d out of range [0,%d)", row, rowCount());
      return {-1, -1};
    }
    pm.CastIncrement(castIncrements, mCachedJsons[row].m_Cast.toLowerSets());
    psm.StudioIncrement(studioIncrements, mCachedJsons[row].m_Studio);
    ++cnt;
  }
  qDebug("Increment of Cast:%d, Studio:%d from %d row(s)", castIncrements.size(), studioIncrements.size(), cnt);
  if (pm.WriteIntoLocalDictionaryFiles(castIncrements) < 0) {
    qWarning("Write %d Cast into local file failed", castIncrements.size());
    return {-1, studioIncrements.size()};
  }
  if (psm.WriteIntoLocalDictionaryFiles(studioIncrements) < 0) {
    qWarning("Write %d Studio into local file failed", studioIncrements.size());
    return {castIncrements.size(), -1};
  }
  return {castIncrements.size(), studioIncrements.size()};
}

int JsonTableModel::AppendCastFromSentence(const QModelIndex& ind, const QString& sentence, bool isUpperCaseSentence) {
  int row = ind.row();
  if (row < 0 || row >= rowCount()) {
    qWarning("row: %d out of range [0,%d)", row, rowCount());
    return -1;
  }
  static const NameTool nt;
  const QStringList& newLst{isUpperCaseSentence ? nt.castFromUpperCaseSentence(sentence) : nt.castFromSentence(sentence)};

  auto& stCast = mCachedJsons[row].m_Cast;
  int beforeCastCnt = stCast.count();
  stCast.insertBatch(newLst);
  int afterCastCnt = stCast.count();
  if (afterCastCnt <= beforeCastCnt) {
    qDebug("nothing cast increased by selected sentence[%s]", qPrintable(sentence));
    return 0;
  }

  setModifiedNoEmit(row, true);
  const QModelIndex& frontAndBackInd = ind.siblingAtColumn(JSON_KEY_E::Cast);
  emit dataChanged(frontAndBackInd, frontAndBackInd, {Qt::DisplayRole | Qt::ForegroundRole});
  emit headerDataChanged(Qt::Vertical, row, row);
  qDebug("%d cast increased by selected sentence[%s]", afterCastCnt - beforeCastCnt, qPrintable(sentence));
  return afterCastCnt - beforeCastCnt;
}
