#include "DevicesDrivesTV.h"
#include "public/PublicMacro.h"
#include "public/PublicVariable.h"
#include "Actions/DevicesDrivesActions.h"
#include "Tools/FileDescriptor/MountHelper.h"
#include "Component/Notificator.h"
#include <QPainter>
#include <QApplication>
#include <QDate>
#include "Tools/FileDescriptor/TableFields.h"

using namespace DEV_DRV_TABLE;
class ProgressDelegate : public QStyledItemDelegate {
 public:
  ProgressDelegate(QSqlTableModel* model, QObject* parent = nullptr)  //
      : QStyledItemDelegate(parent), mModel{model} {}
  QSqlTableModel* mModel;

  inline float GetUsedPercentage(const QModelIndex& index) const {
    if (mModel == nullptr) {
      return 0.0f;
    }
    return (float)mModel->QSqlTableModel::data(mModel->index(index.row(), AVAIL_BYTES)).toLongLong()  //
           / mModel->QSqlTableModel::data(mModel->index(index.row(), TOTAL_BYTES)).toLongLong();
  }

  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    const float percentage{GetUsedPercentage(index)};
    painter->save();
    painter->fillRect(opt.rect.left(), opt.rect.top(),                         //
                      opt.rect.width() * (1 - percentage), opt.rect.height(),  //
                      QColor::fromHsv(percentage * 120, 255, 255));
    painter->restore();
    QStyledItemDelegate::paint(painter, opt, index);
  }
};

DevicesDrivesTV::DevicesDrivesTV(QWidget* parent)                          //
    : CustomTableView{"DevicesAndDrives", parent},                         //
      mDb{SystemPath::DEVICES_AND_DRIVES_DATABASE, "DeviceAndDriverConn"}  //
{
  if (!mDb.CreateDatabase()) {
    qWarning("CreateDatabase failed");
    return;
  }
  if (!mDb.CreateTable(DB_TABLE::DISKS, DevicesAndDriverDb::CREATE_DEV_DRV_TEMPLATE)) {
    qWarning("CreateTable failed");
    return;
  }
  if (mDb.InitDeviceAndDriver(DB_TABLE::DISKS) < FD_OK) {
    qWarning("InitDeviceAndDriver failed");
    return;
  }
  auto con = mDb.GetDb();
  mDevModel = new (std::nothrow) DevicesDriveModel{this, con};
  CHECK_NULLPTR_RETURN_VOID(mDevModel);
  mDevModel->setTable(DB_TABLE::DISKS);
  mDevModel->select();
  setModel(mDevModel);

  mProgressStyleDelegate = new ProgressDelegate{mDevModel};
  setItemDelegateForColumn(ROOT_PATH, mProgressStyleDelegate);
  setEditTriggers(QAbstractItemView::NoEditTriggers);  // only F2 works. QAbstractItemView.NoEditTriggers

  mMenu = DevicesDrivesActions::Inst().GetMenu(this);

  InitTableView();
  ReadSettings();

  subscribe();

  _DEVICE_AND_DRIVES = DevicesDrivesActions::Inst().DEVICES_AND_DRIVES;
  CHECK_NULLPTR_RETURN_VOID(_DEVICE_AND_DRIVES);
  setWindowIcon(_DEVICE_AND_DRIVES->icon());
  setWindowTitle(_DEVICE_AND_DRIVES->text());
}

void DevicesDrivesTV::closeEvent(QCloseEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(_DEVICE_AND_DRIVES);
  _DEVICE_AND_DRIVES->setChecked(false);
  PreferenceSettings().setValue("DevicesDriveTableViewGeometry", saveGeometry());
  CustomTableView::closeEvent(event);
}

void DevicesDrivesTV::ReadSettings() {
  if (PreferenceSettings().contains("DevicesDriveTableViewGeometry")) {
    restoreGeometry(PreferenceSettings().value("DevicesDriveTableViewGeometry").toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }
}

void DevicesDrivesTV::contextMenuEvent(QContextMenuEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(mMenu);
  const QModelIndex& index = indexAt(event->pos());
  const bool bRowChecked{index.isValid()};
  auto& inst = DevicesDrivesActions::Inst();
  inst.ADT_THIS_DRIVE->setVisible(bRowChecked);
  inst.MOUNT_THIS_DRIVE->setVisible(bRowChecked);
  inst.UNMOUNT_THIS_DRIVE->setVisible(bRowChecked);
  inst.UPDATE_VOLUMES_INFO->setVisible(!bRowChecked);
  mMenu->exec(event->globalPos());
}

void DevicesDrivesTV::onUpdateVolumes() {
  VolumeUpdateResult resStat{0};
  const int ret = mDb.AdtDeviceAndDriver(DB_TABLE::DISKS, &resStat);
  if (ret != FD_OK) {
    Notificator::badNews(QString{"Update Volume(s) FAILED, errorCode:%1"}.arg(ret), "See details in log");
    return;
  }
  Notificator::goodNews("Update Volume(s) ok", QString{"Insert:%1, Delete:%2, Update:%3"}.arg(resStat.insertCnt).arg(resStat.deleteCnt).arg(resStat.updateCnt));
  mDevModel->select();
}

void DevicesDrivesTV::onMountADriver() {
  const QModelIndex& index = currentIndex();
  if (!index.isValid()) {
    return;
  }
  const QString& guid = mDevModel->GetGuid(index);
  const QString label{"D" + QDate::currentDate().toString("yyMM")};
  QString volMountPoint;
  if (!MountHelper::MountVolume(guid, label, volMountPoint)) {
    Notificator::badNews(QString{"Mount Volume(s)[%1] FAILED"}.arg(guid), "See details in log");
    return;
  }
  Notificator::goodNews(QString{"Mount Volume(s)[%1] ok"}.arg(guid), label + " in " + volMountPoint);
  auto setRet = mDevModel->setData(index.siblingAtColumn(MOUNT_POINT), volMountPoint);
  auto submitRet = mDevModel->submitAll();
  qDebug("setData:%d, submitAll: %d", setRet, submitRet);
}

void DevicesDrivesTV::onUnmountADriver() {
  const QModelIndex& index = currentIndex();
  if (!index.isValid()) {
    return;
  }
  const QString& mountedPnt = mDevModel->GetMountedPoint(index);
  if (mountedPnt.isEmpty()) {
    return;
  }
  const QString& guid = mDevModel->GetGuid(index);
  if (!MountHelper::UnmountVolume(mountedPnt)) {
    Notificator::badNews(QString{"Unmount Volume(s)[%1] from pnt:%2 FAILED"}.arg(guid).arg(mountedPnt), "See details in log");
    return;
  }
  Notificator::goodNews(QString{"Unmount Volume(s)[%1] from pnt:%2 ok"}.arg(guid).arg(mountedPnt), "NULL");
  auto setRet = mDevModel->setData(index.siblingAtColumn(MOUNT_POINT), "");
  auto submitRet = mDevModel->submitAll();
  qDebug("setData:%d, submitAll: %d", setRet, submitRet);
}

void DevicesDrivesTV::onAdtADriver() {
  const QModelIndex& index = currentIndex();
  if (!index.isValid()) {
    return;
  }
  const QString& guid = mDevModel->GetGuid(index);
  const QString& rootPath = mDevModel->GetRootPath(index);
  Notificator::badNews(QString{"Adt Volume(s)[%1] %2 FAILED"}.arg(guid).arg(rootPath), "No support now...");
  auto setRet = mDevModel->setData(index.siblingAtColumn(ADT_TIME), QDateTime::currentMSecsSinceEpoch());
  auto submitRet = mDevModel->submitAll();
  qDebug("setData:%d, submitAll: %d", setRet, submitRet);
}

void DevicesDrivesTV::subscribe() {
  connect(DevicesDrivesActions::Inst().UPDATE_VOLUMES_INFO, &QAction::triggered, this, &DevicesDrivesTV::onUpdateVolumes);
  connect(DevicesDrivesActions::Inst().MOUNT_THIS_DRIVE, &QAction::triggered, this, &DevicesDrivesTV::onMountADriver);
  connect(DevicesDrivesActions::Inst().UNMOUNT_THIS_DRIVE, &QAction::triggered, this, &DevicesDrivesTV::onUnmountADriver);
  connect(DevicesDrivesActions::Inst().ADT_THIS_DRIVE, &QAction::triggered, this, &DevicesDrivesTV::onAdtADriver);
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  DevicesDriveTableView win{"DeviceAndDrivePage"};
  win.show();
  return a.exec();
}
#endif
