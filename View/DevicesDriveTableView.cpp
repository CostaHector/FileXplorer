#include "DevicesDriveTableView.h"
#include "public/PublicMacro.h"
#include "public/PublicVariable.h"
#include <QStyledItemDelegate>
#include <QPainter>
#include <QApplication>

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

DevicesDriveTableView::DevicesDriveTableView(const QString& name, QWidget* parent)  //
    : CustomTableView{name, parent},                                                //
      mDb{SystemPath::DEVICES_AND_DRIVER_DATABASE, "DeviceAndDriverConn"}           //
{
  if (!mDb.CreateDatabase()) {
    qWarning("CreateDatabase failed");
    return;
  }
  if (!mDb.CreateTable(DB_TABLE::DEVICE_AND_DRIVER, DevicesAndDriverDb::CREATE_DEV_DRV_TEMPLATE)) {
    qWarning("CreateTable failed");
    return;
  }
  if (mDb.InitDeviceAndDriver(DB_TABLE::DEVICE_AND_DRIVER) < FD_OK) {
    qWarning("InitDeviceAndDriver failed");
    return;
  }
  auto con = mDb.GetDb();
  mDevModel = new (std::nothrow) DevicesDriveModel{this, con};
  CHECK_NULLPTR_RETURN_VOID(mDevModel);

  mDevModel->setTable(DB_TABLE::DEVICE_AND_DRIVER);
  mDevModel->select();

  setModel(mDevModel);

  auto* p = new ProgressDelegate{mDevModel};
  setItemDelegateForColumn(0, p);

  setEditTriggers(QAbstractItemView::NoEditTriggers);  // only F2 works. QAbstractItemView.NoEditTriggers

  InitTableView();
  setWindowTitle("Devices and Drives");
  ReadSettings();
}

void DevicesDriveTableView::closeEvent(QCloseEvent* event) {
  PreferenceSettings().setValue("DevicesDriveTableViewGeometry", saveGeometry());
  CustomTableView::closeEvent(event);
}

void DevicesDriveTableView::ReadSettings() {
  if (PreferenceSettings().contains("DevicesDriveTableViewGeometry")) {
    restoreGeometry(PreferenceSettings().value("DevicesDriveTableViewGeometry").toByteArray());
  } else {
    setGeometry(QRect(0, 0, 1024, 768));
  }
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
