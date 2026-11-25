#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "CastDBTableView.h"
#include "EndToExposePrivateMember.h"

#include "CastBaseDb.h"
#include "CastDbModel.h"

#include "TDir.h"
#include "PublicVariable.h"
#include "CastPsonFileHelper.h"
#include "SqlRecordTestHelper.h"
#include "InputDialogHelper.h"

#include <QSignalSpy>
#include <QSqlRecord>

using namespace SqlRecordTestHelper;

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

Q_DECLARE_METATYPE(QSqlRecord)
    class CastDBTableViewTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir mTDir;
  const QString imageHostFullPath{mTDir.itemPath("host")};
  const QString dbName{mTDir.itemPath("CastViewTest.db")};
  const QString connName{"ConnCastTableViewTest"};
  CastBaseDb castDb{dbName, connName};
  const QString tableName{DB_TABLE::PERFORMERS};

private slots:
  void initTestCase() { //
    QVERIFY(mTDir.IsValid());

    qRegisterMetaType<QSqlRecord>("QSqlRecord");
    QVERIFY(castDb.IsValid());
    QVERIFY(castDb.CreateTable(tableName, CastBaseDb::CREATE_PERF_TABLE_TEMPLATE));

    const QList<FsNodeEntry> nodes {
      FsNodeEntry{"host/Football/Kaka/Kaka.pson", false, ""},
      FsNodeEntry{"host/Football/Cristiano/Cristiano.pson", false, ""},
    };
    QCOMPARE(mTDir.createEntries(nodes), nodes.size());

  }

  void init() {
    GlobalMockObject::reset();
    castDb.DeleteByWhereClause(tableName, "1=1");
  }
  void cleanup() {
    GlobalMockObject::verify();
  }

  void onInsertARowBeforeCurrentIndex_onDeleteRecords_dirtyStatesChanged_ok() {
    CastDbModel castDbModel{imageHostFullPath, nullptr, castDb.GetDb()};
    QCOMPARE(castDbModel.rowCount(), 0);
    QCOMPARE(castDbModel.tableName(), tableName);

    QItemSelectionModel selModel{&castDbModel};
    CastDBTableView castView{&castDbModel, &selModel};
    QCOMPARE(castView.selectionModel(), &selModel);


    QSignalSpy dirtyStatesChangedSpy{&castDbModel, &SqlTableModelPub::dirtyStatesChanged};
    QVERIFY(dirtyStatesChangedSpy.isValid());

    { // initial ok
      // nothing selected
      QVERIFY(!castView.currentIndex().isValid());
      QCOMPARE(castView.onRefreshVidsFieldCore({}), 0);
      QCOMPARE(castView.onRefreshImgsFieldCore({}), 0);
      QCOMPARE(castView.onRefreshAllVidsField(), 0);
      QCOMPARE(castView.onRefreshAllImgsField(), 0);
      QVERIFY(!castView.EmitCurrentCastRecordChanged(QModelIndex{}, QModelIndex{}));
    }
    QCOMPARE(dirtyStatesChangedSpy.count(), 0); //

    QSqlRecord kakaRecord = SqlRecordTestHelper::GetACastRecordLine("Kaka", "Football", ""/*No Imgs*/, ""/*No Vids*/);
    QSqlRecord cristianoRecord = SqlRecordTestHelper::GetACastRecordLine("Cristiano", "Football", ""/*No Imgs*/, ""/*No Vids*/);
    {
      QVERIFY(castDbModel.insertRecord(0, kakaRecord));
      QVERIFY(castDbModel.insertRecord(1, cristianoRecord));
      QCOMPARE(castDbModel.rowCount(), 2);
      QCOMPARE(dirtyStatesChangedSpy.count(), 1);
      QCOMPARE(dirtyStatesChangedSpy.takeLast(), (QVariantList{QVariant{true}, }));  // 插入两条, 内部有去重， 只发出一次
      dirtyStatesChangedSpy.clear();
      QVERIFY(castDbModel.isDirty());
    }

    // onInsertARowBeforeCurrentIndex
    { // onDeleteRecords ok
      // Inserted but not yet successfully submitted rows in the range to be removed are immediately removed from the model.
      // 在待移除范围内，已插入但尚未成功提交的行，会立即从模型中移除。 而不是标记删除
      // 插入2条->未保存->无选择时删除->有选择时取消删除->有选择时确认删除
      MOCKER(InputDialogHelper::YesOrCancelBox).expects(exactly(3)).will(returnValue(false)).then(returnValue(true)).then(returnValue(true));

      QVERIFY(castDbModel.isDirty());
      QVERIFY(!selModel.hasSelection());
      QCOMPARE(castView.onDeleteRecords(), 0); // no selection at all

      castView.selectAll();
      QVERIFY(selModel.hasSelection());
      QCOMPARE(castView.onDeleteRecords(), 0); // cancel mark delete
      QCOMPARE(castDbModel.rowCount(), 2);

      QCOMPARE(castView.onDeleteRecords(), 2); // confirm mark delete 但是这两条记录本就时暂存的， 所以就不时标记了， 直接删除
      QCOMPARE(castDbModel.rowCount(), 0);
      QVERIFY(!castDbModel.isDirty());
      QCOMPARE(dirtyStatesChangedSpy.count(), 1); // 没有调用 submitAll(), select(), revertAll(), 所以现在还是dirty()状态
      QCOMPARE(dirtyStatesChangedSpy.takeLast(), (QVariantList{QVariant{false}, }));  // 删除2行暂存状态，回到初始状态， 发出一次
      dirtyStatesChangedSpy.clear();

      // 无需保存->0行
      QVERIFY(castView.onSubmitAll());
      QCOMPARE(castDbModel.rowCount(), 0);     // 真实入库
      QVERIFY(!castDbModel.isDirty());
      QCOMPARE(dirtyStatesChangedSpy.count(), 0);

      // 插入2条->已保存->有选择时确认删除
      QVERIFY(castDbModel.insertRecord(0, kakaRecord));
      QVERIFY(castDbModel.insertRecord(1, cristianoRecord));
      QVERIFY(castDbModel.isDirty());
      QCOMPARE(dirtyStatesChangedSpy.count(), 1);
      QCOMPARE(dirtyStatesChangedSpy.takeLast(), (QVariantList{QVariant{true}, }));  // 插入2次， 去重， 只发出一次
      dirtyStatesChangedSpy.clear();

      QVERIFY(castView.onSubmitAll());
      QVERIFY(!castDbModel.isDirty());
      QCOMPARE(dirtyStatesChangedSpy.count(), 1);
      QCOMPARE(dirtyStatesChangedSpy.takeLast(), (QVariantList{QVariant{false}, }));  // 保存1次， 发出一次
      dirtyStatesChangedSpy.clear();

      castView.selectAll();
      QCOMPARE(castView.onDeleteRecords(), 2); // confirm mark delete 但是这两条记录持久化在库中， 所以只标记了， 不直接删除
      QCOMPARE(castDbModel.rowCount(), 2);
      QCOMPARE(dirtyStatesChangedSpy.count(), 1);                                    // 有监控到变更垂直表头有变
      QCOMPARE(dirtyStatesChangedSpy.takeLast(), (QVariantList{QVariant{true}, }));  // 删除1次， 发出一次
      dirtyStatesChangedSpy.clear();

      // 保存->0行
      QVERIFY(castView.onSubmitAll());
      QCOMPARE(castDbModel.rowCount(), 0);
      QVERIFY(!castDbModel.isDirty());
      QCOMPARE(dirtyStatesChangedSpy.count(), 1);
      QCOMPARE(dirtyStatesChangedSpy.takeLast(), (QVariantList{QVariant{false}, }));  // 保存1次， 发出一次
      dirtyStatesChangedSpy.clear();
    }

    QCOMPARE(castDbModel.rowCount(), 0);
    {
      QVERIFY(!castView.currentIndex().isValid());
      QCOMPARE(castView.onInsertARowBeforeCurrentIndex(), 0); // insert at index 0
      castView.setCurrentIndex(castDbModel.index(0, 0));
      QCOMPARE(castView.onInsertARowBeforeCurrentIndex(), 0); // insert at index 0
      castView.setCurrentIndex(castDbModel.index(1, 0));
      QCOMPARE(castView.onInsertARowBeforeCurrentIndex(), 1); // insert at index 1
      QCOMPARE(castDbModel.rowCount(), 3);
      QVERIFY(castDbModel.isDirty());
      QCOMPARE(dirtyStatesChangedSpy.count(), 1);
      QCOMPARE(dirtyStatesChangedSpy.takeLast(), (QVariantList{QVariant{true}, }));  // 修改3次， 发出一次
      dirtyStatesChangedSpy.clear();

      QVERIFY(!castView.onSubmitAll()); // 空行， 不可以保存， no fields to update
      QVERIFY(castView.onRevertAll()); // 可以撤销
      QCOMPARE(castDbModel.rowCount(), 0);
      QVERIFY(!castDbModel.isDirty());
      QCOMPARE(dirtyStatesChangedSpy.count(), 1);
      QCOMPARE(dirtyStatesChangedSpy.takeLast(), (QVariantList{QVariant{false}, }));  // submitAll 1次失败， revertAll 1次成功, 后者成功触发信号
      dirtyStatesChangedSpy.clear();
    }
  }

  void onRefreshImgsField_onRefreshVidsFieldCore_currentRecordChangedSig_ok() {
    CastDbModel castDbModel{imageHostFullPath, nullptr, castDb.GetDb()};
    QItemSelectionModel selModel{&castDbModel};
    CastDBTableView castView{&castDbModel, &selModel};
    QCOMPARE(castView.selectionModel(), &selModel);

    QSqlRecord kakaRecord = SqlRecordTestHelper::GetACastRecordLine("Kaka", "Football", ""/*No Imgs*/, ""/*No Vids*/);
    QSqlRecord cristianoRecord = SqlRecordTestHelper::GetACastRecordLine("Cristiano", "Football", ""/*No Imgs*/, ""/*No Vids*/);
    QVERIFY(castDbModel.insertRecord(0, kakaRecord));
    QVERIFY(castDbModel.insertRecord(1, cristianoRecord));
    QCOMPARE(castDbModel.rowCount(), 2);

    QSignalSpy currentRecordChangedSig(&castView, &CastDBTableView::currentRecordChanged);
    currentRecordChangedSig.clear();

    // setCurrentIndex 触发一次 currentRecordChanged
    // selModel信号的QItemSelectionModel::currentRowChanged连接到了currentRecordChanged
    {
      const QModelIndex firstInd = castDbModel.index(0, CastDbModelField::Name);
      QCOMPARE(firstInd.data(Qt::DisplayRole).toString(), "Kaka");
      castView.setCurrentIndex(firstInd);
      QCOMPARE(currentRecordChangedSig.count(), 1);
      const QVariantList sigParms = currentRecordChangedSig.takeLast();
      QCOMPARE(sigParms.size(), 2);
      const QSqlRecord kakaRecordInSignal = sigParms[0].value<QSqlRecord>();
      QVERIFY(!kakaRecordInSignal.isEmpty());
      QCOMPARE(sigParms[1].toString(), imageHostFullPath);
      currentRecordChangedSig.clear();
    }

    MOCKER(CastDBTableView::onRefreshImgsFieldCoreStatic).expects(exactly(1)).will(returnValue(2));
    MOCKER(CastDBTableView::onRefreshVidsFieldCoreStatic).expects(exactly(1)).will(returnValue(1));

    // 阈值至少>2, 才需要弹框
    QVERIFY(CastDBTableView::QUERY_CONFIRM_IF_ROW_SELECTED_COUNT_ABOVE > 2);
    MOCKER(InputDialogHelper::YesOrCancelBox).expects(never()).will(ignoreReturnValue());

    // 没有选中， 不进入流程
    {
      castView.clearSelection();
      QCOMPARE(castView.onRefreshImgsField(), 0);
      QCOMPARE(castView.onRefreshVidsField(), 0);

      currentRecordChangedSig.clear();
    }

    // 有选中， 有当前行， 但是没有变更(Mock导致)， 不会触发currentRecordChanged
    {
      castView.selectAll();
      QVERIFY(castView.currentIndex().isValid());
      QCOMPARE(castView.onRefreshImgsField(), 2);
      QCOMPARE(castView.onRefreshVidsField(), 1);

      currentRecordChangedSig.clear();
    }

    // 有选中， 有当前行, 有变更SqlRecord, 会触发 currentRecordChanged
  }

  void CurrentIndexGuard_ok() {
    // 提交保存/回撤变更操作时. currentIndex, selection都要ok
    CastDbModel castDbModel{imageHostFullPath, nullptr, castDb.GetDb()};
    QItemSelectionModel selModel{&castDbModel};
    CastDBTableView castView{&castDbModel, &selModel};
    QCOMPARE(castView.selectionModel(), &selModel);
    QSignalSpy currentRecordChangedSig(&castView, &CastDBTableView::currentRecordChanged);


    QSqlRecord kakaRecord = SqlRecordTestHelper::GetACastRecordLine("Kaka", "Football", ""/*No Imgs*/, ""/*No Vids*/);
    QSqlRecord cristianoRecord = SqlRecordTestHelper::GetACastRecordLine("Cristiano", "Football", ""/*No Imgs*/, ""/*No Vids*/);
    {
      QVERIFY(castDbModel.insertRecord(0, kakaRecord));
      QVERIFY(castDbModel.insertRecord(1, cristianoRecord));
      QCOMPARE(castDbModel.rowCount(), 2);
    }

    const QModelIndex firstInd = castDbModel.index(0, CastDbModelField::Name);
    QVERIFY(firstInd.isValid());
    QCOMPARE(firstInd.data(Qt::DisplayRole).toString(), "Kaka");

    // 保存, 验证当前选中
    {
      currentRecordChangedSig.clear();
      castView.setCurrentIndex(castDbModel.index(0, 0));
      QCOMPARE(currentRecordChangedSig.count(), 1);

      castView.onSubmitAll(); // 保存
      QCOMPARE(castDbModel.isDirty(), false);

      QVERIFY(castView.currentIndex().isValid());
      QCOMPARE(castView.currentIndex().data(Qt::DisplayRole).toString(), "Kaka");
    }

    // 删除所有行, 只是标记, 还是有选中
    MOCKER(InputDialogHelper::YesOrCancelBox).expects(exactly(1)).will(returnValue(true));
    {
      castView.selectAll();
      QCOMPARE(castView.onDeleteRecords(), 2);
      QCOMPARE(castDbModel.isDirty(), true);
      QVERIFY(selModel.hasSelection());
    }

    // 保存后, 0行
    {
      castView.onSubmitAll(); // 保存
      QCOMPARE(castDbModel.rowCount(), 0);
      QCOMPARE(castDbModel.isDirty(), false);
      QVERIFY(!selModel.hasSelection());
    }

    // 插入顺序反一下
    {
      QVERIFY(castDbModel.insertRecord(0, cristianoRecord));
      QVERIFY(castDbModel.insertRecord(1, kakaRecord));
      QCOMPARE(castDbModel.rowCount(), 2);
    }

    // 保存后, 2行, 选中两个
    {
      castView.selectAll();
      castView.onSubmitAll();           // 保存前有
      QCOMPARE(castDbModel.isDirty(), false);
      QVERIFY(selModel.hasSelection()); // 保存后也有
    }

    QCOMPARE(castDbModel.rowCount(), 2);
    // 全选, 修改内容, 回撤, 验证批量选中
    {
      castView.selectAll();
      QVERIFY(selModel.hasSelection());      // 回撤前有
      castDbModel.setData(castDbModel.index(0, CastDbModelField::Height), 195, Qt::EditRole);
      QCOMPARE(castDbModel.isDirty(), true);
      castView.onRevertAll();                // 回撤修改
      QVERIFY(selModel.hasSelection());      // 回撤后也有
      QCOMPARE(castDbModel.isDirty(), false);
    }
  }
};

#include "CastDBTableViewTest.moc"
REGISTER_TEST(CastDBTableViewTest, false)
