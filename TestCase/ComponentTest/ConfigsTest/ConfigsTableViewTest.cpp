#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "ConfigsTableView.h"
#include "ConfigsModel.h"
#include "EndToExposePrivateMember.h"

#include "Configuration.h"
#include "FileTool.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class ConfigsTableViewTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void init() { //
    Configuration().clear();
    GlobalMockObject::reset();
  }
  void cleanup() { //
    GlobalMockObject::verify();
  }

  void cleanupTestCase() { //
    Configuration().clear();
  }

  void GetFailedCnt_ok() {
    const int failedCnt{3};
    ConfigsTableView cfgView{"cfgViewInTest"};
    QCOMPARE(cfgView.GetName(), "cfgViewInTest");
    QCOMPARE(cfgView.GetFailedCnt(), failedCnt);
    QCOMPARE(cfgView.ConfigsTableView::m_defaultShowBackgroundImage, true);
  }

  void on_cellDoubleClicked_ok() {
    ConfigsTableView cfgView{"cfgViewInTest"};
    ConfigsModel *srcModel = cfgView.m_cfgModel;
    QVERIFY(srcModel != nullptr);
    QSortFilterProxyModel *proxyModel = cfgView.mSortFilterProxy;
    QVERIFY(proxyModel != nullptr);

    QModelIndex src0 = srcModel->index(0, 0); // file
    QModelIndex src1 = srcModel->index(1, 0); // folder
    QModelIndex src2 = srcModel->index(2, 0); // int volume
    QModelIndex src3 = srcModel->index(3, 0); // bool mute

    QModelIndex pro0 = proxyModel->mapFromSource(src0);
    QModelIndex pro1 = proxyModel->mapFromSource(src1);
    QModelIndex pro2 = proxyModel->mapFromSource(src2);
    QModelIndex pro3 = proxyModel->mapFromSource(src3);

    const QFileInfo curFi{__FILE__};
    const QString filePath{curFi.absoluteFilePath()};
    const QString folderPath{curFi.absolutePath()};

    MOCKER(FileTool::OpenLocalFile) //
        .expects(exactly(2))        //
        .will(returnValue(true))    // 1st
        .then(returnValue(false));  // 2nd. return false for distinguish usage only

    QCOMPARE(cfgView.selectionModel()->hasSelection(), false);
    emit cfgView.doubleClicked({});

    QCOMPARE(cfgView.on_cellDoubleClicked({}), false);
    QCOMPARE(cfgView.on_cellDoubleClicked(pro0), false); // path related, but file path not exist
    QCOMPARE(cfgView.on_cellDoubleClicked(pro1), false); // path related, but folder path not exist
    QCOMPARE(cfgView.on_cellDoubleClicked(pro2), false); // not path related
    QCOMPARE(cfgView.on_cellDoubleClicked(pro3), false); // not path related

    setConfig(KVTestOnly::PLAYER_FILE_PATH, filePath);
    setConfig(KVTestOnly::WORK_FOLDER_PATH, folderPath);
    QCOMPARE(cfgView.on_cellDoubleClicked(pro0), true);  // 1st path related, and file path exist
    QCOMPARE(cfgView.on_cellDoubleClicked(pro1), false); // 2nd path related, and folder path exist
  }

  void setFilter_ok() {
    ConfigsTableView cfgView{"cfgViewInTest"};
    ConfigsModel *srcModel = cfgView.m_cfgModel;
    QVERIFY(srcModel != nullptr);
    QCOMPARE(srcModel->rowCount(), 4);

    QSortFilterProxyModel *proxyModel = cfgView.mSortFilterProxy;
    QVERIFY(proxyModel != nullptr);
    QCOMPARE(proxyModel->rowCount(), 4);
    QCOMPARE(proxyModel->sortCaseSensitivity(), Qt::CaseSensitivity::CaseInsensitive);
    QCOMPARE(proxyModel->filterCaseSensitivity(), Qt::CaseSensitivity::CaseInsensitive);

    cfgView.setFilter("NoRowMatch");
    QCOMPARE(proxyModel->rowCount(), 0);

    cfgView.setFilter("PLAYER_MUTE");
    QCOMPARE(proxyModel->rowCount(), 1);

    cfgView.setFilter("");
    QCOMPARE(proxyModel->rowCount(), 4);

    cfgView.setFilter("player_mute"); // 大小写不敏感
    QCOMPARE(proxyModel->rowCount(), 1);
  }

  void modelCfgFailedCountChanged_ok() {
    ConfigsTableView cfgView{"cfgViewInTest"};
    ConfigsModel *srcModel = cfgView.m_cfgModel;
    QVERIFY(srcModel != nullptr);

    QSignalSpy modelCfgFailedCountChangedSpy{srcModel, &ConfigsModel::failedCountChanged};
    emit srcModel->failedCountChanged(99);
    QCOMPARE(modelCfgFailedCountChangedSpy.count(), 1);
    QCOMPARE(modelCfgFailedCountChangedSpy.takeLast(), (QVariantList{99}));
  }
};

#include "ConfigsTableViewTest.moc"
REGISTER_TEST(ConfigsTableViewTest, false)
