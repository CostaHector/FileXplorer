#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
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
    QCOMPARE(MemoryKey::SEARCH_NAME_CASE_SENSITIVE.v, false);
    QCOMPARE(MemoryKey::SEARCH_CONTENTS_CASE_SENSITIVE.v, false);
    QCOMPARE(btn.curNameCaseSensitive(), Qt::CaseInsensitive);
    QCOMPARE(btn.curContentCaseSensitive(), Qt::CaseInsensitive);
  }

  void test_nameCaseSignal() {
    SearchCaseMatterToolButton btn;
    QSignalSpy spy(&btn, &SearchCaseMatterToolButton::nameCaseSensitiveChanged);
    btn.SEARCH_NAME_CASE_SENSITIVE->setChecked(true);
    btn.SEARCH_NAME_CASE_SENSITIVE->triggered(true);
    QTRY_COMPARE(spy.count(), 1);
    QList<QVariant> nameCaseSentiveSignalParam = spy.last();
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
    QList<QVariant> contentCaseSentiveSignalParam = spy.last();
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

    QVERIFY(Configuration().contains(MemoryKey::SEARCH_NAME_CASE_SENSITIVE.name));
    QVERIFY(Configuration().contains(MemoryKey::SEARCH_CONTENTS_CASE_SENSITIVE.name));

    QCOMPARE(Configuration().value(MemoryKey::SEARCH_NAME_CASE_SENSITIVE.name).toBool(), true);
    QCOMPARE(Configuration().value(MemoryKey::SEARCH_CONTENTS_CASE_SENSITIVE.name).toBool(), true);

    // when initialize always use config from local file
    SearchCaseMatterToolButton newBtn;
    QCOMPARE(newBtn.curNameCaseSensitive(), Qt::CaseSensitive);
    QCOMPARE(newBtn.curContentCaseSensitive(), Qt::CaseSensitive);
  }
};

#include "SearchCaseMatterToolButtonTest.moc"
REGISTER_TEST(SearchCaseMatterToolButtonTest, false)
