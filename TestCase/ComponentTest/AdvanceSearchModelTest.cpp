#include <QtTest/QtTest>
#include <QTestEventList>
#include "PlainTestSuite.h"

#include "Logger.h"
#include "MemoryKey.h"
#include "TDir.h"
#include "BeginToExposePrivateMember.h"
#include "AdvanceSearchModel.h"
#include "SearchProxyModel.h"
#include "EndToExposePrivateMember.h"

class AdvanceSearchModelTest : public PlainTestSuite {
  Q_OBJECT
public:
  AdvanceSearchModelTest() : PlainTestSuite{} {
    LOG_D("AdvanceSearchModelTest object created\n");
  }

  TDir tDir;
  QDir mDir;
  const QList<FsNodeEntry> nodeEntries//
      {
          FsNodeEntry{"Cristiano Ronaldo/Kaka.txt",                 false, "My Eyes Never Lie.txt"}, //
          FsNodeEntry{"Cristiano Ronaldo vs Goalkeeper record.txt", false, "Cristiano Ronaldo.txt"}, //
          FsNodeEntry{"Cristiano Ronaldo Tuxedo.svg",               false, "Cristiano Ronaldo.svg"}, //
          FsNodeEntry{"Cristiano Ronaldo Wallpaper.jpg",            false, "Wallpaper.jpg"}, //
          FsNodeEntry{"Cristiano Ronaldo Football.jpg",             false, "Football.jpg"}, //
      };
  QString pathRoot5, pathSub1;
  const QDir::Filters DEFAULT_DIR_FILTERS{QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot};

private slots:
  void initTestCase() {
    QVERIFY(tDir.IsValid());
    pathRoot5 = tDir.path();
    QCOMPARE(tDir.createEntries(nodeEntries), nodeEntries.size());

    mDir = QDir{pathRoot5};
    QVERIFY(mDir.exists());
    pathSub1 = mDir.absoluteFilePath("Cristiano Ronaldo");
  }

  void test_initial_states_ok () {
    // precondition:
    QVERIFY(pathRoot5.count('/') >= 2); // at least 3 slash char

    AdvanceSearchModel sourceModel;
    sourceModel.initFilter(DEFAULT_DIR_FILTERS);
    sourceModel.initIteratorFlag(QDirIterator::IteratorFlag::Subdirectories);

    // 1. in root directory
    sourceModel.setRootPath(pathRoot5);
    QCOMPARE(sourceModel.m_rootPath, pathRoot5);

    // iterator, Files|Dirs|NoDotAndDotDot
    QCOMPARE(sourceModel.m_itemsLst.size(), 5 + 1);
    QCOMPARE(sourceModel.rowCount(), 5 + 1);
    QCOMPARE(sourceModel.m_filters, DEFAULT_DIR_FILTERS);
    QCOMPARE(sourceModel.m_iteratorFlags, QDirIterator::IteratorFlag::Subdirectories);

    // iterator, Files
    sourceModel.setFilter(QDir::Filter::Files);
    QCOMPARE(sourceModel.m_itemsLst.size(), 5);
    QCOMPARE(sourceModel.rowCount(), 5);
    QCOMPARE(sourceModel.m_filters, QDir::Filter::Files);
    QCOMPARE(sourceModel.m_iteratorFlags, QDirIterator::IteratorFlag::Subdirectories);

    // no iterator, File
    sourceModel.setIteratorFlag(QDirIterator::IteratorFlag::NoIteratorFlags);
    QCOMPARE(sourceModel.m_itemsLst.size(), 4);
    QCOMPARE(sourceModel.rowCount(), 4);
    QCOMPARE(sourceModel.m_filters, QDir::Filter::Files);
    QCOMPARE(sourceModel.m_iteratorFlags, QDirIterator::IteratorFlag::NoIteratorFlags);

    // no iterator, Dir|NotDotAndDotDot
    sourceModel.setFilter(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot);
    QCOMPARE(sourceModel.m_itemsLst.size(), 1);
    QCOMPARE(sourceModel.rowCount(), 1);
    QCOMPARE(sourceModel.m_filters, QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot);
    QCOMPARE(sourceModel.m_iteratorFlags, QDirIterator::IteratorFlag::NoIteratorFlags);


    // 2. in subdirectory
    sourceModel.setRootPath(pathSub1);
    // no iterator, Dir|NotDotAndDotDot
    QCOMPARE(sourceModel.m_rootPath, pathSub1);
    QCOMPARE(sourceModel.m_itemsLst.size(), 0);
    // no iterator, Files
    sourceModel.setFilter(QDir::Filter::Files);
    QCOMPARE(sourceModel.m_itemsLst.size(), 1);
  }

  void test_force_refresh_ok() {
    // precondition:
    QVERIFY(pathRoot5.count('/') >= 2); // at least 3 slash char
    AdvanceSearchModel sourceModel;
    sourceModel.initFilter(DEFAULT_DIR_FILTERS);
    sourceModel.initIteratorFlag(QDirIterator::IteratorFlag::NoIteratorFlags);

    QCOMPARE(sourceModel.checkPathNeed("inexist path"), false);
    QCOMPARE(sourceModel.checkPathNeed(""), false);
    QCOMPARE(sourceModel.rowCount(), 0);
    sourceModel.m_rootPath = pathRoot5;
    sourceModel.setRootPath(pathRoot5); // already here, skip into this path
    QCOMPARE(sourceModel.rowCount(), 0);
    sourceModel.forceRefresh();
    QCOMPARE(sourceModel.rowCount(), 5); // forece into this path. no iterator
  }

  void test_search_name_normal_only() {
    AdvanceSearchModel sourceModel;
    sourceModel.initFilter(DEFAULT_DIR_FILTERS);
    sourceModel.initIteratorFlag(QDirIterator::IteratorFlag::Subdirectories);
    sourceModel.setRootPath(pathRoot5);
    QCOMPARE(sourceModel.rowCount(), 5+1);

    SearchProxyModel searchProxyModel;
    searchProxyModel.setSourceModel(&sourceModel);

    searchProxyModel.initSearchMode(SearchTools::SearchModeE::NORMAL);
    searchProxyModel.initNameFilterDisables(false); // bDisableOrHide: false (Hide directly)
    // 1. case sensitive
    searchProxyModel.initFileNameFiltersCaseSensitive(Qt::CaseSensitivity::CaseSensitive);
    searchProxyModel.initFileContentsCaseSensitive(Qt::CaseSensitivity::CaseSensitive);

    searchProxyModel.startFilterWhenTextChanged("jpg", "no use content string now");//
    QCOMPARE(searchProxyModel.rowCount(), 2);

    searchProxyModel.startFilterWhenTextChanged("svg", "no use content string now");//
    QCOMPARE(searchProxyModel.rowCount(), 1);

    searchProxyModel.startFilterWhenTextChanged("Cristiano Ronaldo", "no use content string now");//
    QCOMPARE(searchProxyModel.rowCount(), 5);

    searchProxyModel.startFilterWhenTextChanged("Kaka", "no use content string now");//
    QCOMPARE(searchProxyModel.rowCount(), 1);

    searchProxyModel.startFilterWhenTextChanged("KAKA", "no use content string now");
    QCOMPARE(searchProxyModel.rowCount(), 0);

    searchProxyModel.startFilterWhenTextChanged("", "no use content string now");
    QCOMPARE(searchProxyModel.rowCount(), 6);

    // 2. case insensitive
    searchProxyModel.initFileNameFiltersCaseSensitive(Qt::CaseSensitivity::CaseInsensitive);
    searchProxyModel.initFileContentsCaseSensitive(Qt::CaseSensitivity::CaseInsensitive);
    searchProxyModel.startFilterWhenTextChanged("KAKA", "no use content string now");
    QCOMPARE(searchProxyModel.rowCount(), 1);
  }

  void test_search_name_regex_only() {
    AdvanceSearchModel sourceModel;
    sourceModel.initFilter(DEFAULT_DIR_FILTERS);
    sourceModel.initIteratorFlag(QDirIterator::IteratorFlag::Subdirectories);
    sourceModel.setRootPath(pathRoot5);
    QCOMPARE(sourceModel.rowCount(), 5+1);

    SearchProxyModel searchProxyModel;
    searchProxyModel.setSourceModel(&sourceModel);

    searchProxyModel.initSearchMode(SearchTools::SearchModeE::REGEX);
    searchProxyModel.initNameFilterDisables(false); // bDisableOrHide: false (Hide directly)
    // 1. case sensitive
    searchProxyModel.initFileNameFiltersCaseSensitive(Qt::CaseSensitivity::CaseSensitive);
    searchProxyModel.initFileContentsCaseSensitive(Qt::CaseSensitivity::CaseSensitive);

    searchProxyModel.startFilterWhenTextChanged("g$", "no use content string now");//
    QCOMPARE(searchProxyModel.rowCount(), 3); // 2xjpg and 1xsvg
    searchProxyModel.startFilterWhenTextChanged("G$", "no use content string now");//
    QCOMPARE(searchProxyModel.rowCount(), 0);

    // 2. case insensitive
    searchProxyModel.initFileNameFiltersCaseSensitive(Qt::CaseSensitivity::CaseInsensitive);
    searchProxyModel.initFileContentsCaseSensitive(Qt::CaseSensitivity::CaseInsensitive);
    searchProxyModel.startFilterWhenTextChanged("^c", "no use content string now");
    QCOMPARE(searchProxyModel.rowCount(), 5); // 5xCristiano
  }

  void test_search_name_regex_and_contents_contains_only() {
    AdvanceSearchModel sourceModel;
    sourceModel.initFilter(DEFAULT_DIR_FILTERS);
    sourceModel.initIteratorFlag(QDirIterator::IteratorFlag::Subdirectories);
    sourceModel.setRootPath(pathRoot5);
    QCOMPARE(sourceModel.rowCount(), 5+1);

    SearchProxyModel searchProxyModel;
    searchProxyModel.setSourceModel(&sourceModel);

    searchProxyModel.initSearchMode(SearchTools::SearchModeE::FILE_CONTENTS);
    searchProxyModel.initNameFilterDisables(false); // bDisableOrHide: false (Hide directly)
    // 1. name regex, contents contain, case sensitive
    searchProxyModel.initFileNameFiltersCaseSensitive(Qt::CaseSensitivity::CaseSensitive);
    searchProxyModel.initFileContentsCaseSensitive(Qt::CaseSensitivity::CaseSensitive);

    searchProxyModel.startFilterWhenTextChanged("g$", "Cristiano Ronaldo");//
    QCOMPARE(searchProxyModel.rowCount(), 1); // 2xjpg and 1xsvg | then only 1
    searchProxyModel.startFilterWhenTextChanged("G$", "Cristiano Ronaldo");//
    QCOMPARE(searchProxyModel.rowCount(), 0); // 0

    // 2. name regex, contents contain, case insensitive
    searchProxyModel.initFileNameFiltersCaseSensitive(Qt::CaseSensitivity::CaseInsensitive);
    searchProxyModel.initFileContentsCaseSensitive(Qt::CaseSensitivity::CaseInsensitive);
    searchProxyModel.startFilterWhenTextChanged("^c", "TXT");
    QCOMPARE(searchProxyModel.rowCount(), 1); // 5xCristiano| then only "Kaka.txt"
  }
};

#include "AdvanceSearchModelTest.moc"
REGISTER_TEST(AdvanceSearchModelTest, false)
