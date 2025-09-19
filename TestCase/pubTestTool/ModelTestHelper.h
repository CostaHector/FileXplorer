#ifndef MODELTESTHELPER_H
#define MODELTESTHELPER_H
#include <QStandardItemModel>
#include <QTableView>
#include <QHeaderView>

namespace ModelTestHelper {

inline void InitStdItemModel(QStandardItemModel& model, const QString& cellContent, int rowM = 3, int colN = 2) {
  model.setRowCount(rowM);
  model.setColumnCount(colN);
  for (int row = 0; row < model.rowCount(); ++row) {
    for (int col = 0; col < model.columnCount(); ++col) {
      QModelIndex idx = model.index(row, col);
      model.setData(idx, cellContent.arg(row).arg(col));
    }
  }
}

}  // namespace ModelTestHelper

namespace HeaderRectHelper {
inline QRect GetHeaderRect(const QTableView& ctv, int section, Qt::Orientation headerOrientation) {
  if (section < 0) {
    return {};
  }
  switch (headerOrientation) {
    case Qt::Horizontal: {
      QHeaderView* horHeader = ctv.horizontalHeader();
      if (horHeader == nullptr) {
        return {};
      }
      if (section >= horHeader->count()) {  // out of bound
        return {};
      }
      return {horHeader->sectionViewportPosition(section), 0,  // (x, y===0)
              horHeader->sectionSize(section), horHeader->height()};
    }
    case Qt::Vertical: {
      QHeaderView* verHeader = ctv.verticalHeader();
      if (verHeader == nullptr) {
        return {};
      }
      if (section >= verHeader->count()) {  // out of bound
        return {};
      }
      return {0, verHeader->sectionViewportPosition(section),  // (x===0, y)
              verHeader->width(), verHeader->sectionSize(section)};
    }
    default: {
      Q_UNREACHABLE();
      // should never reach here
      return {};
    }

  }
}

}  // namespace HeaderRectHelper

#endif  // MODELTESTHELPER_H
