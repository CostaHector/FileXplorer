#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "CastDBViewSplitter.h"
#include "EndToExposePrivateMember.h"
#include "DatabaseSearchToolBar.h"
#include "InputDialogHelper.h"
#include "CastDBActions.h"

#include "TDir.h"
#include "PublicVariable.h"
#include "Configuration.h"
#include "CastKey.h"
#include "SqlRecordTestHelper.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class CastDBViewSplitterTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir mTDir;
  const QString imageHostFullPath{mTDir.itemPath("host")};
  const QString dbName{mTDir.itemPath("CastViewTest.db")};
  const QString connName{"ConnCastDBViewSplitterTest"};
  CastBaseDb castDb{dbName, connName};
  const QString tableName{DB_TABLE::PERFORMERS};
private slots:
  void initTestCase() {
    QVERIFY(mTDir.IsValid());

    QVERIFY(castDb.IsValid()); // 这里不要建表， 等splitter建立
    const QList<FsNodeEntry> nodes {
                                   FsNodeEntry{"host/Football/Kaka/Kaka 1.jpg", false, ""},  // 只有1个图片
                                   FsNodeEntry{"host/Football/Cristiano/Cristiano 1.json", false, ""}, // 没有图片
                                   };
    QCOMPARE(mTDir.createEntries(nodes), nodes.size());

  }

  void init() {  //
    Configuration().clear();
    CastDBActions::GetInst().m_showCastsCardsListAct->setChecked(false);
    GlobalMockObject::reset();
  }
  void cleanup() {  //
    CastDBActions::GetInst().m_showCastsCardsListAct->setChecked(false);
    GlobalMockObject::verify();
  }

  void default_ok() {
    QWidget wid;
    CastDatabaseSearchToolBar searchTb{"CastDbSearchBar", &wid};
    CastDBViewSplitter viewSplitter{dbName, connName, imageHostFullPath, &searchTb, &wid};

    QCOMPARE(viewSplitter.GetModel(), viewSplitter.m_castDbModel);
    QCOMPARE(viewSplitter.GetTableView(), viewSplitter.m_castTableView);
    QCOMPARE(viewSplitter.GetListView(), viewSplitter.m_castListView);

    QVERIFY(viewSplitter.m_castDb != nullptr);
    QVERIFY(viewSplitter.m_castDbModel != nullptr);
    QVERIFY(viewSplitter.m_sharedSelectionModel != nullptr);
    QVERIFY(viewSplitter.m_castTableView != nullptr);
    QVERIFY(viewSplitter.m_castListView == nullptr); // 没有Configs时, 必定为空指针
    QCOMPARE(viewSplitter.count(), 1); // 只加入了一个view

    QCOMPARE(viewSplitter.m_castDbModel->tableName(), "");
    // 基本上不需要InitDataBase; database.open()就创建好了
    QVERIFY(viewSplitter.onInitATable());
    QVERIFY(viewSplitter.onInitATable());
    QCOMPARE(viewSplitter.m_castDbModel->tableName(), tableName);
    QCOMPARE(viewSplitter.m_castDbModel->isDirty(), false);
    QCOMPARE(viewSplitter.m_castDbModel->rowCount(), 0);
    viewSplitter.m_castDbModel->setFilter("");
    QCOMPARE(viewSplitter.m_castDbModel->filter(), "");

    // 构造dirty场景
    {
      QSqlRecord neymarRecord = SqlRecordTestHelper::GetACastRecordLine("Neymar", "Football", ""/*No Imgs*/, ""/*No Vids*/);
      neymarRecord.setValue(CastDbModelField::Rate, 9);
      neymarRecord.setValue(CastDbModelField::Height, 180);
      QVERIFY(viewSplitter.m_castDbModel->insertRecord(0, neymarRecord));
      QCOMPARE(viewSplitter.m_castDbModel->rowCount(), 1);
      QCOMPARE(viewSplitter.m_castDbModel->isDirty(), true);
    }
    const QSqlRecord before0RowRecord{viewSplitter.m_castDbModel->record(0)};

    MOCKER(InputDialogHelper::YesOrCancelBox).expects(exactly(6))
        .will(returnValue(false)).then(returnValue(true))
        .then(returnValue(false)).then(returnValue(true))
        .then(returnValue(false)).then(returnValue(true));
    // dirty下时, 涉及select操作, 一律禁用
    {
      QCOMPARE(viewSplitter.m_castDbModel->isDirty(), true);
      QCOMPARE(viewSplitter.onLoadFromPsonDirectory(), 0); // dirty 时拒绝载入
      QCOMPARE(viewSplitter.onLoadFromFileSystemStructure(), 0); // dirty 时拒绝载入
      QCOMPARE(viewSplitter.onInitATable(), false);
      const QSqlRecord after0RowRecord{viewSplitter.m_castDbModel->record(0)};
      QCOMPARE(after0RowRecord, before0RowRecord);

      // 拒绝搜索
      searchTb.SetWhereClause("1=1");
      emit searchTb.whereClauseChanged("1=1");
      QCOMPARE(viewSplitter.m_castDbModel->filter(), ""); // not "1=1"

      emit viewSplitter.m_castTableView->searchSqlStatementChanged("Height>=180");
      QCOMPARE(viewSplitter.m_castDbModel->filter(), ""); // not "Height>=180"
      QCOMPARE(searchTb.GetCurrentWhereClause(), "Height>=180");
    }

    // 撤掉了变更
    {
      QCOMPARE(viewSplitter.m_castDbModel->isDirty(), true);
      QVERIFY(viewSplitter.m_castTableView->onRevertAll());
      QCOMPARE(viewSplitter.m_castDbModel->rowCount(), 0);
      QCOMPARE(viewSplitter.m_castDbModel->isDirty(), false);
    }

    // 外部载入成功, 自动 repopulate下
    {
      QCOMPARE(viewSplitter.onLoadFromPsonDirectory(), 0); // 取消
      QCOMPARE(viewSplitter.onLoadFromPsonDirectory(), 0); // 确认, 但是没有pson
      QCOMPARE(viewSplitter.onLoadFromFileSystemStructure(), 1); // 1 个 Cast 有image, 其它都没有， 不会询问
      QCOMPARE(viewSplitter.m_castDbModel->tableName(), tableName);
      QCOMPARE(viewSplitter.m_castDbModel->isDirty(), false);
      QCOMPARE(viewSplitter.m_castDbModel->rowCount(), 1);
    }

    // 外部删表, 自动 repopulate下
    {
      QCOMPARE(viewSplitter.onDropDeleteTable(DbManagerHelper::DropOrDeleteE::DELETE), false); // 取消
      QCOMPARE(viewSplitter.onDropDeleteTable(DbManagerHelper::DropOrDeleteE::DELETE), true); // 删除数据后
      QCOMPARE(viewSplitter.m_castDbModel->tableName(), tableName);
      QCOMPARE(viewSplitter.m_castDbModel->isDirty(), false);
      QCOMPARE(viewSplitter.m_castDbModel->rowCount(), 0);

      QCOMPARE(viewSplitter.onDropDeleteTable(DbManagerHelper::DropOrDeleteE::DROP), false); // 取消
      QCOMPARE(viewSplitter.onDropDeleteTable(DbManagerHelper::DropOrDeleteE::DROP), true);  // 删表
      QCOMPARE(viewSplitter.m_castDbModel->tableName(), "");
      QCOMPARE(viewSplitter.m_castDbModel->rowCount(), 0);
      QCOMPARE(viewSplitter.m_castDbModel->columnCount(), 0);
    }
  }

  void show_hide_ok() {
    CastDBActions::GetInst().m_showCastsCardsListAct->setChecked(true);

    QWidget wid;
    CastDatabaseSearchToolBar searchTb{"CastDbSearchBar", &wid};
    CastDBViewSplitter viewSplitter{dbName, connName, imageHostFullPath, &searchTb, &wid};
    QCOMPARE(getConfig(CastKey::SHOW_CASTS_IN_CARD_VIEW), true);

    QCOMPARE(viewSplitter.GetTableView(), viewSplitter.m_castTableView);
    QCOMPARE(viewSplitter.GetListView(), viewSplitter.m_castListView);
    QVERIFY(viewSplitter.m_castTableView != nullptr);
    QVERIFY(viewSplitter.m_castListView != nullptr);
    QCOMPARE(viewSplitter.count(), 2); // 加了2个view
    QCOMPARE(viewSplitter.m_castListView->isHidden(), false);

    QCOMPARE(CastDBActions::GetInst().m_showCastsCardsListAct->isChecked(), true);
    CastDBActions::GetInst().m_showCastsCardsListAct->toggle();
    QCOMPARE(CastDBActions::GetInst().m_showCastsCardsListAct->isChecked(), false);
    QCOMPARE(getConfig(CastKey::SHOW_CASTS_IN_CARD_VIEW), false);
    QCOMPARE(viewSplitter.m_castListView->isHidden(), true);
  }
};

#include "CastDBViewSplitterTest.moc"
REGISTER_TEST(CastDBViewSplitterTest, false)