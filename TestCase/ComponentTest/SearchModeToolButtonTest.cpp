#include <QtTest/QtTest>
#include <QTestEventList>
#include <QSignalSpy>

#include "PlainTestSuite.h"
#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "SearchModeToolButton.h"
#include "EndToExposePrivateMember.h"

#include <QDir>
#include <QDirIterator>

Q_DECLARE_METATYPE(SearchTools::SearchModeE)

class SearchModeToolButtonTest : public PlainTestSuite {
  Q_OBJECT
private slots:
  void initTestCase() {
    qRegisterMetaType<SearchTools::SearchModeE>("SearchTools::SearchModeE");
    Configuration().clear();
  }

  void cleanupTestCase() {
    Configuration().clear();
  }

  void test_initialState() {
    // precondition
    QCOMPARE(MemoryKey::ADVANCE_SEARCH_MODE.v.toInt(), ((int)SearchTools::SearchModeE::NORMAL));

    SearchModeToolButton btn;
    QCOMPARE(btn.curSearchMode(), SearchTools::SearchModeE::NORMAL);
    QVERIFY(btn.MATCH_EQUAL != nullptr);
    QVERIFY(btn.MATCH_REGEX != nullptr);
    QVERIFY(btn.SEARCH_SCOPE_CONTENT != nullptr);
  }

  void test_modeChangeSignals() {
    SearchModeToolButton btn;
    QSignalSpy spy(&btn, &SearchModeToolButton::searchModeChanged);

    // simulate regex action triggered, the connected signal searchModeChanged also need triggered
    QAction* pRegexAct = btn.mSearchModeIntAction.setCheckedIfActionExist(SearchTools::SearchModeE::REGEX);
    QVERIFY(pRegexAct != nullptr);
    QVERIFY(pRegexAct->isChecked());
    QCOMPARE(pRegexAct, btn.MATCH_REGEX);
    emit pRegexAct->triggered(true);

    QTRY_COMPARE(spy.count(), 1);
    QList<QVariant> searchModeChangeParams = spy.last();
    QCOMPARE(searchModeChangeParams.size(), 1); // only 1 paramerter
    QCOMPARE(searchModeChangeParams[0].value<SearchTools::SearchModeE>(), SearchTools::SearchModeE::REGEX);
    QCOMPARE(btn.curSearchMode(), SearchTools::SearchModeE::REGEX);
  }

  void test_configurationPersistence() {
    {
      SearchModeToolButton btn;
      auto* pContentAct = btn.mSearchModeIntAction.setCheckedIfActionExist(SearchTools::SearchModeE::FILE_CONTENTS);
      QCOMPARE(pContentAct, btn.SEARCH_SCOPE_CONTENT);
      btn.EmitSearchModeChanged(pContentAct);
    } // cofigure saved to local file when destruct called

    QVERIFY(Configuration().contains(MemoryKey::ADVANCE_SEARCH_MODE.name));
    QCOMPARE(Configuration().value(MemoryKey::ADVANCE_SEARCH_MODE.name).toInt(),
             ((int)SearchTools::SearchModeE::FILE_CONTENTS));

    SearchModeToolButton newBtn;
    QCOMPARE(newBtn.curSearchMode(), SearchTools::SearchModeE::FILE_CONTENTS);
  }
};

#include "SearchModeToolButtonTest.moc"
REGISTER_TEST(SearchModeToolButtonTest, false)
