#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "StyleSheetTreeView.h"
#include "EndToExposePrivateMember.h"

#include "StyleSheetGetter.h"
#include "MemoryKey.h"

#include <QLineEdit>
#include <QFontDialog>
#include <QColorDialog>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class StyleSheetTreeViewTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void init() { GlobalMockObject::reset(); }
  void cleanup() { GlobalMockObject::verify(); }
  void initTestCase() {
    Configuration().clear(); //
  }
  void cleanupTestCase() {
    Configuration().clear(); //
  }

  void onClearModifiedValues_ok() {
    StyleSheetTreeView view;
    QCOMPARE(view.m_defaultExpandAll, true);
    QCOMPARE(view.dragDropMode(), QAbstractItemView::NoDragDrop);
    QCOMPARE(view.onClearModifiedValues(), 0);
    QVERIFY(view.mStyleModel != nullptr);
    QVERIFY(view.mStyleModel->rowCount() > 0);
    QSignalSpy reqSeeChangesSpy{&view, &StyleSheetTreeView::reqSeeChanges};

    // all modified value is not specified, no need clear
    view.selectAll();
    QCOMPARE(view.onClearModifiedValues(), 0);
    QCOMPARE(reqSeeChangesSpy.count(), 0);
  }

  void onRestoreToDefault_ok() {
    StyleSheetTreeView view;
    QSignalSpy reqSeeChangesSpy{&view, &StyleSheetTreeView::reqSeeChanges};

    QCOMPARE(view.onRestoreToDefault(), 0);
    view.selectAll();

    const int rowsAffected = view.onRestoreToDefault();
    QVERIFY(rowsAffected > 0);
    QCOMPARE(view.onRestoreToDefault(), 0); // no need restore

    QCOMPARE(view.onClearModifiedValues(), rowsAffected);
    QCOMPARE(view.onClearModifiedValues(), 0); // already cleared

    QCOMPARE(reqSeeChangesSpy.count(), 0);
  }

  void onRestoreToBackup_ok() {
    StyleSheetTreeView view;
    QSignalSpy reqSeeChangesSpy{&view, &StyleSheetTreeView::reqSeeChanges};

    QCOMPARE(view.onRestoreToBackup(), 0);
    view.selectAll();

    const int rowsAffected = view.onRestoreToBackup();
    QVERIFY(rowsAffected > 0);
    QCOMPARE(view.onRestoreToBackup(), 0); // no need restore

    QCOMPARE(view.onClearModifiedValues(), rowsAffected);
    QCOMPARE(view.onClearModifiedValues(), 0); // already cleared
    QCOMPARE(reqSeeChangesSpy.count(), 0);
  }

  void onBatchSetColor_ok() {
    StyleSheetTreeView view;
    QSignalSpy reqSeeChangesSpy{&view, &StyleSheetTreeView::reqSeeChanges};
    QCOMPARE(view.onBatchSetColor(), 0);

    MOCKER(QColorDialog::getColor)            //
        .expects(exactly(3))                  //
        .will(returnValue(QColor{}))          //
        .then(returnValue(QColor{"#FF0000"})) //
        .then(returnValue(QColor{"#FF0000"}));
    view.selectAll();

    QCOMPARE(view.onBatchSetColor(), -1);
    QVERIFY(view.onBatchSetColor() > 0);  // 至少有1个Color类型的节点
    QVERIFY(view.onBatchSetColor() == 0); // 颜色没有变化
    QCOMPARE(reqSeeChangesSpy.count(), 0);
  }

  void onSeeChanges_ok() {
    StyleSheetGetter::GetInst().init();
    StyleSheetTreeView view;
    QSignalSpy reqSeeChangesSpy{&view, &StyleSheetTreeView::reqSeeChanges};
    QCOMPARE(view.onSeeChanges(), 0); // 无选中, 不调用

    view.selectAll();
    QCOMPARE(view.onSeeChanges(), 0); // 所有modifiedTo列内容均未指定, 不调用
    QCOMPARE(reqSeeChangesSpy.count(), 0);

    QVERIFY(view.onRestoreToBackup() > 0);
    QCOMPARE(view.onSeeChanges(), 0); // 存在modifiedTo列内容已经指定, 同StyleSheetGetter字典一致,
    QCOMPARE(reqSeeChangesSpy.count(), 0);

    view.mSeeChanges->trigger();
    QCOMPARE(view.onSeeChanges(), 0); // 存在modifiedTo列内容已经指定, 但是无需更新已有配置, 不调用
    QCOMPARE(reqSeeChangesSpy.count(), 0);
  }

  void onRequestApplyChanges_ok() {
    const auto& inst = StyleSheetGetter::GetInst();
    bool bKeyFind = false;
    inst.curValue("StyleSheet/Font/Size/General", &bKeyFind);
    QCOMPARE(bKeyFind, true);

    StyleSheetTreeView view;
    QSignalSpy reqSeeChangesSpy{&view, &StyleSheetTreeView::reqSeeChanges};

    // 默认关闭自动应用
    QVERIFY(view.mLivePreviewSwitch != nullptr);
    QVERIFY(!view.mLivePreviewSwitch->isChecked());
    QVERIFY(!view.mStyleModel->m_bLivePreviewSwitch);

    // 开启自动应用
    view.mLivePreviewSwitch->toggle();
    QVERIFY(view.mLivePreviewSwitch->isChecked());
    QVERIFY(view.mStyleModel->m_bLivePreviewSwitch);

    view.onRequestSeeChanges("StyleSheet/Font/Size/General", 124); // call reqSeeChanges 1st
    QCOMPARE(reqSeeChangesSpy.count(), 1);
    reqSeeChangesSpy.takeLast();
    bKeyFind = false;
    QCOMPARE(inst.curValue("StyleSheet/Font/Size/General", &bKeyFind), 124);
    QCOMPARE(bKeyFind, true);

    emit view.mStyleModel->requestSeeChanges("StyleSheet/Font/Size/General", "126"); // call reqSeeChanges 2nd
    QCOMPARE(reqSeeChangesSpy.count(), 1);
    reqSeeChangesSpy.takeLast();
    bKeyFind = false;
    QCOMPARE(inst.curValue("StyleSheet/Font/Size/General", &bKeyFind), 126);
    QCOMPARE(bKeyFind, true);

    // 关闭自动应用
    view.mStyleModel->onLivePreviewSwitchChanged(false);
    QVERIFY(!view.mStyleModel->m_bLivePreviewSwitch);
  }

  void onSetFontGeneral_ok() {
    QFont font0 = FontCfg::ReadGeneralFont();

    QString newFamily{"NewFontFamily"};
    int newPointSize = 99;
    QFont::Weight newWeight = QFont::Weight::Bold;
    QFont::Style newStyle = QFont::Style::StyleItalic;
    QFont font1{newFamily, newPointSize, newWeight, newStyle == QFont::Style::StyleItalic};

    std::pair<bool, QFont> cancel0{false, font0};
    std::pair<bool, QFont> acceptNotChanged{true, font0};
    std::pair<bool, QFont> acceptChanged2{true, font1};

    MOCKER(GetFontWithInitial)               //
        .expects(exactly(4))                 //
        // .with(any(), eq(nullptr), any())     // QFont自带的成员相等会比较没有明确设置的属性
        .will(returnValue(cancel0))          // 主动取消 -> -1
        .then(returnValue(acceptNotChanged)) // modifiedToValue原先为空, 修改为有效值, 视为变更 -> 4
        .then(returnValue(acceptNotChanged)) // modifiedToValue已经和font0一致, 无变更 -> 0
        .then(returnValue(acceptChanged2));  // modifiedToValue和font1有四项不一致 -> 4

    // 实时预览开启情况下, 无需调用 onSeeChanges, 就会刷新缓存值
    StyleSheetTreeView view;
    QCOMPARE(view.mLivePreviewSwitch->isChecked(), false);
    QCOMPARE(view.mStyleModel->m_bLivePreviewSwitch, false);
    view.mLivePreviewSwitch->toggle();
    QCOMPARE(view.mLivePreviewSwitch->isChecked(), true);
    QCOMPARE(view.mStyleModel->m_bLivePreviewSwitch, true);
    view.selectAll();
    view.onClearModifiedValues();

    QCOMPARE(view.onSetFontGeneral(), -1); // user cancelled
    QCOMPARE(view.onSetFontGeneral(), 4);  // modifiedToValue changed from invalid to valid
    QCOMPARE(view.onSetFontGeneral(), 0);  // no changes
    QCOMPARE(view.onSetFontGeneral(), 4);  // user accept font1

    QFont recoverFont1 = FontCfg::ReadGeneralFont();
    QVERIFY(FontCfg::isCoarseEqual(recoverFont1, font1));
  }

  void StyleSheetEditDelegate_ok() { //
    StyleSheetTreeView view;
    StyleSheetEditDelegate* delegate = view.mStyleSheetEditDelegate;
    QVERIFY(delegate != nullptr);
    StyleSheetTreeModel* srcModel = view.mStyleModel;
    QVERIFY(srcModel != nullptr);
    TreeFilterProxyModel<StyleTreeNode>* proxyModel = view.mStyleFilterProxyModel;
    QVERIFY(proxyModel != nullptr);

    std::unique_ptr<StyleTreeNode> rootNode{StyleTreeNode::NewTreeNodeRoot("StyleSheetInTest")};
    auto r = rootNode.get();
    auto r0 = r->appendRow(StyleTreeNode::create(StyleItemData{"0ViewRowHeightNumberLineEdit", 30, 60, StyleItemData::DataTypeE::NUMBER}));
    auto r1 = r->appendRow(StyleTreeNode::create(StyleItemData{"1FontFamilyStringComboBox", "Microsoft YaHei UI", "Noto Sans", StyleItemData::DataTypeE::FONT_FAMILY}));
    auto r2 = r->appendRow(StyleTreeNode::create(StyleItemData{"2FontWeightEnumComboBox", QFont::Weight::Normal, QFont::Weight::Bold, StyleItemData::DataTypeE::FONT_WEIGHT}));
    auto r3 = r->appendRow(StyleTreeNode::create(StyleItemData{"3FontStyleEnumComboBox", QFont::Style::StyleItalic, QFont::Style::StyleNormal, StyleItemData::DataTypeE::FONT_STYLE}));
    auto r4 = r->appendRow(StyleTreeNode::create(StyleItemData{"4FontForegroundColorLineEditAndAction", "#FF0000", "#FF00FF", StyleItemData::DataTypeE::COLOR}));
    auto r5 = r->appendRow(StyleTreeNode::create(StyleItemData{"5Group"}));

    QVERIFY(srcModel->setDatas(std::move(rootNode)));
    proxyModel->setSortRole(Qt::DisplayRole);
    proxyModel->sort(StyleItemData::SORT_COLUMN, Qt::AscendingOrder);

    view.selectAll();
    const QModelIndexList srcIndexes = view.selectedRowsSource();
    QCOMPARE(srcIndexes.size(), 6);
    QCOMPARE(srcIndexes[0].data(), "0ViewRowHeightNumberLineEdit");
    QCOMPARE(srcIndexes[1].data(), "1FontFamilyStringComboBox");
    QCOMPARE(srcIndexes[2].data(), "2FontWeightEnumComboBox");
    QCOMPARE(srcIndexes[3].data(), "3FontStyleEnumComboBox");
    QCOMPARE(srcIndexes[4].data(), "4FontForegroundColorLineEditAndAction");
    QCOMPARE(srcIndexes[5].data(), "5Group");
    const QModelIndex r0Index3 = srcModel->siblingAtColumn(srcIndexes[0], StyleItemData::EDITABLE_COLUMN);
    const QModelIndex r1Index3 = srcModel->siblingAtColumn(srcIndexes[1], StyleItemData::EDITABLE_COLUMN);
    const QModelIndex r2Index3 = srcModel->siblingAtColumn(srcIndexes[2], StyleItemData::EDITABLE_COLUMN);
    const QModelIndex r3Index3 = srcModel->siblingAtColumn(srcIndexes[3], StyleItemData::EDITABLE_COLUMN);
    const QModelIndex r4Index3 = srcModel->siblingAtColumn(srcIndexes[4], StyleItemData::EDITABLE_COLUMN);
    const QModelIndex r5Index3 = srcModel->siblingAtColumn(srcIndexes[5], StyleItemData::EDITABLE_COLUMN);

    QVERIFY(delegate->mFontFamilyItems.size() > 0);
    QVERIFY(delegate->mFontWeightItems.size() > 0);
    QVERIFY(delegate->mFontStyleItems.size() > 0);

    delegate->updateEditorGeometry(nullptr, QStyleOptionViewItem{}, QModelIndex{});

    {
      QWidget* viewRowHeightNumberLineEdit = delegate->createEditor(&view, QStyleOptionViewItem{}, r0Index3);
      QLineEdit* rowHeightLineEdit = qobject_cast<QLineEdit*>(viewRowHeightNumberLineEdit);
      QVERIFY(rowHeightLineEdit != nullptr);
      QCOMPARE(rowHeightLineEdit->actions().size(), 0);

      rowHeightLineEdit->setText("120");

      delegate->setModelData(rowHeightLineEdit, srcModel, r0Index3);
      const StyleItemData& r0ItemData = r0->value();
      QCOMPARE(r0Index3.data(Qt::DisplayRole), "120");
      QCOMPARE(r0Index3.data(Qt::EditRole), "120");
      QCOMPARE(r0ItemData.modifiedToValue.toInt(), 120);

      rowHeightLineEdit->setText("invalid row height number");          // 非数值
      QCOMPARE(rowHeightLineEdit->text(), "invalid row height number"); // 直接从模型中读取EditRole
      delegate->setEditorData(rowHeightLineEdit, r0Index3);
      QCOMPARE(r0Index3.data(Qt::DisplayRole), "120");
      QCOMPARE(r0Index3.data(Qt::EditRole), "120");
      QCOMPARE(r0ItemData.modifiedToValue, 120);
      QCOMPARE(rowHeightLineEdit->text(), "120");
    }

    {
      QWidget* fontFamilyStringComboBox = delegate->createEditor(&view, QStyleOptionViewItem{}, r1Index3);
      QComboBox* fontFamilyCb = qobject_cast<QComboBox*>(fontFamilyStringComboBox);
      QVERIFY(fontFamilyCb != nullptr);

      QCOMPARE(fontFamilyCb->count(), delegate->mFontFamilyItems.size());
      QString newFontFamilyStr = delegate->mFontFamilyItems[0];
      fontFamilyCb->setCurrentText(newFontFamilyStr);
      delegate->setModelData(fontFamilyCb, srcModel, r1Index3);

      const StyleItemData& r1ItemData = r1->value();
      QCOMPARE(r1Index3.data(Qt::DisplayRole), newFontFamilyStr);
      QCOMPARE(r1Index3.data(Qt::EditRole), newFontFamilyStr);
      QCOMPARE(r1ItemData.modifiedToValue, newFontFamilyStr);

      fontFamilyCb->setCurrentText("inexist font family"); // 不在候选列表中
      delegate->setEditorData(fontFamilyCb, r1Index3);     // 直接从模型中读取EditRole
      QCOMPARE(r1Index3.data(Qt::DisplayRole), newFontFamilyStr);
      QCOMPARE(r1Index3.data(Qt::EditRole), newFontFamilyStr);
      QCOMPARE(r1ItemData.modifiedToValue, newFontFamilyStr);
      QCOMPARE(fontFamilyCb->currentText(), newFontFamilyStr);
    }

    {
      QWidget* fontWeightEnumComboBox = delegate->createEditor(&view, QStyleOptionViewItem{}, r2Index3);
      QComboBox* fontWeightCb = qobject_cast<QComboBox*>(fontWeightEnumComboBox);
      QVERIFY(fontWeightCb != nullptr);
      QCOMPARE(fontWeightCb->count(), delegate->mFontWeightItems.size());

      const StyleItemData& r2ItemData = r2->value();
      QString newFontWeightStr = fontWeightCb->currentText();
      fontWeightCb->setCurrentText(newFontWeightStr);
      QFont::Weight newFontWeightInt = delegate->mFontWeightItems.value(newFontWeightStr);
      delegate->setModelData(fontWeightCb, srcModel, r2Index3);

      QCOMPARE(r2Index3.data(Qt::DisplayRole), newFontWeightInt);
      QCOMPARE(r2Index3.data(Qt::EditRole), newFontWeightInt);
      QCOMPARE(r2ItemData.modifiedToValue, newFontWeightInt);

      fontWeightCb->setCurrentText("inexist font weight"); // 不在候选列表中, 清空
      delegate->setEditorData(fontWeightCb, r2Index3);     // 直接从模型中读取EditRole
      QCOMPARE(r2Index3.data(Qt::DisplayRole), newFontWeightInt);
      QCOMPARE(r2Index3.data(Qt::EditRole), newFontWeightInt);
      QCOMPARE(r2ItemData.modifiedToValue, newFontWeightInt);
      QVERIFY(fontWeightCb->currentText() != ""); // "Black" "Normal" etc.
    }

    {
      QWidget* fontStyleEnumComboBox = delegate->createEditor(&view, QStyleOptionViewItem{}, r3Index3);
      QComboBox* fontStyleCb = qobject_cast<QComboBox*>(fontStyleEnumComboBox);
      QVERIFY(fontStyleCb != nullptr);
      QCOMPARE(fontStyleCb->count(), delegate->mFontStyleItems.size());

      const StyleItemData& r3ItemData = r3->value();
      QString newFontStyleStr = fontStyleCb->currentText();
      fontStyleCb->setCurrentText(newFontStyleStr);
      QFont::Style newFontStyleInt = delegate->mFontStyleItems.value(newFontStyleStr);
      delegate->setModelData(fontStyleCb, srcModel, r3Index3);

      QCOMPARE(r3Index3.data(Qt::DisplayRole), newFontStyleInt);
      QCOMPARE(r3Index3.data(Qt::EditRole), newFontStyleInt);
      QCOMPARE(r3ItemData.modifiedToValue, newFontStyleInt);

      fontStyleCb->setCurrentText("inexist font style"); // 不在候选列表中, 清空
      delegate->setEditorData(fontStyleCb, r3Index3);    // 直接从模型中读取EditRole
      QCOMPARE(r3Index3.data(Qt::DisplayRole), newFontStyleInt);
      QCOMPARE(r3Index3.data(Qt::EditRole), newFontStyleInt);
      QCOMPARE(r3ItemData.modifiedToValue, newFontStyleInt);
      QVERIFY(fontStyleCb->currentText() != ""); // "StyleItalic" "StyleNormal"
    }

    { // color lineedit with action
      QWidget* fontForegroundColorLineEditAndAction = delegate->createEditor(&view, QStyleOptionViewItem{}, r4Index3);
      QLineEdit* colorLineEdit = qobject_cast<QLineEdit*>(fontForegroundColorLineEditAndAction);
      QVERIFY(colorLineEdit != nullptr);
      QCOMPARE(colorLineEdit->text(), "");
      colorLineEdit->setText("invalidColorStr");
      QCOMPARE(colorLineEdit->text(), "invalidColorStr");

      QList<QAction*> acts = colorLineEdit->actions();
      QCOMPARE(acts.size(), 1);
      // 点击输出两次
      MOCKER(QColorDialog::getColor)             //
          .expects(exactly(2))                   //
          .will(returnValue(QColor{}))           //
          .then(returnValue(QColor{"#123456"})); // name(QColor::HexArgb) 转化为8位

      QAction* colorAction = acts.front();
      QVERIFY(colorAction != nullptr);
      colorAction->trigger(); // 第一次
      QCOMPARE(colorLineEdit->text(), "invalidColorStr");

      colorAction->trigger(); // 第二次
      QCOMPARE(colorLineEdit->text().toUpper(), "#FF123456");

      const StyleItemData& r4ItemData = r4->value();
      // 这里暂时没有调用setData
      QCOMPARE(r4Index3.data(Qt::DisplayRole), QVariant());
      QCOMPARE(r4Index3.data(Qt::EditRole), QVariant());
      QCOMPARE(r4ItemData.modifiedToValue, QVariant());

      // 调用setData
      delegate->setModelData(colorLineEdit, srcModel, r4Index3);
      QCOMPARE(r4Index3.data(Qt::DisplayRole).toString().toUpper(), "#FF123456");
      QCOMPARE(r4Index3.data(Qt::EditRole).toString().toUpper(), "#FF123456");
      QCOMPARE(r4ItemData.modifiedToValue.toString().toUpper(), "#FF123456");

      // 手动设置#6位
      colorLineEdit->setText("#FFFFFF");
      delegate->setModelData(colorLineEdit, srcModel, r4Index3);
      QCOMPARE(r4Index3.data(Qt::DisplayRole).toString().toUpper(), "#FFFFFF");
      QCOMPARE(r4Index3.data(Qt::EditRole).toString().toUpper(), "#FFFFFF");
      QCOMPARE(r4ItemData.modifiedToValue.toString().toUpper(), "#FFFFFF");

      // 手动设置空字符串"", setEditorData不会修改r4ItemData
      colorLineEdit->setText("");
      delegate->setEditorData(colorLineEdit, r4Index3); // 直接从模型中读取EditRole
      QCOMPARE(r4Index3.data(Qt::DisplayRole).toString().toUpper(), "#FFFFFF");
      QCOMPARE(r4Index3.data(Qt::EditRole).toString().toUpper(), "#FFFFFF");
      QCOMPARE(r4ItemData.modifiedToValue.toString().toUpper(), "#FFFFFF");
      QCOMPARE(colorLineEdit->text(), "#FFFFFF");
    }
  }
};

#include "StyleSheetTreeViewTest.moc"
REGISTER_TEST(StyleSheetTreeViewTest, false)
