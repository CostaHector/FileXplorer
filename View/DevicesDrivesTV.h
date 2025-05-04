#ifndef DEVICESDRIVESTV_H
#define DEVICESDRIVESTV_H

#include "CustomTableView.h"
#include "Tools/FileDescriptor/DevicesAndDriverDb.h"
#include "Model/DevicesDriveModel.h"
#include <QStyledItemDelegate>

class DevicesDrivesTV : public CustomTableView {
 public:
  explicit DevicesDrivesTV(QWidget* parent = nullptr);
  void closeEvent(QCloseEvent* event) override;
  void ReadSettings();
  void contextMenuEvent(QContextMenuEvent* event) override;
  void onUpdateVolumes();
  void onMountADriver();
  void onUnmountADriver();
  void onAdtADriver();
  void subscribe();
 private:
  QAction* _DEVICE_AND_DRIVES{nullptr};
  DevicesAndDriverDb mDb;
  DevicesDriveModel* mDevModel{nullptr};
  QStyledItemDelegate* mProgressStyleDelegate{nullptr};
  QMenu* mMenu{nullptr};
};

#endif  // DEVICESDRIVESTV_H
