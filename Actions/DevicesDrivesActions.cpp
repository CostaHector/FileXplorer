#include "DevicesDrivesActions.h"
#include "PublicMacro.h"
#include <QMenu>
#include <QApplication>
#include <QStyle>

DevicesDrivesActions::DevicesDrivesActions(QObject* parent)  //
    : QObject{parent} {                                      //
  DEVICES_AND_DRIVES = new (std::nothrow) QAction{QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_DriveHDIcon), "Devices and Drives", this};
  CHECK_NULLPTR_RETURN_VOID(DEVICES_AND_DRIVES);
  DEVICES_AND_DRIVES->setCheckable(true);

  ADT_THIS_DRIVE = new (std::nothrow) QAction{QIcon(":img/AUDIT"), "Audit this drive", this};
  CHECK_NULLPTR_RETURN_VOID(ADT_THIS_DRIVE);

  MOUNT_THIS_DRIVE = new (std::nothrow) QAction{"Mount this drive", this};
  CHECK_NULLPTR_RETURN_VOID(MOUNT_THIS_DRIVE);
  MOUNT_THIS_DRIVE->setToolTip(QString("<b>%1 (%2)</b><br/> Mount the selected drive (Run as Administrator needed)")  //
                                   .arg(MOUNT_THIS_DRIVE->text(), MOUNT_THIS_DRIVE->shortcut().toString()));

  UNMOUNT_THIS_DRIVE = new (std::nothrow) QAction{"Unmount this drive", this};
  CHECK_NULLPTR_RETURN_VOID(UNMOUNT_THIS_DRIVE);
  UNMOUNT_THIS_DRIVE->setToolTip(QString("<b>%1 (%2)</b><br/> Umount the selected drive (Run as Administrator needed)")  //
                                     .arg(UNMOUNT_THIS_DRIVE->text(), UNMOUNT_THIS_DRIVE->shortcut().toString()));

  UPDATE_VOLUMES_INFO = new (std::nothrow) QAction{QIcon(":img/RELOAD_FROM_DISK"), "Update Volumes info", this};
  CHECK_NULLPTR_RETURN_VOID(UPDATE_VOLUMES_INFO);
}

QMenu* DevicesDrivesActions::GetMenu(QWidget* parent) {
  auto pMenu{new QMenu{"Devices and Drives Menu", parent}};
  pMenu->addAction(UPDATE_VOLUMES_INFO);
  pMenu->addSeparator();
  pMenu->addAction(ADT_THIS_DRIVE);
  pMenu->addSeparator();
  pMenu->addAction(MOUNT_THIS_DRIVE);
  pMenu->addAction(UNMOUNT_THIS_DRIVE);
  pMenu->setToolTipsVisible(true);
  return pMenu;
}

DevicesDrivesActions& DevicesDrivesActions::Inst() {
  static DevicesDrivesActions inst;
  return inst;
}
