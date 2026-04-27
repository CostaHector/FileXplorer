#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"

#include <QSignalSpy>

#include "Logger.h"
#include "SearchKey.h"
#include "Configuration.h"
#include "BeginToExposePrivateMember.h"
#include "SearchCaseMatterToolButton.h"
#include "EndToExposePrivateMember.h"

#include <QDir>
#include <QDirIterator>

Q_DECLARE_METATYPE(Qt::CaseSensitivity)

class SearchCaseMatterToolButtonTest : public PlainTestSuite {
  Q_OBJECT
private slots:
  void initTestCase() {
    qRegisterMetaType<Qt::CaseSensitivity>("Qt::CaseSensitivity");
    Configuration().clear();
  }
  void cleanupTestCase() {
    Configuration().clear();
  }

  void test_initialState() {
    SearchCaseMatterToolButton btn;
    QCOMPARE(SearchKey::NAME_CASE_SENSITIVE.v.data.b, false);
    QCOMPARE(SearchKey::CONTENTS_CASE_SENSITIVE.v.data.b, false);
    QCOMPARE(btn.curNameCaseSensitive(), Qt::CaseInsensitive);
    QCOMPARE(btn.curContentCaseSensitive(), Qt::CaseInsensitive);
  }

  void test_nameCaseSignal() {
    SearchCaseMatterToolButton btn;
    QSignalSpy spy(&btn, &SearchCaseMatterToolButton::nameCaseSensitiveChanged);
    btn.SEARCH_NAME_CASE_SENSITIVE->setChecked(true);
    btn.SEARCH_NAME_CASE_SENSITIVE->triggered(true);
    QTRY_COMPARE(spy.count(), 1);
    QVariantList nameCaseSentiveSignalParam = spy.last();
    QCOMPARE(nameCaseSentiveSignalParam.size(), 1); // only one parameter
    QCOMPARE(nameCaseSentiveSignalParam[0].value<Qt::CaseSensitivity>(), Qt::CaseSensitive);
    QCOMPARE(btn.curNameCaseSensitive(), Qt::CaseSensitive);
  }

  void test_contentCaseSignal() {
    SearchCaseMatterToolButton btn;
    QSignalSpy spy(&btn, &SearchCaseMatterToolButton::contentCaseSensitiveChanged);
    btn.SEARCH_CONTENTS_CASE_SENSITIVE->setChecked(false);
    btn.SEARCH_CONTENTS_CASE_SENSITIVE->triggered(false);
    QTRY_COMPARE(spy.count(), 1);
    QVariantList contentCaseSentiveSignalParam = spy.last();
    QCOMPARE(contentCaseSentiveSignalParam.size(), 1); // only one parameter
    QCOMPARE(contentCaseSentiveSignalParam[0].value<Qt::CaseSensitivity>(), Qt::CaseInsensitive);
    QCOMPARE(btn.curContentCaseSensitive(), Qt::CaseInsensitive);
  }

  void test_configurationPersistence() {
    {
      SearchCaseMatterToolButton btn;
      btn.SEARCH_NAME_CASE_SENSITIVE->setChecked(true);
      btn.SEARCH_CONTENTS_CASE_SENSITIVE->setChecked(true);
    } // btn destructor called. configure saved to local file

    QVERIFY(Configuration().contains(SearchKey::NAME_CASE_SENSITIVE.name));
    QVERIFY(Configuration().contains(SearchKey::CONTENTS_CASE_SENSITIVE.name));

    QCOMPARE(getConfig(SearchKey::NAME_CASE_SENSITIVE).toBool(), true);
    QCOMPARE(getConfig(SearchKey::CONTENTS_CASE_SENSITIVE).toBool(), true);

    // when initialize always use config from local file
    SearchCaseMatterToolButton newBtn;
    QCOMPARE(newBtn.curNameCaseSensitive(), Qt::CaseSensitive);
    QCOMPARE(newBtn.curContentCaseSensitive(), Qt::CaseSensitive);
  }
};

#include "SearchCaseMatterToolButtonTest.moc"
REGISTER_TEST(SearchCaseMatterToolButtonTest, false)
