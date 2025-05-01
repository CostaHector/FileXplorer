#ifndef DEVICESDRIVETABLEVIEW_H
#define DEVICESDRIVETABLEVIEW_H

#include "CustomTableView.h"
#include "Tools/FileDescriptor/DevicesAndDriverDb.h"
#include "Model/DevicesDriveModel.h"
class DevicesDriveTableView : public CustomTableView
{
 public:
  explicit DevicesDriveTableView(const QString& name, QWidget* parent = nullptr);
  void closeEvent(QCloseEvent* event) override;
  void ReadSettings();
 private:
  DevicesAndDriverDb mDb;
  DevicesDriveModel* mDevModel{nullptr};
};

#endif // DEVICESDRIVETABLEVIEW_H
