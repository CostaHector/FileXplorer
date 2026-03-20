#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>

#include "BeginToExposePrivateMember.h"
#include "MemoryKey.h"
#include "ConfigsTable.h"
#include "EndToExposePrivateMember.h"
#include "FileLeafAction.h"
#include "FileTool.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

#include "GlbDataProtect.h"
#include <QPushButton>

class ConfigsTableTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void init() { GlobalMockObject::reset(); }
  void cleanup() { GlobalMockObject::verify(); }

  void default_ok() {
    GlbDataProtect<QList<const KV*>> editableKVsBackup{KV::mEditableKVs};
    KV::mEditableKVs.clear();

    Configuration().clear();

    // not crash
    ConfigsTable cfgTbl;
    QVERIFY(cfgTbl.m_failItemCnt != nullptr);
    QVERIFY(cfgTbl.m_alertModel != nullptr);
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

    const QString cfgFilePath{Configuration().fileName()};
    bool bCfgExist{QFile::exists(cfgFilePath)};
    if (bCfgExist) {
      MOCKER(FileTool::OpenLocalFile)
          .expects(exactly(3))
          .with(eq(cfgFilePath))
          .will(returnValue(false))                                      // 1st
          .then(returnValue(true))                                       // 2nd
          .then(returnValue(true));                                      // 3rd
      QCOMPARE(cfgTbl.onEditPreferenceSetting(), false);                 // 1st
      QCOMPARE(cfgTbl.onEditPreferenceSetting(), true);                  // 2nd
      emit cfgTbl.m_dlgBtnBox->button(QDialogButtonBox::Open)->click();  // 3rd
    } else {
      MOCKER(FileTool::OpenLocalFile).expects(never()).will(returnValue(true));
      QCOMPARE(cfgTbl.onEditPreferenceSetting(), false);
    }

    QCOMPARE(cfgTbl.on_cellDoubleClicked({}), false);

    QSignalSpy dlgAcceptedWhenOkClicked{&cfgTbl, &QDialog::accepted};
    emit cfgTbl.m_dlgBtnBox->button(QDialogButtonBox::Ok)->click();
    QCOMPARE(dlgAcceptedWhenOkClicked.count(), 1);
    dlgAcceptedWhenOkClicked.takeLast();
  }

  void label_message_after_user_edit_ok() {
    GlbDataProtect<QList<const KV*>> editableKVsBackup{KV::mEditableKVs};
    KV::mEditableKVs.clear();

    KV playerFilePath{"playerFilePath", "inexists player file path", ValueChecker{VALUE_CHECKER_TYPE::VALUE_TYPE::FILE_PATH}, true};    // invalid
    KV workFolderPath{"workFolderPath", "inexists work folder path", ValueChecker{VALUE_CHECKER_TYPE::VALUE_TYPE::FOLDER_PATH}, true};  // invalid
    KV volumeValue{"volumeValue", -5, ValueChecker{0, 101}, true};                                                                      // invalid
    KV playerMute{"playerMute", false, ValueChecker{VALUE_CHECKER_TYPE::VALUE_TYPE::PLAIN_BOOL}, true};                                 // valid
    QCOMPARE(KV::mEditableKVs.size(), 4);

    // 配置当前值清空, 检查总共4条配置, 预设值有3个错误; 标签内容正确
    Configuration().clear();
    ConfigsTable cfgTbl;
    QString failed3ItemsMsg = cfgTbl.m_failItemCnt->text();
    QVERIFY(failed3ItemsMsg.contains("3 in 4 setting(s) error"));
    QVERIFY(!failed3ItemsMsg.contains("All 4 setting passed"));
    QVERIFY(cfgTbl.m_alertModel != nullptr);
    QAbstractTableModel& model = *cfgTbl.m_alertModel;

    QModelIndex playerFilePathIndex{model.index(0, ConfigsModel::NAME)};
    QCOMPARE(model.data(playerFilePathIndex).toString(), "playerFilePath");

    QModelIndex volumeValueIndex{model.index(2, ConfigsModel::NAME)};
    QCOMPARE(model.data(volumeValueIndex).toString(), "volumeValue");

    // 外部修改正确2条配置, 标签内容需要点击Retry才能刷新标签, 剩余失败数=3-2=1条
    const QString correctPlayerPath{__FILE__};
    const QString correctWorkFolderPath{QFileInfo{__FILE__}.absolutePath()};
    const int correctVolumeValue{99};
    Configuration().setValue(playerFilePath.name, correctPlayerPath);
    Configuration().setValue(workFolderPath.name, correctWorkFolderPath);
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

    MOCKER(FileTool::OpenLocalFile)
        .expects(exactly(3))
        .with(eq(correctPlayerPath))  //
        .will(returnValue(false))     // 1st
        .then(returnValue(true))      // 2nd
        .then(returnValue(true));     // 3rd

    // invalid index, skip ok
    QCOMPARE(cfgTbl.on_cellDoubleClicked({}), false);

    // is not a path at all
    QCOMPARE(cfgTbl.on_cellDoubleClicked(volumeValueIndex), false);

    QCOMPARE(cfgTbl.on_cellDoubleClicked(playerFilePathIndex), false);   // 1st
    QCOMPARE(cfgTbl.on_cellDoubleClicked(playerFilePathIndex), true);  // 2nd
    emit cfgTbl.m_alertsTable->doubleClicked(playerFilePathIndex);      // 3rd
  }
};

#include "ConfigsTableTest.moc"
REGISTER_TEST(ConfigsTableTest, false)
