#include <QtTest/QtTest>

#include "PlainTestSuite.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "AdvanceSearchToolBar.h"
#include "EndToExposePrivateMember.h"

#include "OnScopeExit.h"

#include <QDir>
#include <QDirIterator>

class AdvanceSearchToolBarTest : public PlainTestSuite {
  Q_OBJECT
 private slots:
  void initTestCase() {
    Configuration().clear();
    QWidget* parent = new (std::nothrow) QWidget;
    QVERIFY(parent != nullptr);
    ON_SCOPE_EXIT {
      delete parent;
      parent = nullptr;
    };
    auto* pTmp = new (std::nothrow) AdvanceSearchToolBar{"advance search toolbar", parent};
    QVERIFY(pTmp != nullptr);
  }

  void cleanupTestCase() { Configuration().clear(); }

  void test_initial_State() {
    // precondition
    Configuration().setValue(MemoryKey::ADVANCE_SEARCH_LINEEDIT_VALUE.name, "Name: hello word");
    Configuration().setValue(MemoryKey::ADVANCE_SEARCH_CONTENTS_LINEEDIT_VALUE.name, "Contents: henry");

    ModelFilterActions& inst = ModelFilterActions::GetInst(ModelFilterE::ADVANCE_SEARCH);
    inst.clearAllDirFilterFlags();
    inst.FILES->setChecked(true);
    inst.GRAY_ENTRIES_DONT_PASS_FILTER->setChecked(true);
    inst.INCLUDING_SUBDIRECTORIES->setChecked(true);

    Configuration().setValue(MemoryKey::ADVANCE_SEARCH_MODE.name, (int)SearchTools::SearchModeE::FILE_CONTENTS);
    Configuration().setValue(MemoryKey::SEARCH_NAME_CASE_SENSITIVE.name, true);
    Configuration().setValue(MemoryKey::SEARCH_CONTENTS_CASE_SENSITIVE.name, true);

    QWidget parent;
    AdvanceSearchToolBar advanceSearchToolbar{"advance search toolbar", &parent};
    QVERIFY(advanceSearchToolbar.m_nameFilterCB != nullptr);
    QVERIFY(advanceSearchToolbar.m_contentCB != nullptr);
    QVERIFY(advanceSearchToolbar.m_searchFilterButton != nullptr);
    QVERIFY(advanceSearchToolbar.m_searchModeBtn != nullptr);
    QVERIFY(advanceSearchToolbar.m_searchCaseButton != nullptr);
    QVERIFY(advanceSearchToolbar._searchProxyModel == nullptr);
    QVERIFY(advanceSearchToolbar._searchSourceModel == nullptr);
    // should not crash down
    {
      advanceSearchToolbar.onSearchTextChanges();
      advanceSearchToolbar.onSearchEnterAndApply();
      advanceSearchToolbar.BindSearchProxyModel(nullptr);
      advanceSearchToolbar.BindSearchSourceModel(nullptr);
      advanceSearchToolbar.onSearchModeChanged(SearchTools::SearchModeE::FILE_CONTENTS);
    }

    AdvanceSearchModel searchSourceModel;
    SearchProxyModel searchProxyModel;
    searchProxyModel.setSourceModel(&searchSourceModel);
    advanceSearchToolbar.BindSearchAllModel(&searchProxyModel, &searchSourceModel);
    advanceSearchToolbar.BindSearchAllModel(&searchProxyModel, &searchSourceModel);  // rebind
    // all initial state should set ok right after bind
    QCOMPARE(searchSourceModel.m_filters, QDir::Filter::Files);
    QCOMPARE(searchSourceModel.m_iteratorFlags, QDirIterator::IteratorFlag::Subdirectories);
    QCOMPARE(searchProxyModel.m_nameFilterDisableOrHide, true);

    QCOMPARE(searchProxyModel.m_searchMode, SearchTools::SearchModeE::FILE_CONTENTS);
    QCOMPARE(searchProxyModel.m_nameFiltersCaseSensitive, Qt::CaseSensitivity::CaseSensitive);
    QCOMPARE(searchProxyModel.m_fileContentsCaseSensitive, Qt::CaseSensitivity::CaseSensitive);

    advanceSearchToolbar.onSearchModeChanged(SearchTools::SearchModeE::NORMAL);
    QCOMPARE(searchProxyModel.m_searchMode, SearchTools::SearchModeE::NORMAL);
    advanceSearchToolbar.onSearchModeChanged(SearchTools::SearchModeE::FILE_CONTENTS);
    QCOMPARE(searchProxyModel.m_searchMode, SearchTools::SearchModeE::FILE_CONTENTS);
    advanceSearchToolbar.onSearchEnterAndApply();
  }

  void comboBoxCompleter_case_sensitive() {
    Configuration().setValue(MemoryKey::ADVANCE_SEARCH_MODE.name, (int)SearchTools::SearchModeE::FILE_CONTENTS);

    QWidget parent;
    AdvanceSearchToolBar advanceSearchToolbar{"advance search toolbar", &parent};
    QCOMPARE(advanceSearchToolbar.m_searchModeBtn->isEnabled(), true);

    QList<QComboBox*> comboxList{advanceSearchToolbar.m_nameFilterCB, advanceSearchToolbar.m_contentCB};
    for (QComboBox* comboBox : comboxList) {
      comboBox->setCurrentText("abc");
      QTest::keyClick(comboBox, Qt::Key_Enter);

      comboBox->setCurrentText("ABC");
      QTest::keyClick(comboBox, Qt::Key_Enter);

      QCOMPARE(comboBox->currentText(), QString("ABC"));
      QCOMPARE(comboBox->itemText(0), QString("ABC"));  // AtTop
      QCOMPARE(comboBox->itemText(1), QString("abc"));
    }
  }
};

#include "AdvanceSearchToolBarTest.moc"
REGISTER_TEST(AdvanceSearchToolBarTest, false)
