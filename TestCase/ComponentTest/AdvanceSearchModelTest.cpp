#include <QtTest/QtTest>
#include <QTestEventList>
#include "PlainTestSuite.h"

#include "MemoryKey.h"
#include "TDir.h"
#include "BeginToExposePrivateMember.h"
#include "AdvanceSearchModel.h"
#include "SearchProxyModel.h"
#include "EndToExposePrivateMember.h"

class AdvanceSearchModelTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir tDir;
  const QString pathRoot5{tDir.path()};
  const QString pathSub1{tDir.itemPath("Cristiano Ronaldo")};
  const QList<FsNodeEntry> nodeEntries  //
      {
          FsNodeEntry{"Cristiano Ronaldo/Kaka.txt", false, "My Eyes Never Lie.txt"},                  //
          FsNodeEntry{"Cristiano Ronaldo vs Goalkeeper record.txt", false, "Cristiano Ronaldo.txt"},  //
          FsNodeEntry{"Cristiano Ronaldo Tuxedo.svg", false, "Cristiano Ronaldo.svg"},                //
          FsNodeEntry{"Cristiano Ronaldo Wallpaper.jpg", false, "Wallpaper.jpg"},                     //
          FsNodeEntry{"Cristiano Ronaldo Football.jpg", false, "Football.jpg"},                       //
      };
  const QDir::Filters DEFAULT_DIR_FILTERS{QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot};

 private slots:
  void initTestCase() {
    QVERIFY(tDir.IsValid());
    QCOMPARE(tDir.createEntries(nodeEntries), nodeEntries.size());
  }

  void test_initial_states_ok() {
    // precondition:
    QVERIFY(pathRoot5.count('/') >= 2);  // at least 3 slash char

    AdvanceSearchModel sourceModel;
    sourceModel.initFilter(DEFAULT_DIR_FILTERS);
    sourceModel.initIteratorFlag(QDirIterator::IteratorFlag::Subdirectories);

    // 0. empty path/inexist path skip
    sourceModel.setRootPath("");
    sourceModel.setRootPath("path/to/inexist");
    QCOMPARE(sourceModel.rowCount(), 0);

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

    // will not crash down
    {
      sourceModel.appendDisable(QModelIndex());
      sourceModel.appendDisable(QModelIndex());
      sourceModel.removeDisable(QModelIndex());
      sourceModel.removeDisable(QModelIndex());
      QVERIFY(sourceModel.m_disableList.isEmpty());

      QModelIndex firstInd = sourceModel.index(0, 0);
      QVERIFY(firstInd.isValid());
      sourceModel.removeDisable(firstInd);
      QCOMPARE(sourceModel.m_disableList.size(), 0);

      sourceModel.appendDisable(firstInd);
      sourceModel.appendDisable(firstInd);
      QCOMPARE(sourceModel.m_disableList.size(), 1);

      sourceModel.removeDisable(firstInd);
      QCOMPARE(sourceModel.m_disableList.size(), 0);
      sourceModel.removeDisable(firstInd);
    }
  }

  void test_force_refresh_ok() {
    // precondition:
    QVERIFY(pathRoot5.count('/') >= 2);  // at least 3 slash char
    AdvanceSearchModel sourceModel;
    sourceModel.initFilter(DEFAULT_DIR_FILTERS);
    sourceModel.initIteratorFlag(QDirIterator::IteratorFlag::NoIteratorFlags);

    QCOMPARE(sourceModel.checkPathNeed("inexist path"), false);
    QCOMPARE(sourceModel.checkPathNeed(""), false);
    QCOMPARE(sourceModel.rowCount(), 0);
    sourceModel.m_rootPath = pathRoot5;
    sourceModel.setRootPath(pathRoot5);  // already here, skip into this path
    QCOMPARE(sourceModel.rowCount(), 0);
    sourceModel.forceRefresh();
    QCOMPARE(sourceModel.rowCount(), 5);  // forece into this path. no iterator
  }

  void test_search_name_normal_only() {
    AdvanceSearchModel sourceModel;
    sourceModel.initFilter(DEFAULT_DIR_FILTERS);
    sourceModel.initIteratorFlag(QDirIterator::IteratorFlag::Subdirectories);
    sourceModel.setRootPath(pathRoot5);
    QCOMPARE(sourceModel.rowCount(), 5 + 1);

    SearchProxyModel searchProxyModel;
    searchProxyModel.setSourceModel(&sourceModel);

    searchProxyModel.initSearchMode(SearchTools::SearchModeE::NORMAL);
    searchProxyModel.initNameFilterDisables(false);  // bDisableOrHide: false (Hide directly)
    // 1. case sensitive
    searchProxyModel.initFileNameFiltersCaseSensitive(Qt::CaseSensitivity::CaseSensitive);
    searchProxyModel.initFileContentsCaseSensitive(Qt::CaseSensitivity::CaseSensitive);

    searchProxyModel.startFilterWhenTextChanged("jpg", "no use content string now");  //
    QCOMPARE(searchProxyModel.rowCount(), 2);

    searchProxyModel.startFilterWhenTextChanged("svg", "no use content string now");  //
    QCOMPARE(searchProxyModel.rowCount(), 1);

    searchProxyModel.startFilterWhenTextChanged("Cristiano Ronaldo", "no use content string now");  //
    QCOMPARE(searchProxyModel.rowCount(), 5);

    searchProxyModel.startFilterWhenTextChanged("Kaka", "no use content string now");  //
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
    QCOMPARE(sourceModel.rowCount(), 5 + 1);

    SearchProxyModel searchProxyModel;
    searchProxyModel.setSourceModel(&sourceModel);

    searchProxyModel.initSearchMode(SearchTools::SearchModeE::REGEX);
    searchProxyModel.initNameFilterDisables(false);  // bDisableOrHide: false (Hide directly)
    // 1. case sensitive
    searchProxyModel.initFileNameFiltersCaseSensitive(Qt::CaseSensitivity::CaseSensitive);
    searchProxyModel.initFileContentsCaseSensitive(Qt::CaseSensitivity::CaseSensitive);

    searchProxyModel.startFilterWhenTextChanged("g$", "no use content string now");  //
    QCOMPARE(searchProxyModel.rowCount(), 3);                                        // 2xjpg and 1xsvg
    searchProxyModel.startFilterWhenTextChanged("G$", "no use content string now");  //
    QCOMPARE(searchProxyModel.rowCount(), 0);

    // 2. case insensitive
    searchProxyModel.initFileNameFiltersCaseSensitive(Qt::CaseSensitivity::CaseInsensitive);
    searchProxyModel.initFileContentsCaseSensitive(Qt::CaseSensitivity::CaseInsensitive);
    searchProxyModel.startFilterWhenTextChanged("^c", "no use content string now");
    QCOMPARE(searchProxyModel.rowCount(), 5);  // 5xCristiano
  }

  void test_search_name_regex_and_contents_contains_only() {
    AdvanceSearchModel sourceModel;
    sourceModel.initFilter(DEFAULT_DIR_FILTERS);
    sourceModel.initIteratorFlag(QDirIterator::IteratorFlag::Subdirectories);
    sourceModel.setRootPath(pathRoot5);
    QCOMPARE(sourceModel.rowCount(), 5 + 1);  // and 1 under subfolder

    SearchProxyModel searchProxyModel;
    searchProxyModel.setSourceModel(&sourceModel);

    searchProxyModel.initSearchMode(SearchTools::SearchModeE::FILE_CONTENTS);
    searchProxyModel.initNameFilterDisables(false);  // bDisableOrHide: false (Hide directly)
    // 1. name regex, contents contain, case sensitive
    searchProxyModel.initFileNameFiltersCaseSensitive(Qt::CaseSensitivity::CaseSensitive);
    searchProxyModel.initFileContentsCaseSensitive(Qt::CaseSensitivity::CaseSensitive);

    searchProxyModel.startFilterWhenTextChanged("g$", "Cristiano Ronaldo");  //
    QCOMPARE(searchProxyModel.rowCount(), 1);                                // 2xjpg and 1xsvg | then only 1
    searchProxyModel.startFilterWhenTextChanged("G$", "Cristiano Ronaldo");  //
    QCOMPARE(searchProxyModel.rowCount(), 0);                                // 0
    searchProxyModel.PrintRegexDebugMessage();

    // 2. name regex, contents contain, case insensitive
    searchProxyModel.initFileNameFiltersCaseSensitive(Qt::CaseSensitivity::CaseInsensitive);
    searchProxyModel.initFileContentsCaseSensitive(Qt::CaseSensitivity::CaseInsensitive);
    searchProxyModel.startFilterWhenTextChanged("^c", "TXT");
    QCOMPARE(searchProxyModel.rowCount(), 1);  // 5xCristiano| then only "Kaka.txt"
  }

  void source_model_data_retrieve_ok() {
    AdvanceSearchModel sourceModel;
    using namespace FilePropertyHelper;
    {  // when root path not set
      QCOMPARE(sourceModel.columnCount(), SEARCH_TABLE_HEADERS_COUNT);
      QCOMPARE(sourceModel.rowCount(), 0);
      QVERIFY(SEARCH_TABLE_HEADERS_COUNT >= 2);

      QCOMPARE(sourceModel.headerData(PropColumnE::Name, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole).toString(),
               SEARCH_TABLE_HEADERS[PropColumnE::Name]);
      QCOMPARE(sourceModel.headerData(PropColumnE::Size, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole).toString(),
               SEARCH_TABLE_HEADERS[PropColumnE::Size]);
      sourceModel.headerData(1999, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole);  // will not crash down

      QCOMPARE(sourceModel.headerData(0, Qt::Orientation::Vertical, Qt::ItemDataRole::DisplayRole).toInt(), 0 + 1);
      sourceModel.headerData(999, Qt::Orientation::Vertical, Qt::ItemDataRole::DisplayRole).toInt();  // will not crash down

      QCOMPARE(sourceModel.headerData(0, Qt::Orientation::Vertical, Qt::ItemDataRole::TextAlignmentRole).toInt(), (int)Qt::AlignRight);

      QCOMPARE(sourceModel.data(QModelIndex{}, Qt::ItemDataRole::DisplayRole).isNull(), true);
    }

    sourceModel.initFilter(DEFAULT_DIR_FILTERS);
    sourceModel.initIteratorFlag(QDirIterator::IteratorFlag::NoIteratorFlags);
    sourceModel.setRootPath(pathRoot5);
    QCOMPARE(sourceModel.rowCount(), 5);

    const QModelIndex firstInd = sourceModel.index(0, PropColumnE::Name);
    const QModelIndex secondInd = sourceModel.index(1, PropColumnE::Name);

    QCOMPARE(sourceModel.fileName(firstInd), "Cristiano Ronaldo");
    QCOMPARE(sourceModel.data(firstInd, Qt::ItemDataRole::DisplayRole).toString(), "Cristiano Ronaldo");
    QCOMPARE(sourceModel.fileName(secondInd), "Cristiano Ronaldo vs Goalkeeper record.txt");
    QCOMPARE(sourceModel.data(secondInd, Qt::ItemDataRole::DisplayRole).toString(), "Cristiano Ronaldo vs Goalkeeper record.txt");
    QVERIFY(sourceModel.fileInfo(secondInd).isFile());

    QString firstFileAbsFilePath = sourceModel.m_rootPath +
                                   sourceModel.data(sourceModel.index(0, PropColumnE::RelPath), Qt::ItemDataRole::DisplayRole).toString() +
                                   "Cristiano Ronaldo";
    QCOMPARE(firstFileAbsFilePath, tDir.itemPath("Cristiano Ronaldo"));
    QCOMPARE(sourceModel.filePath(firstInd), tDir.itemPath("Cristiano Ronaldo"));

    const QString actualAbsPath = sourceModel.absolutePath(firstInd);
    QCOMPARE(actualAbsPath + "Cristiano Ronaldo", tDir.itemPath("Cristiano Ronaldo"));

    QCOMPARE(sourceModel.fullInfo(firstInd).contains("Cristiano Ronaldo"), true);
    QFileInfo fi(sourceModel.fileInfo(firstInd));
    QCOMPARE(fi.absoluteFilePath(), tDir.itemPath("Cristiano Ronaldo"));

    QCOMPARE(sourceModel.data(firstInd, Qt::ItemDataRole::DecorationRole).isValid(), true);  // folder from icon provider
    QCOMPARE(sourceModel.data(secondInd, Qt::ItemDataRole::DecorationRole).isValid(), true); // from QIconProvider
    QCOMPARE(sourceModel.data(secondInd, Qt::ItemDataRole::DecorationRole).isValid(), true); // from pixmapcache ok

    sourceModel.CutSomething({firstInd});
    sourceModel.data(firstInd, Qt::ItemDataRole::DecorationRole);  // from inexist svg file
    sourceModel.CopiedSomething({firstInd});
    sourceModel.data(firstInd, Qt::ItemDataRole::DecorationRole);
    QCOMPARE(sourceModel.data(firstInd.siblingAtColumn(PropColumnE::Size), Qt::ItemDataRole::TextAlignmentRole).toInt(), (int)Qt::AlignRight);
    QCOMPARE(sourceModel.data(firstInd.siblingAtColumn(PropColumnE::Name), Qt::ItemDataRole::TextAlignmentRole).toInt(), int(Qt::AlignLeft | Qt::AlignTop));

    QVERIFY(sourceModel.m_disableList.isEmpty());
    QVERIFY(sourceModel.m_recycleSet.isEmpty());
    QVERIFY(sourceModel.data(firstInd, Qt::ItemDataRole::ForegroundRole).isNull());  // should be null

    {  // iteratorFlag=Subdirectories, files only
      sourceModel.setFilter(QDir::Filter::Files);
      sourceModel.setIteratorFlag(QDirIterator::IteratorFlag::Subdirectories);
      QCOMPARE(sourceModel.rowCount(), 5);
      QModelIndex firstIndexKaka = sourceModel.index(0, 0);
      QCOMPARE(sourceModel.fileName(firstIndexKaka), "Kaka.txt");
      const QString kakaTxtFileAbsFilePath = sourceModel.filePath(firstIndexKaka);
      QCOMPARE(kakaTxtFileAbsFilePath, tDir.itemPath("Cristiano Ronaldo/Kaka.txt"));
    }

    {  // invalid index should not crash down
      QModelIndex invalidIndex;
      sourceModel.fileInfo(invalidIndex);
      QCOMPARE(sourceModel.fileName(invalidIndex), "");
      QCOMPARE(sourceModel.filePath(invalidIndex), "");
      QCOMPARE(sourceModel.absolutePath(invalidIndex), "");
    }
  }

  void mix_model_recycleSomething_proxy_disable_somthing_ok() {
    AdvanceSearchModel sourceModel;
    sourceModel.initFilter(DEFAULT_DIR_FILTERS);
    sourceModel.initIteratorFlag(QDirIterator::IteratorFlag::NoIteratorFlags);
    sourceModel.setRootPath(pathRoot5);
    QCOMPARE(sourceModel.rowCount(), 5);

    SearchProxyModel searchProxyModel;
    searchProxyModel.setSourceModel(nullptr);  // not crash down
    searchProxyModel.setSourceModel(&sourceModel);
    searchProxyModel.setFilterKeyColumn(FilePropertyHelper::Name);

    searchProxyModel.initNameFilterDisables(false);  // hide
    searchProxyModel.initSearchMode(SearchTools::SearchModeE::FILE_CONTENTS);

    const QModelIndex firstIndex = sourceModel.index(0, FilePropertyHelper::Name);

    {  // 1. recycle => red
      QVERIFY(sourceModel.m_disableList.isEmpty());
      QVERIFY(sourceModel.m_recycleSet.isEmpty());
      sourceModel.RecycleSomething({firstIndex});
      QCOMPARE(sourceModel.m_recycleSet.isEmpty(), false);

      QVariant recycleColorVar = sourceModel.data(firstIndex, Qt::ItemDataRole::ForegroundRole);
      QVERIFY(recycleColorVar.isValid());
      QVERIFY(recycleColorVar.canConvert<QColor>());
      QCOMPARE(recycleColorVar.value<QColor>(), QColor(Qt::GlobalColor::red));

      sourceModel.ClearRecycle();
      QCOMPARE(sourceModel.m_recycleSet.isEmpty(), true);
    }

    {  // 2. disable => gray
      searchProxyModel.setSearchMode(SearchTools::SearchModeE::NORMAL);
      searchProxyModel.setNameFilterDisables(true);  // gray

      searchProxyModel.setFileContentsCaseSensitive(Qt::CaseSensitivity::CaseInsensitive);
      searchProxyModel.setFileContentsCaseSensitive(Qt::CaseSensitivity::CaseSensitive);

      searchProxyModel.setFileNameFiltersCaseSensitive(Qt::CaseSensitivity::CaseInsensitive);
      searchProxyModel.setFileNameFiltersCaseSensitive(Qt::CaseSensitivity::CaseSensitive);

      QVERIFY(sourceModel.m_disableList.isEmpty());
      searchProxyModel.startFilterWhenTextChanged("not exist name", "");
      QCOMPARE(sourceModel.m_disableList.size(), 5);

      QCOMPARE(sourceModel.rowCount(), 5);       //
      QCOMPARE(searchProxyModel.rowCount(), 5);  // all items grayed

      QVariant disgrayedColorVar = sourceModel.data(firstIndex, Qt::ItemDataRole::ForegroundRole);
      QVERIFY(disgrayedColorVar.isValid());
      QVERIFY(disgrayedColorVar.canConvert<QColor>());
      QCOMPARE(disgrayedColorVar.value<QColor>(), QColor(Qt::GlobalColor::lightGray));

      sourceModel.clearDisables();
    }
  }
};

#include "AdvanceSearchModelTest.moc"
REGISTER_TEST(AdvanceSearchModelTest, false)
