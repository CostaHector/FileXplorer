#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "ConfigsMgr.h"
#include "EndToExposePrivateMember.h"

#include "ConfigsModel.h"
#include "Configuration.h"
#include "FileLeafAction.h"
#include "FileTool.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

#include "GlbDataProtect.h"
#include <QPushButton>

class ConfigsMgrTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void init() { GlobalMockObject::reset(); }
  void cleanup() { GlobalMockObject::verify(); }

  void default_ok() {
    Configuration().clear();

    // not crash
    ConfigsMgr cfgTbl;
    QVERIFY(cfgTbl.m_failItemCnt != nullptr);
    QVERIFY(cfgTbl.m_alertsTable != nullptr);
    QVERIFY(cfgTbl.m_dlgBtnBox != nullptr);
    QVERIFY(cfgTbl.m_dlgBtnBox->button(QDialogButtonBox::Ok) != nullptr);
    QVERIFY(cfgTbl.m_dlgBtnBox->button(QDialogButtonBox::Retry) != nullptr);
    QVERIFY(cfgTbl.m_dlgBtnBox->button(QDialogButtonBox::Open) != nullptr);

    cfgTbl.showEvent(nullptr);

    QShowEvent defaultShowEvent;
    cfgTbl.showEvent(&defaultShowEvent);

    cfgTbl.hideEvent(nullptr);
    QHideEvent defaultHideEvent;
    cfgTbl.hideEvent(&defaultHideEvent);
    QCOMPARE(g_fileLeafActions()._SETTINGS->isChecked(), false);

    cfgTbl.RefreshWindowIcon();

    QSignalSpy dlgAcceptedWhenOkClicked{&cfgTbl, &QDialog::accepted};
    emit cfgTbl.m_dlgBtnBox->button(QDialogButtonBox::Ok)->click();
    QCOMPARE(dlgAcceptedWhenOkClicked.count(), 1);
    dlgAcceptedWhenOkClicked.takeLast();
  }

  void onEditPreferenceSetting_ok() {
    ConfigsMgr cfgTbl;

    const QString cfgFilePath{Configuration().fileName()};

    bool bCfgExist{QFile::exists(cfgFilePath)};
    if (bCfgExist) {
      MOCKER(FileTool::OpenLocalFile)
          .expects(exactly(3))
          .with(eq(cfgFilePath))
          .will(returnValue(false))  // 1st
          .then(returnValue(true))   // 2nd
          .then(returnValue(true));  // 3rd

      QCOMPARE(cfgTbl.onEditPreferenceSetting(), false);                 // 1st
      QCOMPARE(cfgTbl.onEditPreferenceSetting(), true);                  // 2nd
      emit cfgTbl.m_dlgBtnBox->button(QDialogButtonBox::Open)->click();  // 3rd
    } else {
      MOCKER(FileTool::OpenLocalFile).expects(never()).will(returnValue(true));

      QCOMPARE(cfgTbl.onEditPreferenceSetting(), false);
    }
  }

  void label_message_after_user_edit_ok() {
    QCOMPARE(KVTestOnly::KV_TEST_COUNT, 4);

    // 配置当前值清空, 检查总共4条配置, 预设值有3个错误; 标签内容正确
    Configuration().clear();
    ConfigsMgr cfgTbl;

    const QAbstractTableModel* pConstModel = cfgTbl.m_alertsTable->GetModel();
    QVERIFY(pConstModel != nullptr);
    QCOMPARE(pConstModel->rowCount(), KVTestOnly::KV_TEST_COUNT);

    QAbstractTableModel* pModel = cfgTbl.m_alertsTable->GetModel();
    QVERIFY(pModel != nullptr);
    QCOMPARE(pModel->rowCount(), KVTestOnly::KV_TEST_COUNT);

    QCOMPARE(pModel, pConstModel);

    QString failed3ItemsMsg = cfgTbl.m_failItemCnt->text();
    QVERIFY(failed3ItemsMsg.contains("3 in 4 setting(s) error"));
    QVERIFY(!failed3ItemsMsg.contains("All 4 setting passed"));

    QAbstractTableModel& model = *pModel;

    QModelIndex playerFilePathIndex{model.index(0, ConfigsModel::NAME)};
    QCOMPARE(model.data(playerFilePathIndex).toString(), "playerFilePath");

    QModelIndex volumeValueIndex{model.index(2, ConfigsModel::NAME)};
    QCOMPARE(model.data(volumeValueIndex).toString(), "volumeValue");

    // 外部修改正确2条配置, 标签内容需要点击Retry才能刷新标签, 剩余失败数=3-2=1条
    const QString correctPlayerPath{__FILE__};
    const QString correctWorkFolderPath{QFileInfo{__FILE__}.absolutePath()};
    const int correctVolumeValue{99};
    setConfig(KVTestOnly::playerFilePath, correctPlayerPath);
    setConfig(KVTestOnly::workFolderPath, correctWorkFolderPath);
    const QString stillFailed3ItemsMsg = cfgTbl.m_failItemCnt->text();
    QVERIFY(stillFailed3ItemsMsg.contains("3 in 4 setting(s) error"));
    QVERIFY(!stillFailed3ItemsMsg.contains("All 4 setting passed"));

    // 点击Retry刷新标签
    QSignalSpy dialogNotAcceptedWhenRetryClicked{&cfgTbl, &QDialog::accepted};
    QPushButton* retryBtn = cfgTbl.m_dlgBtnBox->button(QDialogButtonBox::Retry);
    QVERIFY(retryBtn != nullptr);
    retryBtn->click();
    const QString failed1ItemsMsg = cfgTbl.m_failItemCnt->text();
    QVERIFY(failed1ItemsMsg.contains("1 in 4 setting(s) error"));
    QVERIFY(!failed1ItemsMsg.contains("All 4 setting passed"));
    QCOMPARE(dialogNotAcceptedWhenRetryClicked.count(), 0);

    // 通过model修改正确1条, 将自动刷新标签, 剩余失败数=1-1=0条
    QCOMPARE(model.setData(volumeValueIndex.siblingAtColumn(ConfigsModel::CURRENT_VALUE), correctVolumeValue, Qt::EditRole), true);
    const QString failed0ItemsMsg = cfgTbl.m_failItemCnt->text();
    QVERIFY(!failed0ItemsMsg.contains("0 in 4 setting(s) error"));
    QVERIFY(failed0ItemsMsg.contains("All 4 setting passed"));
  }
};

#include "ConfigsMgrTest.moc"
REGISTER_TEST(ConfigsMgrTest, false)
