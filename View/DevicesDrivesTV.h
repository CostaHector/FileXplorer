#ifndef DEVICESDRIVESTV_H
#define DEVICESDRIVESTV_H

#include "CustomTableView.h"
#include <QStyledItemDelegate>
#include <QStandardItemModel>

struct DiskInfo {
  QString diskName;      // 磁盘名称（Windows 为盘符，Linux 为挂载点）
  qint64 totalSpace;     // 总容量（字节）
  qint64 occupiedSpace;  // 已使用空间（字节）
};
using DiskInfoList = QList<DiskInfo>;
DiskInfoList GetDiskInfoList();

namespace DevicesDrivesTVMock {
inline DiskInfoList& DiskInfoListMock() {
  static DiskInfoList diskInfoLst;
  return diskInfoLst;
}
inline void clear() {
  DiskInfoListMock().clear();
}
}  // namespace DevicesDrivesTVMock

class ProgressDelegate : public QStyledItemDelegate {
 public:
  explicit ProgressDelegate(QStandardItemModel* model, QObject* parent = nullptr);
  float GetUsedPercentage(const QModelIndex& index) const;
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

 private:
  QStandardItemModel* mModel{nullptr};
};

class DevicesDrivesTV : public CustomTableView {
 public:
  explicit DevicesDrivesTV(QWidget* parent = nullptr);
  void closeEvent(QCloseEvent* event) override;
  void ReadSettings();
  void showEvent(QShowEvent* event) override;

 private:
  DiskInfoList mDisks;
  QStandardItemModel* mDevModel{nullptr};
  QStyledItemDelegate* mProgressStyleDelegate{nullptr};
};

#endif  // DEVICESDRIVESTV_H
