#include "QAbstractListModelPub.h"
#include "Logger.h"
#include "StringTool.h"
#include "PublicVariable.h"
#include "ImageTool.h"
#include "MemoryKey.h"
#include <QFile>

QAbstractListModelPub::QAbstractListModelPub(const QString& listViewName, QObject* parent)  //
    : QAbstractListModel{parent}, mMemoryName{listViewName} {
  const int scaledIndex{IMAGE_SIZE::GetInitialScaledSize(GetName())};
  setPixmapWidth(IMAGE_SIZE::ICON_SIZE_CANDIDATES[scaledIndex].width());
  setPixmapHeight(IMAGE_SIZE::ICON_SIZE_CANDIDATES[scaledIndex].height());

  m_isSmooth = Configuration().value(GetName() + "_PIXMAP_TRANSFORMATION_SMOOTH", false).toBool();
  _PIXMAP_TRANSFORMATION_SMOOTH = new (std::nothrow) QAction{QIcon{":img/IMAGE_TRANSFORMATION_SMOOTH"}, tr("smooth transformation"), this};
  _PIXMAP_TRANSFORMATION_SMOOTH->setCheckable(true);
  _PIXMAP_TRANSFORMATION_SMOOTH->setChecked(m_isSmooth);
  _PIXMAP_TRANSFORMATION_SMOOTH->setToolTip(
      "QPixmap resize is transformed using bilinear filtering(smooth) when enabled, by default(false): no smooth");

  subscribe();
}

QAbstractListModelPub::~QAbstractListModelPub() {
  Configuration().setValue(GetName() + "_PIXMAP_TRANSFORMATION_SMOOTH", _PIXMAP_TRANSFORMATION_SMOOTH->isChecked());
}

void QAbstractListModelPub::subscribe() {
  connect(_PIXMAP_TRANSFORMATION_SMOOTH, &QAction::toggled, this, &QAbstractListModelPub::onPixmapSmoothTransformationToggled);
}

QList<QAction*> QAbstractListModelPub::GetExcusiveActions() const {
  return {_PIXMAP_TRANSFORMATION_SMOOTH};
}

bool QAbstractListModelPub::RowsCountBeginChange(int beforeRow, int afterRow) {
  if (!IsDimensionCntValid(beforeRow, afterRow)) {
    LOG_W("row count[bef:%d, aft:%d] invalid", beforeRow, afterRow);
    return false;
  }
  mRowChangeStack.emplace(beforeRow, afterRow);
  if (beforeRow == afterRow) {
    return true;
  } else if (beforeRow < afterRow) {
    beginInsertRows(QModelIndex(), beforeRow, afterRow - 1);
  } else {
    beginRemoveRows(QModelIndex(), afterRow, beforeRow - 1);
  }
  return true;
}

bool QAbstractListModelPub::RowsCountEndChange() {
  if (mRowChangeStack.empty()) {
    LOG_W("No row count change at all!");
    return false;
  }
  DimensionRange rng = mRowChangeStack.top();
  mRowChangeStack.pop();
  if (rng.m_bef == rng.m_aft) {
    if (rng.m_aft > 0) {
      emit dataChanged(index(0, 0), index(rng.m_aft - 1, 0), {Qt::ItemDataRole::DisplayRole, Qt::ItemDataRole::DecorationRole});
    }
  } else if (rng.m_bef < rng.m_aft) {
    endInsertRows();
  } else {
    endRemoveRows();
  }
  return true;
}

int QAbstractListModelPub::onRowsRangeListRemoved(const QModelIndexList& indexes, ModelTools::FuncRangeListRemoveCallback fCallback) {
  if (indexes.isEmpty()) {
    LOG_D("Indexes list empty. skip");
    return 0;
  }
  // 获取有效行(按照行号升序, 且无重复元素)
  QList<int> validRows = ModelTools::GetIndexesRows(indexes);
  // 合并行号到区间(保持升序, 例如输入{0,1,3,4,5} -> 输出{{0,1}, {3,5}})
  QList<std::pair<int, int>> ranges = ModelTools::MergeList2SectionsRange(validRows);
  // 倒序删行区间
  if (fCallback) {
    beginResetModel();
    fCallback(ranges);
    endResetModel();
  }
  return validRows.size();
}

int QAbstractListModelPub::onRowsRemoved(const QModelIndexList& indexes, ModelTools::FuncRangeRemoveCallback fCallback) {
  if (indexes.isEmpty()) {
    LOG_D("Indexes list empty. skip");
    return 0;
  }
  // 获取有效行(按照行号升序, 且无重复元素)
  QList<int> validRows = ModelTools::GetIndexesRows(indexes);
  // 合并行号到区间(保持升序, 例如输入{0,1,3,4,5} -> 输出{{0,1}, {3,5}})
  QList<std::pair<int, int>> ranges = ModelTools::MergeList2SectionsRange(validRows);
  // 倒序删行区间
  {
    beginResetModel();
    for (auto it = ranges.rbegin(); it != ranges.rend(); ++it) {
      int frontRow = it->first;
      int backRow = it->second;
      if (fCallback) {
        // beginRemoveRows({}, frontRow, backRow);
        fCallback(frontRow, backRow + 1);
        // endRemoveRows();
      }
    }
    endResetModel();
  }
  return validRows.size();
}

QPixmap QAbstractListModelPub::GetDecorationPixmap(const QString& fileAbsPath) const {
  return ImageTool::GetPixmapFromCached(fileAbsPath, getPixmapWidth(), getPixmapHeight(), isPixmapTransformationSmooth());
}

bool QAbstractListModelPub::onIconSizeChange(const QSize& newSize) {
  if (getPixmapWidth() == newSize.width() && getPixmapHeight() == newSize.height()) {
    return false;
  }
  setPixmapWidth(newSize.width());
  setPixmapHeight(newSize.height());
  if (rowCount() != 0) {
    emit dataChanged(index(0), index(rowCount() - 1), {Qt::ItemDataRole::DecorationRole});
  }
  return true;
}

#include <QDebug>
bool QAbstractListModelPub::onPixmapSmoothTransformationToggled(bool newSmooth) {
  if (isPixmapTransformationSmooth() == newSmooth) {
    return false;
  }
  m_isSmooth = newSmooth;
  if (rowCount() != 0) {
    connect(this, &QAbstractListModelPub::dataChanged, [](const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles) {
      qWarning() << "dataChanged emitted with roles:" << roles << topLeft << bottomRight;
    });
    emit dataChanged(index(0), index(rowCount() - 1), {Qt::ItemDataRole::DecorationRole});
  }
  return true;
}

template <typename RandomAccessible1DimensionContainerDataType>
std::pair<bool, RandomAccessible1DimensionContainerDataType> MoveItemsBase(const RandomAccessible1DimensionContainerDataType& dataList,
                                                                           const QList<int>& ascUnqiueSelectRows,
                                                                           int destRow) {
  // precondition: ascUnqiueSelectRows must be unique and in ascending order

  const int N{dataList.size()};
  const int selectedCount{ascUnqiueSelectRows.size()};
  if (N == 0 || selectedCount == 0) {
    return {false, {}};
  }

  // continous selection rows and destination in it
  const bool isSelectRowsContinousAndDestBeFirstSelect{
      ascUnqiueSelectRows.back() - ascUnqiueSelectRows.front() + 1 == selectedCount      // continous
      &&                                                                                 //
      (ascUnqiueSelectRows.front() <= destRow && destRow <= ascUnqiueSelectRows.back())  //
  };
  if (isSelectRowsContinousAndDestBeFirstSelect) {
    return {false, {}};
  }

  // all given row selected
  if (selectedCount == N) {
    return {false, {}};
  }

  // 创建新列表
  RandomAccessible1DimensionContainerDataType newDataList;
  newDataList.reserve(N);

  // 第一部分：目标行之前的非选中元素
  int srcIndex = 0;
  int selectedIndex = 0;

  // 复制目标行之前的非选中元素
  while (srcIndex < destRow) {
    if (selectedIndex < selectedCount && srcIndex == ascUnqiueSelectRows[selectedIndex]) {
      // 跳过选中的元素
      ++selectedIndex;
    } else {
      // 复制非选中的元素
      newDataList.push_back(dataList[srcIndex]);
    }
    ++srcIndex;
  }

  // 第二部分：插入选中的元素（保持原来的顺序）
  for (int selectedRow : ascUnqiueSelectRows) {
    newDataList.push_back(dataList[selectedRow]);
  }

  // 第三部分：复制剩余的非选中元素
  while (srcIndex < N) {
    if (selectedIndex < selectedCount && srcIndex == ascUnqiueSelectRows[selectedIndex]) {
      // 跳过选中的元素
      ++selectedIndex;
    } else {
      // 复制非选中的元素
      newDataList.push_back(dataList[srcIndex]);
    }
    ++srcIndex;
  }

  return {true, std::move(newDataList)};
  // RandomAccessible1DimensionContainerDataType newDataList = dataList;
  // // 提取要移动的项目
  // RandomAccessible1DimensionContainerDataType itemsToMoveInReverse;
  // itemsToMoveInReverse.reserve(selectedCount);
  // for (auto it = ascUnqiueSelectRows.crbegin(); it != ascUnqiueSelectRows.crend(); ++it) {
  //   itemsToMoveInReverse.push_back(newDataList.takeAt(*it));
  // }

  // // 目标应当插入到的位置（targetRow前面的有些行也被takeAt了, 找出前面的行数, 移动这个数量）
  // const int itemsCntBeforeTarget{                                                                         //
  //                                std::count_if(ascUnqiueSelectRows.cbegin(), ascUnqiueSelectRows.cend(),  //
  //                                              [destRow](int ele) {                                       //
  //                                                return ele < destRow;
  //                                              })};
  // const int adjustedTargetRow{destRow - itemsCntBeforeTarget};

  // RandomAccessible1DimensionContainerDataType resultList;
  // resultList.reserve(N);
  // resultList += RandomAccessible1DimensionContainerDataType(newDataList.cbegin(), newDataList.cbegin() + adjustedTargetRow);  // before target
  // resultList += RandomAccessible1DimensionContainerDataType(itemsToMoveInReverse.crbegin(), itemsToMoveInReverse.crend());    // selections
  // resultList += RandomAccessible1DimensionContainerDataType(newDataList.cbegin() + adjustedTargetRow, newDataList.cend());    // after target
  // return {true, resultList};
}

#include "ImgReorderDataType.h"
template std::pair<bool, QList<int>> MoveItemsBase<QList<int>>(const QList<int>&, const QList<int>&, int);
template std::pair<bool, ImgReorderDataLst> MoveItemsBase<ImgReorderDataLst>(const ImgReorderDataLst&, const QList<int>&, int);
