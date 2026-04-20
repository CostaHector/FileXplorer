#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"

#include "Logger.h"
#include "BeginToExposePrivateMember.h"
#include "StyleSheetMgr.h"
#include "EndToExposePrivateMember.h"

#include "StyleSheetGetter.h"
#include "PreferenceActions.h"
#include <QPushButton>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class StyleSheetMgrTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:

  void init() { GlobalMockObject::reset(); }
  void cleanup() { GlobalMockObject::verify(); }

  void search_ok() {
    StyleSheetMgr mgr;
    mgr.showEvent(nullptr);

    QShowEvent eventShow;
    mgr.showEvent(&eventShow);

    QVERIFY(mgr.m_searchLineEdit != nullptr);
    QVERIFY(mgr.m_startSearchAct != nullptr);
    QVERIFY(mgr.m_styleSheetView != nullptr);

    mgr.m_searchLineEdit->setText("Kaka");
    emit mgr.m_searchLineEdit->returnPressed();
    QCOMPARE(mgr.m_styleSheetView->curFilter(), "Kaka");

    mgr.m_searchLineEdit->setText("Cristinao Ronaldo");
    mgr.m_startSearchAct->trigger();
    QCOMPARE(mgr.m_styleSheetView->curFilter(), "Cristinao Ronaldo");

    mgr.m_searchLineEdit->setText("Varane");
    mgr.onStartFilter();
    QCOMPARE(mgr.m_styleSheetView->curFilter(), "Varane");
  }

  void only_accept_will_callWriteIntoSettings_ok() {
    MOCKER(StyleSheetGetter::WriteIntoSettingsCore).expects(exactly(1)).id("writeQSetting");
    MOCKER(PreferenceActions::ApplyNewStyleSheet).expects(exactly(1)).after("writeQSetting");

    StyleSheetMgr mgr;
    QVERIFY(mgr.m_dlgBtnBox != nullptr);
    QPushButton* applyBtn = mgr.m_dlgBtnBox->button(QDialogButtonBox::Apply);
    QPushButton* cancelBtn = mgr.m_dlgBtnBox->button(QDialogButtonBox::Cancel);
    QVERIFY(applyBtn != nullptr);
    QVERIFY(cancelBtn != nullptr);

    cancelBtn->click(); // will not call WriteIntoSettingsCore

    applyBtn->click(); // call WriteIntoSettingsCore, and call ApplyNewStyleSheet
  }
};

#include "StyleSheetMgrTest.moc"
REGISTER_TEST(StyleSheetMgrTest, false)
