#include "ImgReorderListModel.h"
#include "RenameHelper.h"
#include "Logger.h"
#include "PublicVariable.h"
#include "PathTool.h"
#include "FileTool.h"
#include <QMimeData>

constexpr const char* ImgReorderListModel::MIME_TYPE;

bool ImgReorderListModel::setImagesToReorder(const QStringList& imgs, const QString& baseName, int startIndex, const QString& namePattern) {
  if (!namePattern.contains("%1")) {
    LOG_E("namePattern[%s] invalid, no %%1 find", qPrintable(namePattern));
    return false;
  }
  beginResetModel();

  ImgReorderDataLst new_imgs;
  new_imgs.reserve(imgs.size());
  for (int i = 0; i < imgs.size(); ++i) {
    new_imgs.push_back(ImgReorderDataType{imgs[i], i});
  }
  initOccupiedRows(imgs.size());
  m_imgs.swap(new_imgs);
  m_baseName = baseName;
  m_startNo = startIndex;
  m_namePattern = namePattern;
  endResetModel();
  return true;
}

QString ImgReorderListModel::filePath(const QModelIndex& ind) const {
  if (!ind.isValid()) {
    return {};
  }
  const int row = ind.row();
  return m_imgs[row].fullPath;
}

QVariant ImgReorderListModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return {};
  }
  int row = index.row();
  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    return m_imgs[row].number;
  } else if (role == Qt::DecorationRole) {
    return GetDecorationPixmap(m_imgs[row].fullPath);
  }
  return {};
}

bool ImgReorderListModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (!index.isValid()) {
    return false;
  }
  if (role != Qt::EditRole) {
    return false;
  }
  const int beforeNumber = m_imgs[index.row()].number;
  bool bIsInt{false};
  const int newNumber{value.toInt(&bIsInt)};
  if (!bIsInt) {
    return false;
  }
  if (newNumber == beforeNumber) {
    return false;
  }
  if (m_occupiedRows.contains(newNumber)) {
    LOG_D("number[%d] is occupied", newNumber);
    return false;
  }
  m_occupiedRows.remove(beforeNumber);
  m_occupiedRows.insert(newNumber);
  m_imgs[index.row()].number = newNumber;
  return true;
}

QStringList ImgReorderListModel::getOrderedNames() const {
  const int N{m_imgs.size()};
  if (N == 0) {
    return {};
  }
  const int maxVal = m_startNo + N - 1;
  const int fieldWidth{RenameHelper::GetDigitsCount(maxVal)};
  QStringList newNames;
  newNames.reserve(N);
  for (const ImgReorderDataType& imgsInfo : m_imgs) {
    newNames.push_back(RenameHelper::GetNameWithPatternIndex(m_baseName, m_namePattern, imgsInfo.number + m_startNo, fieldWidth));
  }
  return newNames;
}

QMimeData* ImgReorderListModel::mimeData(const QModelIndexList& indexes) const {
  if (indexes.isEmpty()) {
    return nullptr;
  }
  // 存储所有选中的行索引
  QList<int> rowsInAscending;
  rowsInAscending.reserve(indexes.size());
  for (const QModelIndex& index : indexes) {
    if (index.isValid()) {
      rowsInAscending.append(index.row());
    }
  }

  if (rowsInAscending.isEmpty()) {
    return nullptr;
  }

  QByteArray encodedData;
  QDataStream stream(&encodedData, QIODevice::WriteOnly);
  std::sort(rowsInAscending.begin(), rowsInAscending.end());
  stream << rowsInAscending;

  QMimeData* mimeData = new QMimeData();
  mimeData->setData(MIME_TYPE, encodedData);
  return mimeData;
}

bool ImgReorderListModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
  if (data == nullptr || !data->hasFormat(MIME_TYPE)) {
    return false;
  }
  if (action == Qt::IgnoreAction) {
    return false;
  }

  // 解码选中的行
  QByteArray encodedData = data->data(MIME_TYPE);
  QDataStream stream(&encodedData, QIODevice::ReadOnly);
  QList<int> ascSelectedRows;
  stream >> ascSelectedRows;

  if (ascSelectedRows.isEmpty()) {
    return false;
  }

  // 计算目标行
  int targetRow = row;  // 列表项之间
  if (targetRow == -1) {
    if (parent.isValid()) {  // 在项上, 直接取项的索引
      targetRow = parent.row();
    } else {
      targetRow = m_imgs.size();  // 放到末尾
    }
  }

  std::pair<bool, ImgReorderDataLst> moveResult = MoveItemsBase<ImgReorderDataLst>(m_imgs, ascSelectedRows, targetRow);
  if (!moveResult.first) {
    return false;  // no need move
  }
  beginResetModel();
  m_imgs.swap(moveResult.second);
  endResetModel();
  return true;
}

Qt::DropActions ImgReorderListModel::supportedDropActions() const {
  return Qt::MoveAction;
}

bool ImgReorderListModel::onBatchShiftSelectedRowsByStep(const QModelIndexList& indexes, int step) {
  if (indexes.isEmpty() || step == 0) {
    return false;
  }
  QList<int> selectedRows;
  selectedRows.reserve(indexes.size());
  for (const QModelIndex& ind : indexes) {
    selectedRows.push_back(ind.row());
  }
  bool bNeedShift{false};
  ImgReorderDataLst newImages;
  std::tie(bNeedShift, newImages) = BatchShiftSelectedRowsByStep(m_imgs, selectedRows, step);
  if (!bNeedShift) {
    return false;
  }

  beginResetModel();
  m_imgs.swap(newImages);
  endResetModel();

  updateOccupiedRows();
  return true;
}

bool ImgReorderListModel::onNormalizeKeepRelativeOrder() {
  if (m_imgs.isEmpty()) {
    return false;
  }

  bool bNeedNormalize{false};
  ImgReorderDataLst newImages;
  std::tie(bNeedNormalize, newImages) = NormalizeKeepRelativeOrder(m_imgs);
  if (!bNeedNormalize) {
    return false;
  }

  beginResetModel();
  m_imgs.swap(newImages);
  endResetModel();

  initOccupiedRows(m_imgs.size());
  return true;
}

bool ImgReorderListModel::onOpenFileInSystemApplication(const QModelIndex& ind) {
  if (!ind.isValid()) {
    return false;
  }
  return FileTool::OpenLocalFileUsingDesktopService(filePath(ind));
}

void ImgReorderListModel::initOccupiedRows(int n) const {
  m_occupiedRows.clear();
  for (int i = 0; i < n; ++i) {
    m_occupiedRows.insert(i);
  }
}

void ImgReorderListModel::updateOccupiedRows() const {
  m_occupiedRows.clear();
  for (const ImgReorderDataType& ele : m_imgs) {
    m_occupiedRows.insert(ele.number);
  }
}

std::pair<bool, ImgReorderDataLst> BatchShiftSelectedRowsByStep(const ImgReorderDataLst& datas, const QList<int>& selectedRows, int step) {
  // 0. precondition: datas is strictly ascending and unqiue. selectedRows must in range of datas
  // 1. 检查边界条件
  if (step == 0 || datas.isEmpty() || selectedRows.isEmpty()) {
    return {false, {}};
  }
  // 2. 创建datas的副本并修改选中行
  ImgReorderDataLst newDatas = datas;
  for (int index : selectedRows) {
    newDatas[index] += step;
  }
  // 3. 检查是否冲突
  std::sort(newDatas.begin(), newDatas.end());
  // std::unique(newDatas.cbegin(), newDatas.cend()) != newDatas.cend() will move duplicates(if exists) to after the return iterator
  // while adjacent_find will not, aka for (auto it = newDatas.begin(); it < newDatas.end() - 1; ++it) if (*it == *(it + 1)) return false;
  if (std::adjacent_find(newDatas.cbegin(), newDatas.cend()) != newDatas.cend()) {
    return {false, {}};
  }
  return {true, newDatas};
}

std::pair<bool, ImgReorderDataLst> NormalizeKeepRelativeOrder(const ImgReorderDataLst& datas) {
  // precondition: datas are sorted in number ascending and unique
  if (datas.isEmpty()) {
    return {false, {}};
  }
  if (datas.front().number == 0 && datas.back().number == datas.size() - 1) {
    // Already Normalized
    return {false, {}};
  }

  QHash<int, int> valueToRank;
  for (int i = 0; i < datas.size(); ++i) {
    valueToRank[datas[i].number] = i;
  }

  ImgReorderDataLst newDatas(datas);
  for (int i = 0; i < datas.size(); ++i) {
    newDatas[i].number = valueToRank[newDatas[i].number];
  }
  return {true, newDatas};
}
