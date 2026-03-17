#include "ImgReorderListModel.h"
#include "RenameHelper.h"
#include "Logger.h"
#include "PublicVariable.h"
#include "PathTool.h"
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
  m_imgs.swap(new_imgs);
  m_baseName = baseName;
  m_startNo = startIndex;
  m_namePattern = namePattern;
  endResetModel();
  return true;
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
  bool bOk{false};
  const int newNumber{value.toInt(&bOk)};
  if (!bOk) {
    LOG_D("invalid number");
    return false;
  }
  if (newNumber == beforeNumber) {
    LOG_D("number[%d] unchanged", beforeNumber);
    return false;
  }
  if (0 <= newNumber && newNumber < rowCount()) {
    LOG_D("number[%d] in occupied range[0, %d)", newNumber, rowCount());
    return false;
  }
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
  int targetRow = row; // 列表项之间
  if (targetRow == -1) {
    if (parent.isValid()) { // 在项上, 直接取项的索引
      targetRow = parent.row();
    } else {
      targetRow = m_imgs.size(); // 放到末尾
    }
  }

  std::pair<bool, ImgReorderDataLst> moveResult = MoveItemsBase<ImgReorderDataLst>(m_imgs, ascSelectedRows, targetRow);
  if (!moveResult.first) {
    return false; // no need move
  }
  beginResetModel();
  m_imgs.swap(moveResult.second);
  endResetModel();
  return true;
}

Qt::DropActions ImgReorderListModel::supportedDropActions() const {
  return Qt::MoveAction;
}
