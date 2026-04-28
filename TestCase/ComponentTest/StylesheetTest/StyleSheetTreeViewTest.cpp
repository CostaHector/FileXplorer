#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "StyleSheetTreeView.h"
#include "ComboBoxGeneral.h"
#include "ComboBoxString.h"
#include "ComboBoxEnum.h"
#include "EndToExposePrivateMember.h"

#include "StyleSheetGetter.h"
#include "Configuration.h"
#include "InputDialogHelper.h"
#include "PublicVariable.h"
#include "PathTool.h"
#include <QLineEdit>
#include <QPlainTextEdit>
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

    MOCKER(InputDialogHelper::GetFontWithInitial)   //
        .expects(exactly(4))                        //
        .with(any(), eq((QWidget*) nullptr), any()) // QFont自带的成员相等会比较没有明确设置的属性
        .will(returnValue(cancel0))                 // 主动取消 -> -1
        .then(returnValue(acceptNotChanged))        // modifiedToValue原先为空, 修改为有效值, 视为变更 -> 4
        .then(returnValue(acceptNotChanged))        // modifiedToValue已经和font0一致, 无变更 -> 0
        .then(returnValue(acceptChanged2));         // modifiedToValue和font1有四项不一致 -> 4

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
    auto r0 = r->appendRow(StyleTreeNode::create(StyleItemData{"0ViewRowHeightNumberLineEdit", 30, 60, GeneralDataType::Type::PLAIN_INT}));
    auto r1 = r->appendRow(StyleTreeNode::create(StyleItemData{"1FontFamilyComboBoxString", "Microsoft YaHei UI", "Noto Sans", GeneralDataType::Type::FONT_FAMILY}));
    auto r2 = r->appendRow(StyleTreeNode::create(StyleItemData{"2FontWeightComboBoxEnum", QFont::Weight::Normal, QFont::Weight::Bold, GeneralDataType::Type::FONT_WEIGHT}));
    auto r3 = r->appendRow(StyleTreeNode::create(StyleItemData{"3FontStyleComboBoxEnum", QFont::Style::StyleItalic, QFont::Style::StyleNormal, GeneralDataType::Type::FONT_STYLE}));
    auto r4 = r->appendRow(StyleTreeNode::create(StyleItemData{"4FontForegroundColorLineEditAndAction", "#FF0000", "#FF00FF", GeneralDataType::Type::COLOR}));
    auto r5 = r->appendRow(StyleTreeNode::create(StyleItemData{"5BackgroundImageFileLineEditAndAction", ":DefImg0", ":CurImg0", GeneralDataType::Type::IMAGE_PATH_OPTIONAL}));
    auto r6 = r->appendRow(StyleTreeNode::create(StyleItemData{"6FolderLineEditAndAction", SystemPath::HOME_PATH(), SystemPath::HOME_PATH(), GeneralDataType::Type::FOLDER_PATH}));
    auto r7 = r->appendRow(StyleTreeNode::create(StyleItemData{"7PlainTextEdit", "line0\nline1", "line0\nline1", GeneralDataType::Type::MULTI_LINE_STR}));
    auto r8 = r->appendRow(StyleTreeNode::create(StyleItemData{"8Group"}));

    QVERIFY(srcModel->setDatas(std::move(rootNode)));
    proxyModel->setSortRole(Qt::DisplayRole);
    proxyModel->sort(StyleItemData::SORT_COLUMN, Qt::AscendingOrder);

    view.selectAll();
    const QModelIndexList srcIndexes = view.selectedRowsSource();
    QCOMPARE(srcIndexes.size(), 9);
    QCOMPARE(srcIndexes[0].data(), "0ViewRowHeightNumberLineEdit");
    QCOMPARE(srcIndexes[1].data(), "1FontFamilyComboBoxString");
    QCOMPARE(srcIndexes[2].data(), "2FontWeightComboBoxEnum");
    QCOMPARE(srcIndexes[3].data(), "3FontStyleComboBoxEnum");
    QCOMPARE(srcIndexes[4].data(), "4FontForegroundColorLineEditAndAction");
    QCOMPARE(srcIndexes[5].data(), "5BackgroundImageFileLineEditAndAction");
    QCOMPARE(srcIndexes[6].data(), "6FolderLineEditAndAction");
    QCOMPARE(srcIndexes[7].data(), "7PlainTextEdit");
    QCOMPARE(srcIndexes[8].data(), "8Group");
    const QModelIndex r0Index3 = srcModel->siblingAtColumn(srcIndexes[0], StyleItemData::EDITABLE_COLUMN);
    const QModelIndex r1Index3 = srcModel->siblingAtColumn(srcIndexes[1], StyleItemData::EDITABLE_COLUMN);
    const QModelIndex r2Index3 = srcModel->siblingAtColumn(srcIndexes[2], StyleItemData::EDITABLE_COLUMN);
    const QModelIndex r3Index3 = srcModel->siblingAtColumn(srcIndexes[3], StyleItemData::EDITABLE_COLUMN);
    const QModelIndex r4Index3 = srcModel->siblingAtColumn(srcIndexes[4], StyleItemData::EDITABLE_COLUMN);
    const QModelIndex r5Index3 = srcModel->siblingAtColumn(srcIndexes[5], StyleItemData::EDITABLE_COLUMN);
    const QModelIndex r6Index3 = srcModel->siblingAtColumn(srcIndexes[6], StyleItemData::EDITABLE_COLUMN);
    const QModelIndex r7Index3 = srcModel->siblingAtColumn(srcIndexes[7], StyleItemData::EDITABLE_COLUMN);
    const QModelIndex r8Index3 = srcModel->siblingAtColumn(srcIndexes[8], StyleItemData::EDITABLE_COLUMN);

    const QStringList* const mFontFamilyItems = ComboBoxString::GetCandidates(GeneralDataType::Type::FONT_FAMILY);
    const QMap<QString, int>* const mFallbackOkItems = ComboBoxEnum::GetCandidates(GeneralDataType::Type::PLAIN_INT).first;
    const QMap<QString, int>* const mFontWeightItems = ComboBoxEnum::GetCandidates(GeneralDataType::Type::FONT_WEIGHT).first;
    const QMap<QString, int>* const mFontStyleItems = ComboBoxEnum::GetCandidates(GeneralDataType::Type::FONT_STYLE).first;
    QVERIFY(mFontFamilyItems != nullptr);
    QVERIFY(mFallbackOkItems != nullptr);
    QVERIFY(mFontWeightItems != nullptr);
    QVERIFY(mFontStyleItems != nullptr);

    QVERIFY(mFontFamilyItems->size() > 0);
    QVERIFY(mFontWeightItems->size() > 0);
    QVERIFY(mFontStyleItems->size() > 0);

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
      QWidget* fontFamilyComboBoxString = delegate->createEditor(&view, QStyleOptionViewItem{}, r1Index3);
      QComboBox* fontFamilyCb = qobject_cast<QComboBox*>(fontFamilyComboBoxString);
      QVERIFY(fontFamilyCb != nullptr);

      QCOMPARE(fontFamilyCb->count(), mFontFamilyItems->size());
      QString newFontFamilyStr = (*mFontFamilyItems)[0];
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
      QWidget* fontWeightComboBoxEnum = delegate->createEditor(&view, QStyleOptionViewItem{}, r2Index3);
      QComboBox* fontWeightCb = qobject_cast<QComboBox*>(fontWeightComboBoxEnum);
      QVERIFY(fontWeightCb != nullptr);
      QCOMPARE(fontWeightCb->count(), mFontWeightItems->size());

      const StyleItemData& r2ItemData = r2->value();
      QString newFontWeightStr = fontWeightCb->currentText();
      fontWeightCb->setCurrentText(newFontWeightStr);
      QFont::Weight newFontWeightInt = static_cast<QFont::Weight>(mFontWeightItems->value(newFontWeightStr));
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
      QWidget* fontStyleComboBoxEnum = delegate->createEditor(&view, QStyleOptionViewItem{}, r3Index3);
      QComboBox* fontStyleCb = qobject_cast<QComboBox*>(fontStyleComboBoxEnum);
      QVERIFY(fontStyleCb != nullptr);
      QCOMPARE(fontStyleCb->count(), mFontStyleItems->size());

      const StyleItemData& r3ItemData = r3->value();
      QString newFontStyleStr = fontStyleCb->currentText();
      fontStyleCb->setCurrentText(newFontStyleStr);
      QFont::Style newFontStyleInt = static_cast<QFont::Style>(mFontStyleItems->value(newFontStyleStr));
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

    { // color chooser lineedit with action
      QWidget* fontForegroundColorLineEditAndAction = delegate->createEditor(&view, QStyleOptionViewItem{}, r4Index3);
      QLineEdit* colorLineEdit = qobject_cast<QLineEdit*>(fontForegroundColorLineEditAndAction);
      QVERIFY(colorLineEdit != nullptr);
      QCOMPARE(colorLineEdit->text(), "");
      colorLineEdit->setText("#FF123456");

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

      // 手动设置任意字符串或者空字符串"", setEditorData从模型中读取EditRole, 直接覆盖掉
      colorLineEdit->setText("");
      QCOMPARE(colorLineEdit->text(), "");
      delegate->setEditorData(colorLineEdit, r4Index3);
      QCOMPARE(colorLineEdit->text(), "#FFFFFF");
      QCOMPARE(r4Index3.data(Qt::DisplayRole).toString().toUpper(), "#FFFFFF");
      QCOMPARE(r4Index3.data(Qt::EditRole).toString().toUpper(), "#FFFFFF");
      QCOMPARE(r4ItemData.modifiedToValue.toString().toUpper(), "#FFFFFF");

      // 手动设置空字符串""(接受, 有变更), setModelData重置 modifiedToValue
      colorLineEdit->setText("");
      delegate->setModelData(colorLineEdit, srcModel, r4Index3);
      QCOMPARE(r4Index3.data(Qt::DisplayRole), (QVariant{}));
      QCOMPARE(r4Index3.data(Qt::EditRole), (QVariant{}));
      QCOMPARE(r4ItemData.modifiedToValue, (QVariant{}));
    }

    { // file chooser lineedit with action
      QWidget* fileChooserLineEditAndAction = delegate->createEditor(&view, QStyleOptionViewItem{}, r5Index3);
      QLineEdit* fileLineEdit = qobject_cast<QLineEdit*>(fileChooserLineEditAndAction);
      QVERIFY(fileLineEdit != nullptr);
      QCOMPARE(fileLineEdit->text(), "");

      const QString filePathInLineEdit{__FILE__};
      fileLineEdit->setText(filePathInLineEdit);

      const StyleItemData& r5ItemData = r5->value();
      // 这里暂时没有调用setData
      QCOMPARE(r5Index3.data(Qt::DisplayRole), QVariant());
      QCOMPARE(r5Index3.data(Qt::EditRole), QVariant());
      QCOMPARE(r5ItemData.modifiedToValue, QVariant());

      // 调用setData
      delegate->setModelData(fileLineEdit, srcModel, r5Index3);
      QCOMPARE(r5Index3.data(Qt::DisplayRole).toString(), filePathInLineEdit);
      QCOMPARE(r5Index3.data(Qt::EditRole).toString(), filePathInLineEdit);
      QCOMPARE(r5ItemData.modifiedToValue.toString(), filePathInLineEdit);

      // 手动设置非文件值, 修改不生效
      fileLineEdit->setText("Not/existed/File");
      delegate->setModelData(fileLineEdit, srcModel, r5Index3);
      QCOMPARE(r5Index3.data(Qt::DisplayRole).toString(), filePathInLineEdit);
      QCOMPARE(r5Index3.data(Qt::EditRole).toString(), filePathInLineEdit);
      QCOMPARE(r5ItemData.modifiedToValue.toString(), filePathInLineEdit);

      // 手动设置任意字符串或者空字符串"", setEditorData从模型中读取EditRole, 直接覆盖掉
      fileLineEdit->setText("");
      QCOMPARE(fileLineEdit->text(), "");
      delegate->setEditorData(fileLineEdit, r5Index3);
      QCOMPARE(fileLineEdit->text(), filePathInLineEdit);
      QCOMPARE(r5Index3.data(Qt::DisplayRole).toString(), filePathInLineEdit);
      QCOMPARE(r5Index3.data(Qt::EditRole).toString(), filePathInLineEdit);
      QCOMPARE(r5ItemData.modifiedToValue.toString(), filePathInLineEdit);

      // 手动设置空字符串""(接受, 有变更), setModelData重置 modifiedToValue
      fileLineEdit->setText("");
      delegate->setModelData(fileLineEdit, srcModel, r5Index3);
      QCOMPARE(r5ItemData.modifiedToValue, (QVariant{}));
      QCOMPARE(r5Index3.data(Qt::DisplayRole), (QVariant{}));
      QCOMPARE(r5Index3.data(Qt::EditRole), (QVariant{}));
    }

    { // folder chooser lineedit with action 预期几乎与 file chooser完全一致
      QWidget* folderChooserLineEditAndAction = delegate->createEditor(&view, QStyleOptionViewItem{}, r6Index3);
      QLineEdit* folderLineEdit = qobject_cast<QLineEdit*>(folderChooserLineEditAndAction);
      QVERIFY(folderLineEdit != nullptr);
      QCOMPARE(folderLineEdit->text(), "");

      const QString folderPathInLineEdit{QFileInfo{__FILE__}.absolutePath()};
      folderLineEdit->setText(folderPathInLineEdit);

      const StyleItemData& r6ItemData = r6->value();
      // 这里暂时没有调用setData
      QCOMPARE(r6Index3.data(Qt::DisplayRole), QVariant());
      QCOMPARE(r6Index3.data(Qt::EditRole), QVariant());
      QCOMPARE(r6ItemData.modifiedToValue, QVariant());

      // 调用setData
      delegate->setModelData(folderLineEdit, srcModel, r6Index3);
      QCOMPARE(r6Index3.data(Qt::DisplayRole).toString(), folderPathInLineEdit);
      QCOMPARE(r6Index3.data(Qt::EditRole).toString(), folderPathInLineEdit);
      QCOMPARE(r6ItemData.modifiedToValue.toString(), folderPathInLineEdit);

      // 手动设置非文件夹值, 修改不生效
      folderLineEdit->setText("Not/existed/Folder");
      delegate->setModelData(folderLineEdit, srcModel, r6Index3);
      QCOMPARE(r6Index3.data(Qt::DisplayRole).toString(), folderPathInLineEdit);
      QCOMPARE(r6Index3.data(Qt::EditRole).toString(), folderPathInLineEdit);
      QCOMPARE(r6ItemData.modifiedToValue.toString(), folderPathInLineEdit);
    }

    {
      QWidget* multiLinePlainTextEdit = delegate->createEditor(&view, QStyleOptionViewItem{}, r7Index3);
      QPlainTextEdit* mutilineEdit = qobject_cast<QPlainTextEdit*>(multiLinePlainTextEdit);
      QVERIFY(mutilineEdit != nullptr);
      QCOMPARE(mutilineEdit->actions().size(), 0);

      mutilineEdit->setPlainText("a\nb\nc");
      QCOMPARE(mutilineEdit->toPlainText(), "a\nb\nc");

      delegate->setModelData(mutilineEdit, srcModel, r7Index3);
      const StyleItemData& r7ItemData = r7->value();
      QCOMPARE(r7Index3.data(Qt::DisplayRole), "a\nb\nc");
      QCOMPARE(r7Index3.data(Qt::EditRole), "a\nb\nc");
      QCOMPARE(r7ItemData.modifiedToValue.toString(), "a\nb\nc");

      // mutilineEdit contents remains;
      mutilineEdit->setPlainText("d\ne\nf");
      QCOMPARE(mutilineEdit->toPlainText(), "d\ne\nf");
      delegate->setEditorData(mutilineEdit, r7Index3);
      QCOMPARE(mutilineEdit->toPlainText(), "a\nb\nc");
    }
  }
};

#include "StyleSheetTreeViewTest.moc"
REGISTER_TEST(StyleSheetTreeViewTest, false)
