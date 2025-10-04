#include "DevicesDrivesTV.h"
#include "PublicMacro.h"
#include "NotificatorMacro.h"
#include "MemoryKey.h"
#include "TableFields.h"
#include "StyleSheet.h"

#include <QStorageInfo>
#include <QPainter>

DiskInfoList GetDiskInfoList() {
  // 获取所有挂载的存储设备
  const QList<QStorageInfo>& volumes = QStorageInfo::mountedVolumes();
  DiskInfoList diskList;
  diskList.reserve(volumes.size());
  for (const QStorageInfo& storage : volumes) {
    if (!storage.isValid() || !storage.isReady()) {
      // 跳过无效或未就绪的存储设备
      continue;
    }
    // 跳过只读设备（如光盘）
    if (storage.isReadOnly()) {
      continue;
    }
    // 在 Windows 上跳过网络驱动器
#ifdef Q_OS_WIN
    if (storage.rootPath().startsWith("\\\\"))
      continue;
#endif
      // 在 Linux 上跳过系统虚拟文件系统
#ifdef Q_OS_LINUX
    QString fsType = storage.fileSystemType();
    if (fsType == "tmpfs" || fsType == "proc" || fsType == "sysfs" || fsType == "devtmpfs" || fsType == "cgroup" || fsType == "overlay") {
      continue;
    }
#endif
    DiskInfo info;
    info.diskName = storage.rootPath();
    info.totalSpace = storage.bytesTotal();
    info.occupiedSpace = storage.bytesTotal() - storage.bytesFree();
    diskList.append(info);
  }
  return diskList;
}

ProgressDelegate::ProgressDelegate(QStandardItemModel* model, QObject* parent)  //
    : QStyledItemDelegate(parent), mModel{model} {}
float ProgressDelegate::GetUsedPercentage(const QModelIndex& index) const {
  CHECK_NULLPTR_RETURN_INT(mModel, 0);
  return (float)mModel->index(index.row(), DEV_DRV_TABLE::USED_BYTES).data().toLongLong()  //
         / mModel->index(index.row(), DEV_DRV_TABLE::TOTAL_BYTES).data().toLongLong();     //
}
void ProgressDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);
  const float percentage{GetUsedPercentage(index)};
  painter->save();
  painter->fillRect(opt.rect.left(), opt.rect.top(),                         //
                    opt.rect.width() * percentage, opt.rect.height(),  //
                    QColor::fromHsv((1 - percentage) * 120, 255, 255));
  painter->restore();
  QStyledItemDelegate::paint(painter, opt, index);
}

DevicesDrivesTV::DevicesDrivesTV(QWidget* parent)  //
    : CustomTableView{"DevicesAndDrives", parent}  //
{
#ifdef RUNNING_UNIT_TESTS
  mDisks = DevicesDrivesTVMock::DiskInfoListMock();
#else
  mDisks = GetDiskInfoList();
#endif
  mDevModel = new (std::nothrow) QStandardItemModel;
  CHECK_NULLPTR_RETURN_VOID(mDevModel);
  mDevModel->setRowCount(mDisks.size());
  mDevModel->setColumnCount(DEV_DRV_TABLE::FILED_BUTT);
  for (int row = 0; row < mDevModel->rowCount(); ++row) {
    QModelIndex idxRootPath = mDevModel->index(row, DEV_DRV_TABLE::ROOT_PATH);
    QModelIndex idxTotalSpace = mDevModel->index(row, DEV_DRV_TABLE::TOTAL_BYTES);
    QModelIndex idxUsedSpace = mDevModel->index(row, DEV_DRV_TABLE::USED_BYTES);
    mDevModel->setData(idxRootPath, mDisks[row].diskName);
    mDevModel->setData(idxTotalSpace, mDisks[row].totalSpace);
    mDevModel->setData(idxUsedSpace, mDisks[row].occupiedSpace);
  }
  mDevModel->setHorizontalHeaderLabels(DEV_DRV_TABLE::GetDevDrvTableHeaders());
  setModel(mDevModel);

  mProgressStyleDelegate = new (std::nothrow) ProgressDelegate{mDevModel};
  CHECK_NULLPTR_RETURN_VOID(mProgressStyleDelegate);

  setItemDelegateForColumn(DEV_DRV_TABLE::ROOT_PATH, mProgressStyleDelegate);
  setEditTriggers(QAbstractItemView::NoEditTriggers);  // only F2 works. QAbstractItemView.NoEditTriggers

  InitTableView();
  ReadSettings();

  setWindowIcon(QIcon(":img/DISKS"));
  setWindowTitle("Devices and Drives");
}

void DevicesDrivesTV::closeEvent(QCloseEvent* event) {
  Configuration().setValue("DevicesDriveTableViewGeometry", saveGeometry());
  CustomTableView::closeEvent(event);
}

void DevicesDrivesTV::ReadSettings() {
  if (Configuration().contains("DevicesDriveTableViewGeometry")) {
    restoreGeometry(Configuration().value("DevicesDriveTableViewGeometry").toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }
}

void DevicesDrivesTV::showEvent(QShowEvent* event) {
  CustomTableView::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}
