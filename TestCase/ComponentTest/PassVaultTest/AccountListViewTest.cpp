#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "AccountListView.h"
#include "AccountCardEditor.h"

#include <QSignalSpy>
#include <QLineEdit>
#include <QPlainTextEdit>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS


class AccountListViewTest : public PlainTestSuite {
  Q_OBJECT
public:
private:
  const int TYPE_COLUMN{AccountInfo::Role2Column(AccountInfo::Role::TYPE_STR_ROLE)};
  const int NAME_COLUMN{AccountInfo::Role2Column(AccountInfo::Role::NAME_STR_ROLE)};
  const int ACCOUNT_COLUMN{AccountInfo::Role2Column(AccountInfo::Role::ACCOUNT_STR_ROLE)};

private slots:
  void init() {  //
    GlobalMockObject::reset();
  }
  void cleanup() {  //
    GlobalMockObject::verify();
  }

  void default_ok() {
    AccountTableModel model{"Path/To/An/Inexist/enc.csv", "RANDOM_AES_KEY"};
    QCOMPARE(model.rowCount(), 0); // 0 行
    AccountSortFilterProxyModel proxyModel;
    proxyModel.setSourceModel(&model);

    AccountListView lv{&model, &proxyModel};

    QVERIFY(lv.itemDelegate() != nullptr);
    QVERIFY(lv.uniformItemSizes());
  }

  void AccountCardDelegate_ok() {
    QList<AccountInfo> accList{
                               AccountInfo{"9Game", "Minecraft", "DiamondTuber", "123", "Duck"},
                               AccountInfo{"3Game", "Stardew Valley", "DanceingFire", "456", "emhh"},
                               };

    MOCKER(AccountFileHelper::ValidateAndLoad)
        .expects(exactly(1))
        .will(returnValue(AccountFileHelper::LoadResult{true,  accList}));

    AccountTableModel model{"Path/to/an/exist/accounts.enc.csv", "CORRECT_AES_KEY"};
    QCOMPARE(model.rowCount(), 2); // 2 行

    AccountSortFilterProxyModel proxyModel;
    proxyModel.setSourceModel(&model);
    proxyModel.initSortSetting(AccountInfo::Role::TYPE_STR_ROLE, false); // 按照Type， 升序
    {
      QCOMPARE(proxyModel.index(0, TYPE_COLUMN).data(Qt::DisplayRole).toString(), "3Game");
      QCOMPARE(proxyModel.index(1, TYPE_COLUMN).data(Qt::DisplayRole).toString(), "9Game");
    }

    AccountListView lv{&model, &proxyModel};
    AccountCardDelegate* pDelegates = lv.GetCardDelegate();
    QVERIFY(pDelegates != nullptr);


    // 在末尾插入2个空行
    QCOMPARE(model.InsertNRows(2, 2), 2);
    QCOMPARE(model.rowCount(), 2 + 2);
    QModelIndex src0Ind = model.index(0, 0);
    QModelIndex src1Ind = model.index(1, 0);
    QModelIndex src2Ind = model.index(2, 0);
    QModelIndex src3Ind = model.index(3, 0);

    {
      // 排序已生效
      QCOMPARE(proxyModel.index(0, TYPE_COLUMN).data(Qt::DisplayRole).toString(), "");
      QCOMPARE(proxyModel.index(1, TYPE_COLUMN).data(Qt::DisplayRole).toString(), "");
      QCOMPARE(proxyModel.index(2, TYPE_COLUMN).data(Qt::DisplayRole).toString(), "3Game");
      QCOMPARE(proxyModel.index(3, TYPE_COLUMN).data(Qt::DisplayRole).toString(), "9Game");
    }

    // 不测试信号链路, 启用编辑， 修改src0Ind. 9Game -> 0Game
    {
      QWidget* editorWidget0 = pDelegates->createEditor(&lv, QStyleOptionViewItem{}, proxyModel.mapFromSource(src0Ind));
      QVERIFY(editorWidget0 != nullptr);
      AccountCardEditor* cardEditor0 = qobject_cast<AccountCardEditor*>(editorWidget0);
      QVERIFY(cardEditor0 != nullptr);
      pDelegates->setEditorData(cardEditor0, proxyModel.mapFromSource(src0Ind));
      QVERIFY(cardEditor0->typeEdit() != nullptr);
      QVERIFY(cardEditor0->nameEdit() != nullptr);
      QVERIFY(cardEditor0->accountEdit() != nullptr);
      QVERIFY(cardEditor0->pwdEdit() != nullptr);
      QVERIFY(cardEditor0->othersEdit() != nullptr);

      // 预期setEditorData调用了
      QCOMPARE(cardEditor0->typeEdit()->text(), "9Game");
      QCOMPARE(cardEditor0->nameEdit()->text(), "Minecraft");
      QCOMPARE(cardEditor0->accountEdit()->text(), "DiamondTuber");
      QCOMPARE(cardEditor0->pwdEdit()->text(), "123");
      QCOMPARE(cardEditor0->othersEdit()->toPlainText(), "Duck");

      cardEditor0->typeEdit()->setText("0Game");
      cardEditor0->nameEdit()->setText("ModifiedMinecraft");
      cardEditor0->accountEdit()->setText("ModifiedDiamondTuber");
      cardEditor0->pwdEdit()->setText("Modified123");
      cardEditor0->othersEdit()->setPlainText("ModifiedDuck");

      // 调用一次setModelData， 然后5轮完整的(setData, setEditorData), 前者写内存时的emit dataChanged触发后者写gui。
      pDelegates->setModelData(cardEditor0, &proxyModel, proxyModel.mapFromSource(src0Ind));
      // 注意: 这里没有关闭编辑器界面
      delete cardEditor0;
      cardEditor0 = nullptr;

      // AccountInfo{"0Game", "ModifiedMinecraft", "ModifiedDiamondTuber", "Modified123", "ModifiedDuck"},
      // AccountInfo{"3Game", "Stardew Valley", "DanceingFire", "456", "emhh"},
      // AccountInfo{}
      // AccountInfo{}
      //
      // 检查内存， 修改写入行正确
      QCOMPARE(*model.rowDataAt(0), (AccountInfo{"0Game", "ModifiedMinecraft", "ModifiedDiamondTuber", "Modified123", "ModifiedDuck"}));
      QCOMPARE(*model.rowDataAt(1), (AccountInfo{"3Game", "Stardew Valley", "DanceingFire", "456", "emhh"}));
      QCOMPARE(*model.rowDataAt(2), (AccountInfo{}));
      QCOMPARE(*model.rowDataAt(3), (AccountInfo{}));

      // 按照Type， 升序
      QCOMPARE(proxyModel.index(0, TYPE_COLUMN).data(Qt::DisplayRole).toString(), "");
      QCOMPARE(proxyModel.index(0, NAME_COLUMN).data(Qt::DisplayRole).toString(), "");
      QCOMPARE(proxyModel.index(0, ACCOUNT_COLUMN).data(Qt::DisplayRole).toString(), "");

      QCOMPARE(proxyModel.index(1, TYPE_COLUMN).data(Qt::DisplayRole).toString(), "");
      QCOMPARE(proxyModel.index(1, NAME_COLUMN).data(Qt::DisplayRole).toString(), "");
      QCOMPARE(proxyModel.index(1, ACCOUNT_COLUMN).data(Qt::DisplayRole).toString(), "");

      QCOMPARE(proxyModel.index(2, TYPE_COLUMN).data(Qt::DisplayRole).toString(), "0Game");
      QCOMPARE(proxyModel.index(2, NAME_COLUMN).data(Qt::DisplayRole).toString(), "ModifiedMinecraft");
      QCOMPARE(proxyModel.index(2, ACCOUNT_COLUMN).data(Qt::DisplayRole).toString(), "ModifiedDiamondTuber");

      QCOMPARE(proxyModel.index(3, TYPE_COLUMN).data(Qt::DisplayRole).toString(), "3Game");
      QCOMPARE(proxyModel.index(3, NAME_COLUMN).data(Qt::DisplayRole).toString(), "Stardew Valley");
      QCOMPARE(proxyModel.index(3, ACCOUNT_COLUMN).data(Qt::DisplayRole).toString(), "DanceingFire");
    }

    // 不测试信号链路, 启用编辑， 修改 src3Ind
    {
      QWidget* editorWidget3 = pDelegates->createEditor(&lv, QStyleOptionViewItem{}, proxyModel.mapFromSource(src3Ind));
      QVERIFY(editorWidget3 != nullptr);
      AccountCardEditor* cardEditor3 = qobject_cast<AccountCardEditor*>(editorWidget3);
      QVERIFY(cardEditor3 != nullptr);
      pDelegates->setEditorData(cardEditor3, proxyModel.mapFromSource(src3Ind));
      QCOMPARE(cardEditor3->typeEdit()->text(), "");
      QCOMPARE(cardEditor3->nameEdit()->text(), "");
      QCOMPARE(cardEditor3->accountEdit()->text(), "");
      QCOMPARE(cardEditor3->pwdEdit()->text(), "");
      QCOMPARE(cardEditor3->othersEdit()->toPlainText(), "");

      cardEditor3->typeEdit()->setText("1Game");
      cardEditor3->nameEdit()->setText("1Name");
      cardEditor3->accountEdit()->setText("1Account");
      cardEditor3->pwdEdit()->setText("1Pwd");
      cardEditor3->othersEdit()->setPlainText("1Others");

      pDelegates->setModelData(cardEditor3, &proxyModel, proxyModel.mapFromSource(src3Ind));
      // 注意: 这里没有关闭编辑器界面
      delete cardEditor3;
      cardEditor3 = nullptr;

      // AccountInfo{"0Game", "ModifiedMinecraft", "ModifiedDiamondTuber", "Modified123", "ModifiedDuck"},
      // AccountInfo{"3Game", "Stardew Valley", "DanceingFire", "456", "emhh"},
      // AccountInfo{},
      // AccountInfo{"1Game", "1Name", "1Account", "1Pwd", "1Others"},
      //
      // 检查内存， 修改写入行正确
      QCOMPARE(*model.rowDataAt(0), (AccountInfo{"0Game", "ModifiedMinecraft", "ModifiedDiamondTuber", "Modified123", "ModifiedDuck"}));
      QCOMPARE(*model.rowDataAt(1), (AccountInfo{"3Game", "Stardew Valley", "DanceingFire", "456", "emhh"}));
      QCOMPARE(*model.rowDataAt(2), (AccountInfo{}));
      QCOMPARE(*model.rowDataAt(3), (AccountInfo{"1Game", "1Name", "1Account", "1Pwd", "1Others"}));

      // 按照Type， 升序
      QCOMPARE(proxyModel.index(0, TYPE_COLUMN).data(Qt::DisplayRole).toString(), "");
      QCOMPARE(proxyModel.index(0, NAME_COLUMN).data(Qt::DisplayRole).toString(), "");
      QCOMPARE(proxyModel.index(0, ACCOUNT_COLUMN).data(Qt::DisplayRole).toString(), "");

      QCOMPARE(proxyModel.index(1, TYPE_COLUMN).data(Qt::DisplayRole).toString(), "0Game");
      QCOMPARE(proxyModel.index(1, NAME_COLUMN).data(Qt::DisplayRole).toString(), "ModifiedMinecraft");
      QCOMPARE(proxyModel.index(1, ACCOUNT_COLUMN).data(Qt::DisplayRole).toString(), "ModifiedDiamondTuber");

      QCOMPARE(proxyModel.index(2, TYPE_COLUMN).data(Qt::DisplayRole).toString(), "1Game");
      QCOMPARE(proxyModel.index(2, NAME_COLUMN).data(Qt::DisplayRole).toString(), "1Name");
      QCOMPARE(proxyModel.index(2, ACCOUNT_COLUMN).data(Qt::DisplayRole).toString(), "1Account");

      QCOMPARE(proxyModel.index(3, TYPE_COLUMN).data(Qt::DisplayRole).toString(), "3Game");
      QCOMPARE(proxyModel.index(3, NAME_COLUMN).data(Qt::DisplayRole).toString(), "Stardew Valley");
      QCOMPARE(proxyModel.index(3, ACCOUNT_COLUMN).data(Qt::DisplayRole).toString(), "DanceingFire");
    }
  }

  void editStage_fullSignalChain_ok() {
    QList<AccountInfo> accList{
                               AccountInfo{"0Game", "ModifiedMinecraft", "ModifiedDiamondTuber", "Modified123", "ModifiedDuck"},
                               AccountInfo{"3Game", "Stardew Valley", "DanceingFire", "456", "emhh"},
                               AccountInfo{"8PLACEHOLDER", "8PLACEHOLDER", "8PLACEHOLDER", "8PLACEHOLDER", "8PLACEHOLDER"},
                               AccountInfo{"1Game", "1Name", "1Account", "1Pwd", "1Others"},
                               };

    MOCKER(AccountFileHelper::ValidateAndLoad)
        .expects(exactly(1))
        .will(returnValue(AccountFileHelper::LoadResult{true,  accList}));

    AccountTableModel model{"Path/to/an/exist/accounts.enc.csv", "CORRECT_AES_KEY"};
    QCOMPARE(model.rowCount(), 4); // 4 行

    AccountSortFilterProxyModel proxyModel;
    proxyModel.setSourceModel(&model);
    proxyModel.initSortSetting(AccountInfo::Role::TYPE_STR_ROLE, false); // 按照Type， 升序

    AccountListView lv{&model, &proxyModel};
    AccountCardDelegate* pDelegates = lv.GetCardDelegate();
    QVERIFY(pDelegates != nullptr);

    QModelIndex src2Ind = model.index(2, 0);

    // 完整链路验证, 启用编辑， 修改 src2Ind, 暂存按钮触发stageCurrentChanges->QListView->QAbstractTableModel::setModel->setEditorData
    {
      lv.setCurrentIndex(proxyModel.mapFromSource(src2Ind));
      lv.edit(proxyModel.mapFromSource(src2Ind));           // 走 view 的编辑流程
      AccountCardEditor *cardEditor2 = qobject_cast<AccountCardEditor*>(lv.indexWidget(proxyModel.mapFromSource(src2Ind)));
      QVERIFY(cardEditor2 != nullptr);
      // 不需要主动setEditorData, 交给QAbstractItemView::edit自动处理
      QCOMPARE(cardEditor2->typeEdit()->text(), "8PLACEHOLDER");
      QCOMPARE(cardEditor2->nameEdit()->text(), "8PLACEHOLDER");
      QCOMPARE(cardEditor2->accountEdit()->text(), "8PLACEHOLDER");
      QCOMPARE(cardEditor2->pwdEdit()->text(), "8PLACEHOLDER");
      QCOMPARE(cardEditor2->othersEdit()->toPlainText(), "8PLACEHOLDER");

      cardEditor2->typeEdit()->setText("2Game");
      cardEditor2->nameEdit()->setText("2Name");
      cardEditor2->accountEdit()->setText("2Account");
      cardEditor2->pwdEdit()->setText("2Pwd");
      cardEditor2->othersEdit()->setPlainText("2Others");

      emit cardEditor2->stageCurrentChanges();
      QCOMPARE(lv.indexWidget(proxyModel.mapFromSource(src2Ind)), nullptr); // 编辑器界面要被正确关闭

      // AccountInfo{"0Game", "ModifiedMinecraft", "ModifiedDiamondTuber", "Modified123", "ModifiedDuck"},
      // AccountInfo{"3Game", "Stardew Valley", "DanceingFire", "456", "emhh"},
      // AccountInfo{"2Game", "2Name", "2Account", "2Pwd", "2Others"},
      // AccountInfo{"1Game", "1Name", "1Account", "1Pwd", "1Others"},
      //
      // 检查内存， 修改写入行正确
      QCOMPARE(*model.rowDataAt(0), (AccountInfo{"0Game", "ModifiedMinecraft", "ModifiedDiamondTuber", "Modified123", "ModifiedDuck"}));
      QCOMPARE(*model.rowDataAt(1), (AccountInfo{"3Game", "Stardew Valley", "DanceingFire", "456", "emhh"}));
      QCOMPARE(*model.rowDataAt(2), (AccountInfo{"2Game", "2Name", "2Account", "2Pwd", "2Others"}));
      QCOMPARE(*model.rowDataAt(3), (AccountInfo{"1Game", "1Name", "1Account", "1Pwd", "1Others"}));

      QCOMPARE(proxyModel.index(0, TYPE_COLUMN).data(Qt::DisplayRole).toString(), "0Game");
      QCOMPARE(proxyModel.index(0, NAME_COLUMN).data(Qt::DisplayRole).toString(), "ModifiedMinecraft");
      QCOMPARE(proxyModel.index(0, ACCOUNT_COLUMN).data(Qt::DisplayRole).toString(), "ModifiedDiamondTuber");

      QCOMPARE(proxyModel.index(1, TYPE_COLUMN).data(Qt::DisplayRole).toString(), "1Game");
      QCOMPARE(proxyModel.index(1, NAME_COLUMN).data(Qt::DisplayRole).toString(), "1Name");
      QCOMPARE(proxyModel.index(1, ACCOUNT_COLUMN).data(Qt::DisplayRole).toString(), "1Account");

      QCOMPARE(proxyModel.index(2, TYPE_COLUMN).data(Qt::DisplayRole).toString(), "2Game");
      QCOMPARE(proxyModel.index(2, NAME_COLUMN).data(Qt::DisplayRole).toString(), "2Name");
      QCOMPARE(proxyModel.index(2, ACCOUNT_COLUMN).data(Qt::DisplayRole).toString(), "2Account");

      QCOMPARE(proxyModel.index(3, TYPE_COLUMN).data(Qt::DisplayRole).toString(), "3Game");
      QCOMPARE(proxyModel.index(3, NAME_COLUMN).data(Qt::DisplayRole).toString(), "Stardew Valley");
      QCOMPARE(proxyModel.index(3, ACCOUNT_COLUMN).data(Qt::DisplayRole).toString(), "DanceingFire");
    }
  }

};

#include "AccountListViewTest.moc"
REGISTER_TEST(AccountListViewTest, false)