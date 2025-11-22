#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "DevicesDrivesTV.h"
#include "EndToExposePrivateMember.h"
#include "TableFields.h"
#include "MemoryKey.h"
#include <QPainter>

class DevicesDrivesTVTest : public PlainTestSuite {
  Q_OBJECT

 private slots:
  void initTestCase() {
    DevicesDrivesTVMock::clear();
    Configuration().clear();

    GetDiskInfoList();
  }

  void cleanupTestCase() {
    DevicesDrivesTVMock::clear();  //
  }

  void constructor_ok() {
    DevicesDrivesTVMock::DiskInfoListMock() = {{"C:", 50000000000, 30000000000},  //
                                               {"D:", 100000000000, 80000000000}};
    QCOMPARE(DevicesDrivesTVMock::DiskInfoListMock().size(), 2);

    DevicesDrivesTV devicesDrivesTV;
    QVERIFY(devicesDrivesTV.mDevModel != nullptr);
    QCOMPARE(devicesDrivesTV.mDevModel->rowCount(), 2);
    QCOMPARE(devicesDrivesTV.mDevModel->columnCount(), DEV_DRV_TABLE::FILED_BUTT);

    QStringList expectedHeaders = DEV_DRV_TABLE::GetDevDrvTableHeaders();
    QCOMPARE(devicesDrivesTV.mDevModel->horizontalHeaderItem(DEV_DRV_TABLE::ROOT_PATH)->text(), expectedHeaders[DEV_DRV_TABLE::ROOT_PATH]);

    QVERIFY(devicesDrivesTV.mProgressStyleDelegate != nullptr);
  }

  void modelPopulation_ok() {
    DevicesDrivesTV devicesDrivesTV;
    QStandardItemModel* model = devicesDrivesTV.mDevModel;

    QCOMPARE(model->rowCount(), 2);
    devicesDrivesTV.sortByColumn(DEV_DRV_TABLE::ROOT_PATH, Qt::SortOrder::AscendingOrder);

    QModelIndex rootPathIndex = model->index(0, DEV_DRV_TABLE::ROOT_PATH);
    QModelIndex totalBytesIndex = model->index(0, DEV_DRV_TABLE::TOTAL_BYTES);
    QModelIndex usedBytesIndex = model->index(0, DEV_DRV_TABLE::USED_BYTES);

    QCOMPARE(model->data(rootPathIndex).toString(), QString("C:"));
    QCOMPARE(model->data(totalBytesIndex).toLongLong(), 50000000000LL);
    QCOMPARE(model->data(usedBytesIndex).toLongLong(), 30000000000LL);
  }

  void progressDelegate_ok() {
    DevicesDrivesTV devicesDrivesTV;
    QStandardItemModel* model = devicesDrivesTV.mDevModel;
    ProgressDelegate delegate(model);

    devicesDrivesTV.sortByColumn(DEV_DRV_TABLE::ROOT_PATH, Qt::SortOrder::AscendingOrder);
    QModelIndex index = model->index(0, DEV_DRV_TABLE::ROOT_PATH);
    float percentage = delegate.GetUsedPercentage(index);
    QCOMPARE(percentage, 0.6f);  // 30GB / 50GB = 0.6

    QImage image(200, 50, QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter(&image);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 50);
    option.state = QStyle::State_Enabled;

    delegate.paint(&painter, option, index);

    QVERIFY(!image.isNull());
  }

  void settingsPersistence_ok() {
    DevicesDrivesTV devicesDrivesTV;

    QCloseEvent closeEvent;
    devicesDrivesTV.closeEvent(&closeEvent);

    Configuration().contains("DevicesDriveTableViewGeometry");
  }

  void data_retrieve_ok() {
    DevicesDrivesTVMock::clear();
    DevicesDrivesTVMock::DiskInfoListMock().append({"/", 200000000000, 150000000000});
    DevicesDrivesTV devicesDrivesTV;
    QStandardItemModel* model = devicesDrivesTV.mDevModel;
    QCOMPARE(model->rowCount(), 1);
    devicesDrivesTV.sortByColumn(DEV_DRV_TABLE::ROOT_PATH, Qt::SortOrder::AscendingOrder);

    QModelIndex rootPathIndex = model->index(0, DEV_DRV_TABLE::ROOT_PATH);
    QCOMPARE(model->data(rootPathIndex).toString(), QString("/"));
  }

  void show_close_Event_ok() {
    DevicesDrivesTV devicesDrivesTV;
    QShowEvent showEvent;
    devicesDrivesTV.showEvent(&showEvent);

    // will not crash down
    QByteArray originalGeometry = devicesDrivesTV.saveGeometry();

    QCloseEvent closeEvent;
    devicesDrivesTV.closeEvent(&closeEvent);

    QByteArray savedGeometry = Configuration().value("DevicesDriveTableViewGeometry").toByteArray();
    QCOMPARE(savedGeometry, originalGeometry);
  }
};

#include "DevicesDrivesTVTest.moc"
REGISTER_TEST(DevicesDrivesTVTest, false)
