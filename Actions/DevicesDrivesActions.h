#ifndef DEVICESDRIVESACTIONS_H
#define DEVICESDRIVESACTIONS_H

#include <QAction>
#include <QActionGroup>

class DevicesDrivesActions : public QObject {
 public:
  static DevicesDrivesActions& Inst();
  QAction* DEVICES_AND_DRIVES{nullptr};
  QAction* ADT_THIS_DRIVE{nullptr};
  QAction* MOUNT_THIS_DRIVE{nullptr};
  QAction* UNMOUNT_THIS_DRIVE{nullptr};
  QAction* UPDATE_VOLUMES_INFO{nullptr};
  QMenu* GetMenu(QWidget* parent);
 private:
  explicit DevicesDrivesActions(QObject* parent = nullptr);
};

#endif  // DEVICESDRIVESACTIONS_H
