#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "ConfigsMgr.h"
#include "EndToExposePrivateMember.h"

#include "ConfigsModel.h"
#include "Configuration.h"
#include "FileLeafAction.h"
#include "FileTool.h"
#include <QPushButton>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

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
    QVERIFY(cfgTbl.m_failedCountLabel != nullptr);
    QVERIFY(cfgTbl.m_cfgsTable != nullptr);
    QVERIFY(cfgTbl.m_dlgBtnBox != nullptr);
    QVERIFY(cfgTbl.m_dlgBtnBox->button(QDialogButtonBox::Ok) != nullptr);
    QVERIFY(cfgTbl.m_dlgBtnBox->button(QDialogButtonBox::Open) != nullptr);

    cfgTbl.showEvent(nullptr);

    QShowEvent defaultShowEvent;
    cfgTbl.showEvent(&defaultShowEvent);

    cfgTbl.hideEvent(nullptr);
    QHideEvent defaultHideEvent;
    cfgTbl.hideEvent(&defaultHideEvent);
    QCOMPARE(g_fileLeafActions()._SETTINGS->isChecked(), false);

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
          .will(returnValue(false)) // 1st
          .then(returnValue(true))  // 2nd
          .then(returnValue(true)); // 3rd

      QCOMPARE(cfgTbl.onEditPreferenceSetting(), false);                // 1st
      QCOMPARE(cfgTbl.onEditPreferenceSetting(), true);                 // 2nd
      emit cfgTbl.m_dlgBtnBox->button(QDialogButtonBox::Open)->click(); // 3rd
    } else {
      MOCKER(FileTool::OpenLocalFile).expects(never()).will(returnValue(true));
      QCOMPARE(cfgTbl.onEditPreferenceSetting(), false);
    }
  }

  void label_updated_ok() {
    ConfigsMgr cfgTbl;
    QLineEdit* searchLe = cfgTbl.m_searchLineEdit;
    QAction* searchAct = cfgTbl.m_searchAction;
    QAction* recheckAct = cfgTbl.m_recheckAction;
    QLabel* failedLabel = cfgTbl.m_failedCountLabel;
    ConfigsTableView* cfgView = cfgTbl.m_cfgsTable;

    QVERIFY(searchLe != nullptr);
    QVERIFY(searchAct != nullptr);
    QVERIFY(recheckAct != nullptr);
    QVERIFY(failedLabel != nullptr);
    QVERIFY(cfgView != nullptr);

    QString failedText = failedLabel->text();
    QVERIFY(failedText.contains("Failed: 3"));

    QSignalSpy acceptSpy{&cfgTbl, &QDialog::accept};

    {
      // model内容不改变, 直接emit信号, 模拟刷新
      emit cfgView->modelCfgFailedCountChanged(99);
      failedText = failedLabel->text();
      QVERIFY(failedText.contains("Failed: 99"));

      cfgTbl.RefreshFailedCountLabel();
      failedText = failedLabel->text();
      QVERIFY(failedText.contains("Failed: 3"));

      // fail=0时, "All Passed"
      emit cfgView->modelCfgFailedCountChanged(0);
      failedText = failedLabel->text();
      QVERIFY(failedText.contains("All Passed"));

      // 点击action, 模拟强制重新检查
      recheckAct->trigger();
      failedText = failedLabel->text();
      QVERIFY(failedText.contains("Failed: 3"));
    }

    {
      MOCKER(ConfigsMgr::focusWidgetCore)            //
          .expects(exactly(2))                       //
          .will(returnValue((QWidget*) failedLabel)) // 1st
          .then(returnValue((QWidget*) cfgView));    // 2nd

      // enter/return eater
      cfgTbl.keyPressEvent(nullptr);

      // focus在lineEdit中, return不会关闭窗口
      failedLabel->setFocus();
      QKeyEvent enter0PressEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
      cfgTbl.keyPressEvent(&enter0PressEvent); // 1st
      QCOMPARE(acceptSpy.count(), 0);          // 接受按钮未触发窗口关闭信号
      QVERIFY(enter0PressEvent.isAccepted()); // 不接受, 不会关闭按钮

      // focus不在lineEdit中, return关闭窗口
      cfgView->setFocus();
      QKeyEvent enter1PressEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
      cfgTbl.keyPressEvent(&enter1PressEvent); // 2nd
      QCOMPARE(acceptSpy.count(), 0);          // 不会accept
      QVERIFY(enter1PressEvent.isAccepted());  // 被接受, 关闭窗口

      QPushButton* pOkBtn = cfgTbl.m_dlgBtnBox->button(QDialogButtonBox::Open);
      QVERIFY(pOkBtn != nullptr);
      pOkBtn->clicked();              // 点击ok, 会关闭窗口
      QCOMPARE(acceptSpy.count(), 1); // accept
      acceptSpy.clear();
    }
  }

  void StartFilter_ok() {
    ConfigsMgr cfgTbl;

    QLineEdit* searchLe = cfgTbl.m_searchLineEdit;
    QAction* searchAct = cfgTbl.m_searchAction;
    QVERIFY(searchLe != nullptr);
    QVERIFY(searchAct != nullptr);

    searchLe->setText("Kaka");
    emit searchLe->returnPressed();

    searchLe->setText("Cristiano Ronaldo");
    searchAct->trigger();

    searchLe->setText("Varane");
    cfgTbl.onStartFilter();
  }
};

#include "ConfigsMgrTest.moc"
REGISTER_TEST(ConfigsMgrTest, false)
