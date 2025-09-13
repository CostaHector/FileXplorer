#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "TypeFilterButton.h"
#include "EndToExposePrivateMember.h"

#include <QDir>
#include <QDirIterator>

Q_DECLARE_METATYPE(QDir::Filters)
Q_DECLARE_METATYPE(QDirIterator::IteratorFlag)

class TypeFilterButtonTest : public PlainTestSuite {
  Q_OBJECT
public:
  TypeFilterButtonTest() : PlainTestSuite{} {
    LOG_D("TypeFilterButtonTest object created\n");
  }

  bool isInitialStateMatchConfigurationFile(const TypeFilterButton& btn) {
    const QString dirFilterKey = btn.mFilterHelper.GetMemoryKeyName(MemoryKey::DIR_FILTER_ON_SWITCH_ENABLE.name);
    const int expectInitialFilters = Configuration().value(dirFilterKey, MemoryKey::DIR_FILTER_ON_SWITCH_ENABLE.v).toInt();
    QDir::Filters actualDirFilters = btn.curDirFilters();
    const int actualDirFiltersValue = actualDirFilters.operator Int();
    if (actualDirFiltersValue != expectInitialFilters) {
      LOG_W("DirFilters dismatch. actual: %d, expect: %d", actualDirFiltersValue, expectInitialFilters);
      return false;
    }

    const QString grayOrHideKey = btn.mFilterHelper.GetMemoryKeyName(MemoryKey::GRAY_ENTRIES_DONT_PASS_FILTER.name);
    const bool expectGrayOrHideKey = Configuration().value(grayOrHideKey, MemoryKey::GRAY_ENTRIES_DONT_PASS_FILTER.v).toBool();
    const bool actualGrayOrHide = btn.curGrayOrHideUnpassItem();
    if (actualGrayOrHide != expectGrayOrHideKey) {
      LOG_W("GrayOrHide dismatch. actual: %d, expect: %d", actualGrayOrHide, expectGrayOrHideKey);
      return false;
    }

    const QString includeSubKey = btn.mFilterHelper.GetMemoryKeyName(MemoryKey::SEARCH_INCLUDING_SUBDIRECTORIES.name);
    const bool expectIncludeSub = Configuration().value(includeSubKey, MemoryKey::SEARCH_INCLUDING_SUBDIRECTORIES.v).toBool();
    const bool actualIncludeSub = btn.curIteratorFlag() == QDirIterator::Subdirectories;
    if (actualIncludeSub != expectIncludeSub) {
      LOG_W("IncludeSub dismatch. actual: %d, expect: %d", actualIncludeSub, expectIncludeSub);
      return false;
    }
    return true;
  }

private slots:
  void initTestCase() {
    qRegisterMetaType<QDir::Filters>("QDir::Filters");
    qRegisterMetaType<QDirIterator::IteratorFlag>("QDirIterator::IteratorFlag");

    Configuration().clear();
  }

  void cleanupTestCase() {
    Configuration().clear();
  }

  void initial_states_ok() {
    TypeFilterButton btnSearch(ModelFilterE::ADVANCE_SEARCH, nullptr);
    QVERIFY(btnSearch.mFilterHelper.INCLUDING_SUBDIRECTORIES != nullptr);
    QVERIFY(isInitialStateMatchConfigurationFile(btnSearch));

    TypeFilterButton btnFS(ModelFilterE::FILE_SYSTEM, nullptr);
    QVERIFY(btnFS.mFilterHelper.INCLUDING_SUBDIRECTORIES == nullptr);
    QVERIFY(isInitialStateMatchConfigurationFile(btnFS));
  }

  void configure_signals_emit_ok() {
    TypeFilterButton btnSearch(ModelFilterE::ADVANCE_SEARCH, nullptr);

    QSignalSpy spyFilterChanged(&btnSearch, &TypeFilterButton::filterChanged);
    QSignalSpy spyNameFilterDisables(&btnSearch, &TypeFilterButton::nameFilterDisablesChanged);
    QSignalSpy spyincludingSubdirectory(&btnSearch, &TypeFilterButton::includingSubdirectoryChanged);

    // 触发多个动作
    QVERIFY(btnSearch.mFilterHelper.clearAllDirFilterFlags());

    btnSearch.mFilterHelper.FILES->setChecked(true); // filterChanged 1
    emit btnSearch.mFilterHelper.FILES->triggered(true);

    btnSearch.mFilterHelper.HIDDEN->setChecked(true); // filterChanged 2
    emit btnSearch.mFilterHelper.HIDDEN->triggered(true);

    btnSearch.mFilterHelper.DIRS->setChecked(false); // filterChanged 2
    emit btnSearch.mFilterHelper.DIRS->triggered(false);

    btnSearch.mFilterHelper.GRAY_ENTRIES_DONT_PASS_FILTER->setChecked(false); // grayOrHide = false, nameFilterDisablesChanged
    emit btnSearch.mFilterHelper.GRAY_ENTRIES_DONT_PASS_FILTER->triggered(false);

    btnSearch.mFilterHelper.INCLUDING_SUBDIRECTORIES->setChecked(true); // includingSubdirectoryChanged=true
    emit btnSearch.mFilterHelper.INCLUDING_SUBDIRECTORIES->triggered(true);

    QCOMPARE(spyFilterChanged.count(), 3);
    QCOMPARE(spyNameFilterDisables.count(), 1);
    QCOMPARE(spyincludingSubdirectory.count(), 1);

    QDir::Filters expectFilters{QDir::Filter::Files, QDir::Filter::Hidden};
    QDir::Filters actualFilters = btnSearch.curDirFilters();
    QCOMPARE(actualFilters, expectFilters);
    QList<QVariant> lastSignalParams = spyFilterChanged.last();
    QCOMPARE(lastSignalParams.size(), 1);
    QVariant filterParm = lastSignalParams.front();
    QCOMPARE(filterParm.typeName(), "QDir::Filters");
    QCOMPARE(filterParm.value<QDir::Filters>(), expectFilters);

    QCOMPARE(btnSearch.curGrayOrHideUnpassItem(), false); // grayOrHide = false
    QCOMPARE(spyNameFilterDisables.last().front().toBool(), false);

    QCOMPARE(btnSearch.curIteratorFlag(), QDirIterator::IteratorFlag::Subdirectories); // true
    QList<QVariant> lastIncludeSubParams = spyincludingSubdirectory.last();
    QVariant includeParam = lastIncludeSubParams.front();
    QCOMPARE(includeParam.typeName(), "QDirIterator::IteratorFlag");
    QCOMPARE(includeParam.value<QDirIterator::IteratorFlag>(), QDirIterator::IteratorFlag::Subdirectories);
  }


  void configures_saved_seperately_in_destructor_ok() {
    Configuration().clear();
    QVERIFY(Configuration().allKeys().isEmpty());

    QString keyEndWith;
    {
      TypeFilterButton btnFS(ModelFilterE::FILE_SYSTEM, nullptr);
      keyEndWith = btnFS.mFilterHelper.c_str();
    } // force call destructor

    const QStringList onlyFskeys = Configuration().allKeys();
    QCOMPARE(onlyFskeys.size(), 2); // only 2 key, and not contains SEARCH_INCLUDING_SUBDIRECTORIES
    for (const QString& key: onlyFskeys) {
      QVERIFY(key.endsWith(keyEndWith));
      QVERIFY(!key.contains(MemoryKey::SEARCH_INCLUDING_SUBDIRECTORIES.name));
    }

    {
      TypeFilterButton btnSearch(ModelFilterE::ADVANCE_SEARCH, nullptr);
      keyEndWith = btnSearch.mFilterHelper.c_str();
    }
    const QStringList fsAndSearchkeys = Configuration().allKeys();
    QCOMPARE(fsAndSearchkeys.size(), 2 + 3); // onlyFskeys + onlySearchKeys
  }
};

#include "TypeFilterButtonTest.moc"
REGISTER_TEST(TypeFilterButtonTest, true)
