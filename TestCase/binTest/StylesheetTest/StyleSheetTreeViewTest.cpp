#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "StyleSheetTreeView.h"
#include "StyleSheetMgr.h"
#include "EndToExposePrivateMember.h"

#include "MemoryKey.h"
#include "PreferenceActions.h"
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
  void cleanupTestCase() {
    Configuration().clear(); //
  }

  void StyleSheetMgr_ok() {
    auto& prefInst = g_PreferenceActions();
    QVERIFY(prefInst.STYLESHEET_MGR->isCheckable());
    prefInst.STYLESHEET_MGR->setChecked(true);

    StyleSheetMgr mgr;
    mgr.showEvent(nullptr);
    mgr.hideEvent(nullptr);
    QVERIFY(prefInst.STYLESHEET_MGR->isChecked());

    QShowEvent eventShow;
    QHideEvent eventHide;
    mgr.showEvent(&eventShow);
    mgr.hideEvent(&eventHide);
    QVERIFY(!prefInst.STYLESHEET_MGR->isChecked());

    QVERIFY(mgr.m_searchLineEdit != nullptr);
    QVERIFY(mgr.m_startSearchAct != nullptr);
    QVERIFY(mgr.m_styleSheetView != nullptr);

    mgr.m_searchLineEdit->setText("Kaka");
    emit mgr.m_searchLineEdit->returnPressed();
    QCOMPARE(mgr.m_styleSheetView->curFilter(), "Kaka");

    mgr.m_searchLineEdit->setText("Cristinao Ronaldo");
    mgr.m_startSearchAct->trigger();
    QCOMPARE(mgr.m_styleSheetView->curFilter(), "Cristinao Ronaldo");

    mgr.m_searchLineEdit->setText("Varane");
    mgr.onStartFilter();
    QCOMPARE(mgr.m_styleSheetView->curFilter(), "Varane");
  }

  void onClearModifiedValues_ok() {
    MOCKER(PreferenceActions::ApplyNewStyleSheet).expects(never());

    StyleSheetTreeView view;
    QCOMPARE(view.m_defaultExpandAll, true);
    QCOMPARE(view.dragDropMode(), QAbstractItemView::NoDragDrop);
    QCOMPARE(view.onClearModifiedValues(), 0);
    QVERIFY(view.mStyleModel != nullptr);
    QVERIFY(view.mStyleModel->rowCount() > 0);

    // all modified value is not specified, no need clear
    view.selectAll();
    QCOMPARE(view.onClearModifiedValues(), 0);
  }

  void onRestoreToDefault_ok() {
    MOCKER(PreferenceActions::ApplyNewStyleSheet).expects(never());

    StyleSheetTreeView view;
    QCOMPARE(view.onRestoreToDefault(), 0);
    view.selectAll();

    const int rowsAffected = view.onRestoreToDefault();
    QVERIFY(rowsAffected > 0);
    QCOMPARE(view.onRestoreToDefault(), 0); // no need restore

    QCOMPARE(view.onClearModifiedValues(), rowsAffected);
    QCOMPARE(view.onClearModifiedValues(), 0); // already cleared
  }

  void onRestoreToBackup_ok() {
    MOCKER(PreferenceActions::ApplyNewStyleSheet).expects(never());

    StyleSheetTreeView view;
    QCOMPARE(view.onRestoreToBackup(), 0);
    view.selectAll();

    const int rowsAffected = view.onRestoreToBackup();
    QVERIFY(rowsAffected > 0);
    QCOMPARE(view.onRestoreToBackup(), 0); // no need restore

    QCOMPARE(view.onClearModifiedValues(), rowsAffected);
    QCOMPARE(view.onClearModifiedValues(), 0); // already cleared
  }

  void onBatchSetColor_ok() {
    MOCKER(PreferenceActions::ApplyNewStyleSheet).expects(never());

    StyleSheetTreeView view;
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
  }

  void onApplyChanges_ok() {
    Configuration().clear();

    MOCKER(PreferenceActions::ApplyNewStyleSheet).expects(exactly(1));
    StyleSheetTreeView view;
    QCOMPARE(view.onApplyChanges(), 0); // 无选中, 不调用

    view.selectAll();
    QCOMPARE(view.onApplyChanges(), 0); // 所有modifiedTo列内容均未指定, 不调用

    QVERIFY(view.onRestoreToBackup() > 0);
    QVERIFY(view.onApplyChanges() > 0); // 存在modifiedTo列内容已经指定, 需要更新已有配置, 调用一次Apply

    view.mApplyChanges->trigger();
    QCOMPARE(view.onApplyChanges(), 0); // 存在modifiedTo列内容已经指定, 但是无需更新已有配置, 不调用
  }

  void onRequestApplyChanges_ok() {
    QVERIFY(!Configuration().contains("StyleSheet/RandomKey"));
    MOCKER(PreferenceActions::ApplyNewStyleSheet).expects(exactly(2));

    StyleSheetTreeView view;
    // 默认关闭自动应用
    QVERIFY(view.mApplyInstantly != nullptr);
    QVERIFY(!view.mApplyInstantly->isChecked());
    QVERIFY(!view.mStyleModel->m_bInstantApply);

    // 开启自动应用
    view.mApplyInstantly->toggle();
    QVERIFY(view.mApplyInstantly->isChecked());
    QVERIFY(view.mStyleModel->m_bInstantApply);

    view.onRequestApplyChanges("StyleSheet/RandomKey", 124); // 调用第一次Apply
    QVERIFY(Configuration().contains("StyleSheet/RandomKey"));
    QCOMPARE(Configuration().value("StyleSheet/RandomKey").toInt(), 124);

    emit view.mStyleModel->requestApplyChanges("StyleSheet/RandomKey", "126"); // 调用第二次Apply
    QCOMPARE(Configuration().value("StyleSheet/RandomKey").toInt(), 126);

    // 关闭自动应用
    view.mStyleModel->onInstantApplySwitchChanged(false);
    QVERIFY(!view.mStyleModel->m_bInstantApply);
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
    auto r1 = r->appendRow(StyleTreeNode::create(StyleItemData{"1FontFamilyStringComboBox", "Microsoft YaHei", "Noto Sans", StyleItemData::DataTypeE::FONT_FAMILY}));
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
    }
  }
};

#include "StyleSheetTreeViewTest.moc"
REGISTER_TEST(StyleSheetTreeViewTest, false)
