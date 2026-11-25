#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "CastDBListView.h"
#include "EndToExposePrivateMember.h"
#include "CastBaseDb.h"
#include "CastCardDelegate.h"
#include "CastDBCardEditor.h"

#include "TDir.h"
#include "PublicVariable.h"
#include "Configuration.h"
#include "SqlRecordTestHelper.h"

#include <QLineEdit>
#include <QSpinBox>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

    class CastDBListViewTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir mTDir;
  const QString imageHostFullPath{mTDir.itemPath("host")};
  const QString dbName{mTDir.itemPath("CastViewTest.db")};
  const QString connName{"ConnCastListViewTest"};
  CastBaseDb castDb{dbName, connName};
  const QString tableName{DB_TABLE::PERFORMERS};

private slots:
  void initTestCase() {
    QVERIFY(mTDir.IsValid());

    QVERIFY(castDb.IsValid());
    QVERIFY(castDb.CreateTable(tableName, CastBaseDb::CREATE_PERF_TABLE_TEMPLATE));
    const QList<FsNodeEntry> nodes {
                                   FsNodeEntry{"host/Football/Kaka/Kaka.pson", false, ""},
                                   FsNodeEntry{"host/Football/Cristiano/Cristiano.pson", false, ""},
                                   };
    QCOMPARE(mTDir.createEntries(nodes), nodes.size());

    Configuration().clear();
  }

  void init() {  //
    GlobalMockObject::reset();
    castDb.DeleteByWhereClause(tableName, "1=1");
  }

  void cleanup() {  //
    GlobalMockObject::verify();
  }

  void default_ok() {
    CastDbModel castDbModel{imageHostFullPath, nullptr, castDb.GetDb()};
    QCOMPARE(castDbModel.rowCount(), 0);
    QCOMPARE(castDbModel.tableName(), tableName);

    QItemSelectionModel selModel{&castDbModel};
    CastDBListView castView{&castDbModel, &selModel};
    QCOMPARE(castView.selectionModel(), &selModel);
    QCOMPARE(castView.model(), &castDbModel);

    // 初始配置正确
    QCOMPARE(castView.CustomListView::m_defaultFlowLeft2Right, true);
    QCOMPARE(castView.isLeftToRight(), true);
    QCOMPARE(castView.CustomListView::m_defaultViewModeIcon, true);
    QCOMPARE(castView.viewMode(), QListView::ViewMode::IconMode);
    QCOMPARE(castView.CustomListView::m_defaultWrapping, true);
    QCOMPARE(castView.isWrapping(), true);
    QCOMPARE(castView.CustomListView::m_uniformItemSize, true);
    QCOMPARE(castView.uniformItemSizes(), true);
    QCOMPARE(castView.CustomListView::m_resizeModeAdjustFixed, true);
    QCOMPARE(castView.resizeMode(), QListView::ResizeMode::Adjust);

    QCOMPARE(castView.movement(), QListView::Movement::Static);
    QCOMPARE(castView.hasMouseTracking(), true);

    // 初始卡片是竖直长图， 验证 IMAGE_SIZE::GetInitialVerticalLongerMode(GetName()) GetName()中含有cast， 返回正确尺寸索引
    QSize initialIconSize = castView.iconSize();
    QVERIFY(initialIconSize.isValid());
    QVERIFY(initialIconSize.width() < initialIconSize.height());

    QCOMPARE(castView.GetCardDelegate(), castView.m_castCardDel);
    QVERIFY(castView.m_castCardDel != nullptr);
    QCOMPARE(castView.m_castCardDel->cardSize(), initialIconSize);
  }


  void delegate_ok () {
    QSqlRecord kakaRecord = SqlRecordTestHelper::GetACastRecordLine("Kaka", "Football", ""/*No Imgs*/, ""/*No Vids*/);
    kakaRecord.setValue(CastDbModelField::Rate, 9);
    kakaRecord.setValue(CastDbModelField::Height, 185);

    QSqlRecord cristianoRecord = SqlRecordTestHelper::GetACastRecordLine("Cristiano", "Football", ""/*No Imgs*/, ""/*No Vids*/);
    cristianoRecord.setValue(CastDbModelField::Rate, 10);
    cristianoRecord.setValue(CastDbModelField::Height, 187);

    CastDbModel castDbModel{imageHostFullPath, nullptr, castDb.GetDb()};
    QCOMPARE(castDbModel.rowCount(), 0);
    QCOMPARE(castDbModel.tableName(), tableName);
    QVERIFY(castDbModel.insertRecord(0, kakaRecord));
    QVERIFY(castDbModel.insertRecord(1, cristianoRecord));
    QVERIFY(castDbModel.submitSaveAllChanges());
    QCOMPARE(castDbModel.rowCount(), 2);

    QItemSelectionModel selModel{&castDbModel};
    CastDBListView lv{&castDbModel, &selModel};
    QCOMPARE(lv.selectionModel(), &selModel);
    QCOMPARE(lv.model(), &castDbModel);
    QVERIFY(!castDbModel.isDirty());
    castDbModel.sort(CastDbModelField::Name, Qt::SortOrder::AscendingOrder);
    QCOMPARE(castDbModel.rowCount(), 2);

    QCOMPARE(castDbModel.index(0, CastDbModelField::Name).data(Qt::DisplayRole).toString(), "Cristiano");
    QCOMPARE(castDbModel.index(1, CastDbModelField::Name).data(Qt::DisplayRole).toString(), "Kaka");


    CastCardDelegate* pDelegates = lv.GetCardDelegate();

    // 编辑态. 不测试信号链路, 启用编辑， 修改 index(0). Name: "Cristiano" -> "BrazilCristiano", Height: 187 -> 188
    {
      // 前置检查内存
      QCOMPARE(castDbModel.index(0, CastDbModelField::Name).data(Qt::DisplayRole).toString(), "Cristiano");
      QCOMPARE(castDbModel.index(0, CastDbModelField::Rate).data(Qt::DisplayRole).toInt(), 10);
      QCOMPARE(castDbModel.index(0, CastDbModelField::Height).data(Qt::DisplayRole).toInt(), 187);

      QCOMPARE(castDbModel.index(1, CastDbModelField::Name).data(Qt::DisplayRole).toString(), "Kaka");
      QCOMPARE(castDbModel.index(1, CastDbModelField::Rate).data(Qt::DisplayRole).toInt(), 9);
      QCOMPARE(castDbModel.index(1, CastDbModelField::Height).data(Qt::DisplayRole).toInt(), 185);

      QWidget* editorWidget0 = pDelegates->createEditor(&lv, QStyleOptionViewItem{}, castDbModel.index(0, CastDbModelField::Name));
      QVERIFY(editorWidget0 != nullptr);
      CastDBCardEditor* cardEditor0 = qobject_cast<CastDBCardEditor*>(editorWidget0);
      QVERIFY(cardEditor0 != nullptr);
      pDelegates->setEditorData(cardEditor0, castDbModel.index(0, CastDbModelField::Name));
      QVERIFY(cardEditor0->mActorNameEdit != nullptr);
      QVERIFY(cardEditor0->mRateEdit != nullptr);
      QVERIFY(cardEditor0->mAliasEdit != nullptr);
      QVERIFY(cardEditor0->mTagsEdit != nullptr);
      QVERIFY(cardEditor0->mOriBelongToEdit != nullptr);
      QVERIFY(cardEditor0->mHeightEdit != nullptr);
      QVERIFY(cardEditor0->mSizeEdit != nullptr);
      QVERIFY(cardEditor0->mDetailEdit != nullptr);

      QCOMPARE(cardEditor0->mActorNameEdit->text(), "Cristiano");
      QCOMPARE(cardEditor0->mRateEdit->value(), 10);
      QCOMPARE(cardEditor0->mOriBelongToEdit->text(), "Football");
      QCOMPARE(cardEditor0->mHeightEdit->value(), 187);

      cardEditor0->mActorNameEdit->setText("ZZZCristiano"); // 预期不能做到动态排序
      cardEditor0->mHeightEdit->setValue(188);
      QCOMPARE(cardEditor0->mHeightEdit->value(), 188);

      MOCKER(CastDbModel::MigrateCastNameFolder).expects(exactly(1)).will(returnValue(1)); // 至少返回1个
      MOCKER(CastDbModel::MigrateCastsOriFolder).expects(exactly(1)).will(returnValue(0)); // 没有改动

      QCOMPARE(castDbModel.isDirty(), false);

      // 调用一次setModelData， 然后多轮完整的(setData, setEditorData), 前者写内存时的emit dataChanged触发后者写gui。
      pDelegates->setModelData(cardEditor0, &castDbModel, castDbModel.index(0, CastDbModelField::Name));
      // 注意: 这里没有关闭编辑器界面
      delete cardEditor0;
      cardEditor0 = nullptr;

      QCOMPARE(castDbModel.isDirty(), true); // 有变更

      // 后置检查内存
      QCOMPARE(castDbModel.index(0, CastDbModelField::Name).data(Qt::DisplayRole).toString(), "ZZZCristiano"); // Cristiano->ZZZCristiano
      QCOMPARE(castDbModel.index(0, CastDbModelField::Rate).data(Qt::DisplayRole).toInt(), 10);
      QCOMPARE(castDbModel.index(0, CastDbModelField::Height).data(Qt::DisplayRole).toInt(), 188); // 187 -> 188

      QCOMPARE(castDbModel.index(1, CastDbModelField::Name).data(Qt::DisplayRole).toString(), "Kaka");
      QCOMPARE(castDbModel.index(1, CastDbModelField::Rate).data(Qt::DisplayRole).toInt(), 9);
      QCOMPARE(castDbModel.index(1, CastDbModelField::Height).data(Qt::DisplayRole).toInt(), 185);
    }

  }
};

#include "CastDBListViewTest.moc"
REGISTER_TEST(CastDBListViewTest, false)