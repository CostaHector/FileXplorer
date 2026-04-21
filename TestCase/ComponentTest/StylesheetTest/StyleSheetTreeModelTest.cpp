#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "StyleSheetTreeModel.h"
#include "EndToExposePrivateMember.h"

#include <QSignalSpy>

std::unique_ptr<StyleTreeNode> GetRootNodeForTest(StyleTreeNode*& r0, //
                                                  /* -- */ StyleTreeNode*& r00,
                                                  /* ---- */ StyleTreeNode*& r000,
                                                  /* ---- */ StyleTreeNode*& r001,
                                                  /* ---- */ StyleTreeNode*& r002,
                                                  /* -- */ StyleTreeNode*& r01,
                                                  /* ---- */ StyleTreeNode*& r010,
                                                  /* ---- */ StyleTreeNode*& r011) {
  std::unique_ptr<StyleTreeNode> rootNode{StyleTreeNode::NewTreeNodeRoot("StyleSheetInTest")};
  r0 = rootNode.get();

  r00 = rootNode->appendRow(StyleTreeNode::create(StyleItemData{"View"}));
  r000 = r00->appendRow(StyleTreeNode::create(StyleItemData{"RowHeight", 30, 60, StyleItemData::DataTypeE::NUMBER}));
  r001 = r00->appendRow(StyleTreeNode::create(StyleItemData{"FontFamily", "Microsoft YaHei", "Microsoft YaHei", StyleItemData::DataTypeE::FONT_FAMILY}));
  r002 = r00->appendRow(StyleTreeNode::create(StyleItemData{"AlternateRowColor", "#4F4F4F", "#5E5E5E", StyleItemData::DataTypeE::COLOR}));

  r01 = rootNode->appendRow(StyleTreeNode::create(StyleItemData{"Font"}));
  r010 = r01->appendRow(StyleTreeNode::create(StyleItemData{"Foreground", "#000000", "#FFFFFF", StyleItemData::DataTypeE::COLOR}));
  r011 = r01->appendRow(StyleTreeNode::create(StyleItemData{"Weight", QFont::Weight::Normal, QFont::Weight::Bold, StyleItemData::DataTypeE::FONT_WEIGHT}));
  return rootNode;
}

class StyleSheetTreeModelTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void headers_ok() {
    StyleSheetTreeModel defModel;
    QVERIFY(defModel.rowCount() > 0);
    QCOMPARE(defModel.columnCount(), StyleItemData::COLUMN_COUNT);
    QVERIFY(StyleItemData::COLUMN_COUNT > 0);
    QCOMPARE(defModel.headerData(0, Qt::Orientation::Horizontal, Qt::DisplayRole).toString(), "Name");

    // headerData行号/列号默认行为: 从1开始计数
    QCOMPARE(defModel.headerData(998, Qt::Orientation::Horizontal, Qt::DisplayRole).toInt(), 998 + 1); // not crash
    QCOMPARE(defModel.headerData(0, Qt::Orientation::Vertical, Qt::DisplayRole).toInt(), 0 + 1);

    // nullptr will not crash down
    std::unique_ptr<StyleTreeNode> pRoot{nullptr};
    QVERIFY(!defModel.initFontRelated(pRoot));
    QVERIFY(!defModel.initColorRelated(pRoot, Style::StyleSheetE::STYLESHEET_LIGHT));

    QCOMPARE(defModel.editCellFailed({}), false);
    QCOMPARE(defModel.editCellEraseIndex({}), false);
  }

  void data_ok() {
    StyleTreeNode                                                  //
        *r0{nullptr},                                              //
        /* -- */ *r00{nullptr},                                    //
        /* ---- */ *r000{nullptr}, *r001{nullptr}, *r002{nullptr}, //
        /* -- */ *r01{nullptr},                                    //
        /* ---- */ *r010{nullptr}, *r011{nullptr};                 //

    std::unique_ptr<StyleTreeNode> rootNode = GetRootNodeForTest(r0, r00, r000, r001, r002, r01, r010, r011);

    StyleSheetTreeModel model;
    model.setDatas(std::move(rootNode));

    QModelIndex r00Index0{model.index(0, StyleItemData::NAME_COLUMN, {})};
    QModelIndex r000Index0{model.index(0, StyleItemData::NAME_COLUMN, r00Index0)};
    QModelIndex r001Index0{model.index(1, StyleItemData::NAME_COLUMN, r00Index0)};
    QModelIndex r002Index0{model.index(2, StyleItemData::NAME_COLUMN, r00Index0)};
    QModelIndex r01Index0{model.index(1, StyleItemData::NAME_COLUMN, {})};
    QModelIndex r010Index0{model.index(0, StyleItemData::NAME_COLUMN, r01Index0)};
    QModelIndex r011Index0{model.index(1, StyleItemData::NAME_COLUMN, r01Index0)};

    QModelIndex r00Index1{model.index(0, StyleItemData::DEF_COLUMN, {})};
    QModelIndex r000Index1{model.index(0, StyleItemData::DEF_COLUMN, r00Index0)};
    QModelIndex r001Index1{model.index(1, StyleItemData::DEF_COLUMN, r00Index0)};
    QModelIndex r002Index1{model.index(2, StyleItemData::DEF_COLUMN, r00Index0)};
    QModelIndex r01Index1{model.index(1, StyleItemData::DEF_COLUMN, {})};
    QModelIndex r010Index1{model.index(0, StyleItemData::DEF_COLUMN, r01Index0)};
    QModelIndex r011Index1{model.index(1, StyleItemData::DEF_COLUMN, r01Index0)};

    QModelIndex r00Index2{model.index(0, StyleItemData::CUR_COLUMN, {})};
    QModelIndex r000Index2{model.index(0, StyleItemData::CUR_COLUMN, r00Index0)};
    QModelIndex r001Index2{model.index(1, StyleItemData::CUR_COLUMN, r00Index0)};
    QModelIndex r002Index2{model.index(2, StyleItemData::CUR_COLUMN, r00Index0)};
    QModelIndex r01Index2{model.index(1, StyleItemData::CUR_COLUMN, {})};
    QModelIndex r010Index2{model.index(0, StyleItemData::CUR_COLUMN, r01Index0)};
    QModelIndex r011Index2{model.index(1, StyleItemData::CUR_COLUMN, r01Index0)};

    QModelIndex r00Index3{model.index(0, StyleItemData::EDITABLE_COLUMN, {})};
    QModelIndex r000Index3{model.index(0, StyleItemData::EDITABLE_COLUMN, r00Index0)};
    QModelIndex r001Index3{model.index(1, StyleItemData::EDITABLE_COLUMN, r00Index0)};
    QModelIndex r002Index3{model.index(2, StyleItemData::EDITABLE_COLUMN, r00Index0)};
    QModelIndex r01Index3{model.index(1, StyleItemData::EDITABLE_COLUMN, {})};
    QModelIndex r010Index3{model.index(0, StyleItemData::EDITABLE_COLUMN, r01Index0)};
    QModelIndex r011Index3{model.index(1, StyleItemData::EDITABLE_COLUMN, r01Index0)};

    QCOMPARE(model.flags({}).testFlag(Qt::ItemIsEnabled), true);

    QCOMPARE(model.flags(r00Index0).testFlag(Qt::ItemIsEditable), false); // not editable column
    QCOMPARE(model.flags(r000Index0).testFlag(Qt::ItemIsEditable), false);
    QCOMPARE(model.flags(r001Index0).testFlag(Qt::ItemIsEditable), false);
    QCOMPARE(model.flags(r002Index0).testFlag(Qt::ItemIsEditable), false);
    QCOMPARE(model.flags(r01Index0).testFlag(Qt::ItemIsEditable), false);
    QCOMPARE(model.flags(r010Index0).testFlag(Qt::ItemIsEditable), false);
    QCOMPARE(model.flags(r011Index0).testFlag(Qt::ItemIsEditable), false);

    QCOMPARE(model.flags(r00Index3).testFlag(Qt::ItemIsEditable), false); // groups
    QCOMPARE(model.flags(r000Index3).testFlag(Qt::ItemIsEditable), true);
    QCOMPARE(model.flags(r001Index3).testFlag(Qt::ItemIsEditable), true);
    QCOMPARE(model.flags(r002Index3).testFlag(Qt::ItemIsEditable), true);
    QCOMPARE(model.flags(r01Index3).testFlag(Qt::ItemIsEditable), false); // groups
    QCOMPARE(model.flags(r010Index3).testFlag(Qt::ItemIsEditable), true);
    QCOMPARE(model.flags(r011Index3).testFlag(Qt::ItemIsEditable), true);

    const QVariant defVar;
    QCOMPARE(model.data({}), defVar);

    { // DisplayRole DATA_TYPE_ROLE ok
      QCOMPARE(model.data(r00Index0, Qt::DisplayRole), "View");
      QCOMPARE(model.data(r00Index1, Qt::DisplayRole), defVar);
      QCOMPARE(model.data(r00Index2, Qt::DisplayRole), defVar);
      QCOMPARE(model.data(r00Index3, Qt::DisplayRole), defVar);
      QCOMPARE(model.data(r00Index0, StyleItemData::DATA_TYPE_ROLE), StyleItemData::GROUP);

      QCOMPARE(model.data(r000Index0, Qt::DisplayRole), "RowHeight");
      QCOMPARE(model.data(r000Index1, Qt::DisplayRole), 30);
      QCOMPARE(model.data(r000Index2, Qt::DisplayRole), 60);
      QCOMPARE(model.data(r000Index3, Qt::DisplayRole), defVar);
      QCOMPARE(model.data(r000Index0, StyleItemData::DATA_TYPE_ROLE), StyleItemData::NUMBER);

      QCOMPARE(model.data(r001Index0, Qt::DisplayRole), "FontFamily");
      QCOMPARE(model.data(r001Index1, Qt::DisplayRole), "Microsoft YaHei");
      QCOMPARE(model.data(r001Index2, Qt::DisplayRole), "Microsoft YaHei");
      QCOMPARE(model.data(r001Index3, Qt::DisplayRole), defVar);
      QCOMPARE(model.data(r001Index0, StyleItemData::DATA_TYPE_ROLE), StyleItemData::FONT_FAMILY);

      QCOMPARE(model.data(r002Index0, Qt::DisplayRole), "AlternateRowColor");
      QCOMPARE(model.data(r002Index1, Qt::DisplayRole), "#4F4F4F");
      QCOMPARE(model.data(r002Index2, Qt::DisplayRole), "#5E5E5E");
      QCOMPARE(model.data(r002Index3, Qt::DisplayRole), defVar);
      QCOMPARE(model.data(r002Index0, StyleItemData::DATA_TYPE_ROLE), StyleItemData::COLOR);

      QCOMPARE(model.data(r01Index0, Qt::DisplayRole), "Font");
      QCOMPARE(model.data(r01Index1, Qt::DisplayRole), defVar);
      QCOMPARE(model.data(r01Index2, Qt::DisplayRole), defVar);
      QCOMPARE(model.data(r01Index3, Qt::DisplayRole), defVar);
      QCOMPARE(model.data(r01Index0, StyleItemData::DATA_TYPE_ROLE), StyleItemData::GROUP);

      QCOMPARE(model.data(r010Index0, Qt::DisplayRole), "Foreground");
      QCOMPARE(model.data(r010Index1, Qt::DisplayRole), "#000000");
      QCOMPARE(model.data(r010Index2, Qt::DisplayRole), "#FFFFFF");
      QCOMPARE(model.data(r010Index3, Qt::DisplayRole), defVar);
      QCOMPARE(model.data(r010Index0, StyleItemData::DATA_TYPE_ROLE), StyleItemData::COLOR);

      QCOMPARE(model.data(r011Index0, Qt::DisplayRole), "Weight");
      QCOMPARE(model.data(r011Index1, Qt::DisplayRole), QFont::Weight::Normal);
      QCOMPARE(model.data(r011Index2, Qt::DisplayRole), QFont::Weight::Bold);
      QCOMPARE(model.data(r011Index3, Qt::DisplayRole), defVar);
      QCOMPARE(model.data(r011Index0, StyleItemData::DATA_TYPE_ROLE), StyleItemData::FONT_WEIGHT);
    }

    { // EditRole ok
      QCOMPARE(model.data(r00Index0, Qt::EditRole), defVar);
      QCOMPARE(model.data(r000Index0, Qt::EditRole), defVar);
      QCOMPARE(model.data(r001Index0, Qt::EditRole), defVar);
      QCOMPARE(model.data(r002Index0, Qt::EditRole), defVar);
      QCOMPARE(model.data(r01Index0, Qt::EditRole), defVar);
      QCOMPARE(model.data(r010Index0, Qt::EditRole), defVar);
      QCOMPARE(model.data(r011Index0, Qt::EditRole), defVar);

      QCOMPARE(model.data(r00Index3, Qt::EditRole), defVar);
      QCOMPARE(model.data(r000Index3, Qt::EditRole), defVar);
      QCOMPARE(model.data(r001Index3, Qt::EditRole), defVar);
      QCOMPARE(model.data(r002Index3, Qt::EditRole), defVar);
      QCOMPARE(model.data(r01Index3, Qt::EditRole), defVar);
      QCOMPARE(model.data(r010Index3, Qt::EditRole), defVar);
      QCOMPARE(model.data(r011Index3, Qt::EditRole), defVar);
    }

    { // DecorationRole ok

      // not def/backup/modifiedTo column
      QCOMPARE(model.data(r00Index0, Qt::DecorationRole), defVar);
      QCOMPARE(model.data(r000Index0, Qt::DecorationRole), defVar);
      QCOMPARE(model.data(r001Index0, Qt::DecorationRole), defVar);
      QCOMPARE(model.data(r002Index0, Qt::DecorationRole), defVar);
      QCOMPARE(model.data(r01Index0, Qt::DecorationRole), defVar);
      QCOMPARE(model.data(r010Index0, Qt::DecorationRole), defVar);
      QCOMPARE(model.data(r011Index0, Qt::DecorationRole), defVar);

      QCOMPARE(model.data(r00Index1, Qt::DecorationRole), defVar);
      QCOMPARE(model.data(r000Index1, Qt::DecorationRole), defVar);
      QCOMPARE(model.data(r001Index1, Qt::DecorationRole), defVar);
      QCOMPARE(model.data(r002Index1, Qt::DecorationRole).isValid(), true); // color def specified
      QCOMPARE(model.data(r01Index1, Qt::DecorationRole), defVar);
      QCOMPARE(model.data(r010Index1, Qt::DecorationRole).isValid(), true); // color def specified
      QCOMPARE(model.data(r011Index1, Qt::DecorationRole), defVar);

      QCOMPARE(model.data(r00Index2, Qt::DecorationRole), defVar);
      QCOMPARE(model.data(r000Index2, Qt::DecorationRole), defVar);
      QCOMPARE(model.data(r001Index2, Qt::DecorationRole), defVar);
      QCOMPARE(model.data(r002Index2, Qt::DecorationRole).isValid(), true); // color backup specified
      QCOMPARE(model.data(r01Index2, Qt::DecorationRole), defVar);
      QCOMPARE(model.data(r010Index2, Qt::DecorationRole).isValid(), true); // color backup specified
      QCOMPARE(model.data(r011Index2, Qt::DecorationRole), defVar);

      QCOMPARE(model.data(r00Index3, Qt::DecorationRole), defVar);
      QCOMPARE(model.data(r000Index3, Qt::DecorationRole), defVar);
      QCOMPARE(model.data(r001Index3, Qt::DecorationRole), defVar);
      QCOMPARE(model.data(r002Index3, Qt::DecorationRole), defVar); // color row but modifiedTo not specified
      QCOMPARE(model.data(r01Index3, Qt::DecorationRole), defVar);
      QCOMPARE(model.data(r010Index3, Qt::DecorationRole), defVar); // color row but modifiedTo not specified
      QCOMPARE(model.data(r011Index3, Qt::DecorationRole), defVar);
    }
  }

  void setData_ok() {
    StyleTreeNode                                                  //
        *r0{nullptr},                                              //
        /* -- */ *r00{nullptr},                                    //
        /* ---- */ *r000{nullptr}, *r001{nullptr}, *r002{nullptr}, //
        /* -- */ *r01{nullptr},                                    //
        /* ---- */ *r010{nullptr}, *r011{nullptr};                 //

    std::unique_ptr<StyleTreeNode> rootNode = GetRootNodeForTest(r0, r00, r000, r001, r002, r01, r010, r011);

    StyleSheetTreeModel model;
    model.setDatas(std::move(rootNode));
    QCOMPARE(model.m_bLivePreviewSwitch, false);

    QModelIndex r00Index0{model.index(0, StyleItemData::NAME_COLUMN, {})};
    QModelIndex r000Index0{model.index(0, StyleItemData::NAME_COLUMN, r00Index0)};
    QModelIndex r001Index0{model.index(1, StyleItemData::NAME_COLUMN, r00Index0)};
    QModelIndex r002Index0{model.index(2, StyleItemData::NAME_COLUMN, r00Index0)};
    QModelIndex r01Index0{model.index(1, StyleItemData::NAME_COLUMN, {})};
    QModelIndex r010Index0{model.index(0, StyleItemData::NAME_COLUMN, r01Index0)};
    QModelIndex r011Index0{model.index(1, StyleItemData::NAME_COLUMN, r01Index0)};

    QModelIndex r00Index1{model.index(0, StyleItemData::DEF_COLUMN, {})};
    QModelIndex r000Index1{model.index(0, StyleItemData::DEF_COLUMN, r00Index0)};
    QModelIndex r001Index1{model.index(1, StyleItemData::DEF_COLUMN, r00Index0)};
    QModelIndex r002Index1{model.index(2, StyleItemData::DEF_COLUMN, r00Index0)};
    QModelIndex r01Index1{model.index(1, StyleItemData::DEF_COLUMN, {})};
    QModelIndex r010Index1{model.index(0, StyleItemData::DEF_COLUMN, r01Index0)};
    QModelIndex r011Index1{model.index(1, StyleItemData::DEF_COLUMN, r01Index0)};

    QModelIndex r00Index2{model.index(0, StyleItemData::CUR_COLUMN, {})};
    QModelIndex r000Index2{model.index(0, StyleItemData::CUR_COLUMN, r00Index0)};
    QModelIndex r001Index2{model.index(1, StyleItemData::CUR_COLUMN, r00Index0)};
    QModelIndex r002Index2{model.index(2, StyleItemData::CUR_COLUMN, r00Index0)};
    QModelIndex r01Index2{model.index(1, StyleItemData::CUR_COLUMN, {})};
    QModelIndex r010Index2{model.index(0, StyleItemData::CUR_COLUMN, r01Index0)};
    QModelIndex r011Index2{model.index(1, StyleItemData::CUR_COLUMN, r01Index0)};

    QModelIndex r00Index3{model.index(0, StyleItemData::EDITABLE_COLUMN, {})};
    QModelIndex r000Index3{model.index(0, StyleItemData::EDITABLE_COLUMN, r00Index0)};
    QModelIndex r001Index3{model.index(1, StyleItemData::EDITABLE_COLUMN, r00Index0)};
    QModelIndex r002Index3{model.index(2, StyleItemData::EDITABLE_COLUMN, r00Index0)};
    QModelIndex r01Index3{model.index(1, StyleItemData::EDITABLE_COLUMN, {})};
    QModelIndex r010Index3{model.index(0, StyleItemData::EDITABLE_COLUMN, r01Index0)};
    QModelIndex r011Index3{model.index(1, StyleItemData::EDITABLE_COLUMN, r01Index0)};

    // onInstantApplySwitchChanged
    QCOMPARE(model.m_bLivePreviewSwitch, false);

    // r00 = rootNode->appendRow(StyleTreeNode::create(StyleItemData{"View"}));
    // r000 = r00->appendRow(StyleTreeNode::create(StyleItemData{"RowHeight", 30, 60, StyleItemData::DataTypeE::NUMBER}));
    // r001 = r00->appendRow(StyleTreeNode::create(StyleItemData{"FontFamily", "Microsoft YaHei", "Microsoft YaHei", StyleItemData::DataTypeE::FONT_FAMILY}));
    // r002 = r00->appendRow(StyleTreeNode::create(StyleItemData{"AlternateRowColor", "#4F4F4F", "#5E5E5E", StyleItemData::DataTypeE::COLOR}));

    // r01 = rootNode->appendRow(StyleTreeNode::create(StyleItemData{"Font"}));
    // r010 = r01->appendRow(StyleTreeNode::create(StyleItemData{"Foreground", "#000000", "#FFFFFF", StyleItemData::DataTypeE::COLOR}));

    // column not support, only ModifiedTo support setData
    {
      QCOMPARE(model.setData({}, 124, Qt::EditRole), false);

      QCOMPARE(model.setData(r00Index0, "new View", Qt::EditRole), false); // group
      QCOMPARE(model.setData(r000Index0, "new RowHeight", Qt::EditRole), false);
      QCOMPARE(model.setData(r001Index0, "new FontFamily", Qt::EditRole), false);
      QCOMPARE(model.setData(r002Index0, "new AlternateRowColor", Qt::EditRole), false);
      QCOMPARE(model.setData(r01Index0, "new Font", Qt::EditRole), false); // group
      QCOMPARE(model.setData(r010Index0, "new Foreground", Qt::EditRole), false);
      QCOMPARE(model.setData(r011Index0, "new Weight", Qt::EditRole), false);

      QCOMPARE(model.setData(r00Index1, "", Qt::EditRole), false);    // group
      QCOMPARE(model.setData(r000Index1, "31", Qt::EditRole), false); // 数值类型无候选下拉框, Delegate返回数值字符串传入setData
      QCOMPARE(model.setData(r001Index1, "Arial", Qt::EditRole), false);
      QCOMPARE(model.setData(r002Index1, "#123456", Qt::EditRole), false);
      QCOMPARE(model.setData(r01Index1, "", Qt::EditRole), false); // group
      QCOMPARE(model.setData(r010Index1, "#123456", Qt::EditRole), false);
      QCOMPARE(model.setData(r011Index1, QFont::Weight::Light, Qt::EditRole), false); // 枚举类型由候选下拉框, Delegate返回枚举数值传入setData

      QCOMPARE(model.setData(r00Index2, "", Qt::EditRole), false);    // group
      QCOMPARE(model.setData(r000Index2, "61", Qt::EditRole), false); // 数值类型无候选下拉框, Delegate将直接返回数值字符串
      QCOMPARE(model.setData(r001Index2, "Arial", Qt::EditRole), false);
      QCOMPARE(model.setData(r002Index2, "#123456", Qt::EditRole), false);
      QCOMPARE(model.setData(r01Index2, "", Qt::EditRole), false); // group
      QCOMPARE(model.setData(r010Index2, "#123456", Qt::EditRole), false);
      QCOMPARE(model.setData(r011Index2, QFont::Weight::Light, Qt::EditRole), false); // 枚举类型由候选下拉框, Delegate返回枚举数值传入setData
    }

    // group not support change
    {
      QCOMPARE(model.setData(r00Index3, "Modified To new View ", Qt::EditRole), false);
      QCOMPARE(model.setData(r01Index3, "Modified To new Font", Qt::EditRole), false);
    }

    // role not correct
    {
      QCOMPARE(model.setData(r00Index3, "", Qt::DisplayRole), false);    // group
      QCOMPARE(model.setData(r000Index3, "61", Qt::DisplayRole), false); // 数值类型无候选下拉框, Delegate将直接返回数值字符串
      QCOMPARE(model.setData(r001Index3, "Arial", Qt::DisplayRole), false);
      QCOMPARE(model.setData(r002Index3, "#123456", Qt::DisplayRole), false);
      QCOMPARE(model.setData(r01Index3, "", Qt::DisplayRole), false); // group
      QCOMPARE(model.setData(r010Index3, "#123456", Qt::DisplayRole), false);
      QCOMPARE(model.setData(r011Index3, QFont::Weight::Light, Qt::DisplayRole), false); // 枚举类型由候选下拉框, Delegate返回枚举数值传入setData
    }

    // invalid QVariant
    {
      QVariant invalidVar;
      QCOMPARE(model.setData(r00Index3, invalidVar, Qt::EditRole), false);  // group
      QCOMPARE(model.setData(r000Index3, invalidVar, Qt::EditRole), false); // 数值类型无候选下拉框, Delegate将直接返回数值字符串
      QCOMPARE(model.setData(r001Index3, invalidVar, Qt::EditRole), false);
      QCOMPARE(model.setData(r002Index3, invalidVar, Qt::EditRole), false);
      QCOMPARE(model.setData(r01Index3, invalidVar, Qt::EditRole), false); // group
      QCOMPARE(model.setData(r010Index3, invalidVar, Qt::EditRole), false);
      QCOMPARE(model.setData(r011Index3, invalidVar, Qt::EditRole), false); // 枚举类型由候选下拉框, Delegate返回枚举数值传入setData
    }

    QSignalSpy dataChangedSpy{&model, &StyleSheetTreeModel::dataChanged};
    QSignalSpy reqApplyChangesSpy{&model, &StyleSheetTreeModel::requestSeeChanges};
    // 不触发通知应用修改信号
    QCOMPARE(model.m_bLivePreviewSwitch, false);
    {
      QVariant defUnspecifiedVar;

      QCOMPARE(r000Index3.data(), defUnspecifiedVar);
      QCOMPARE(model.setData(r000Index3, "61", Qt::EditRole), true); // 数值类型无候选下拉框, Delegate将直接返回数值字符串
      QCOMPARE(dataChangedSpy.count(), 1);
      QCOMPARE(reqApplyChangesSpy.count(), 0);
      dataChangedSpy.clear();
      reqApplyChangesSpy.clear();

      QCOMPARE(r001Index3.data(), defUnspecifiedVar);
      QCOMPARE(model.setData(r001Index3, "Arial", Qt::EditRole), true);
      QCOMPARE(r001Index3.data(), "Arial");
      QCOMPARE(dataChangedSpy.count(), 1);
      QCOMPARE(reqApplyChangesSpy.count(), 0);
      dataChangedSpy.clear();
      reqApplyChangesSpy.clear();

      QCOMPARE(r002Index3.data(), defUnspecifiedVar);
      QCOMPARE(model.setData(r002Index3, "#123456", Qt::EditRole), true);
      QCOMPARE(r002Index3.data(), "#123456");
      QCOMPARE(dataChangedSpy.count(), 1);
      QCOMPARE(reqApplyChangesSpy.count(), 0);
      dataChangedSpy.clear();
      reqApplyChangesSpy.clear();

      QCOMPARE(r010Index3.data(), defUnspecifiedVar);
      QCOMPARE(model.setData(r010Index3, "#123456", Qt::EditRole), true);
      QCOMPARE(r010Index3.data(), "#123456");
      QCOMPARE(dataChangedSpy.count(), 1);
      QCOMPARE(reqApplyChangesSpy.count(), 0);
      dataChangedSpy.clear();
      reqApplyChangesSpy.clear();

      QCOMPARE(r011Index3.data(), defUnspecifiedVar);
      QCOMPARE(model.setData(r011Index3, QFont::Weight::Light, Qt::EditRole), true); // 枚举类型由候选下拉框, Delegate返回枚举数值传入setData
      QCOMPARE(r011Index3.data(), QFont::Weight::Light);
      QCOMPARE(dataChangedSpy.count(), 1);
      QCOMPARE(reqApplyChangesSpy.count(), 0);
      dataChangedSpy.clear();
      reqApplyChangesSpy.clear();

      // siblingAtColumn Ok
      QCOMPARE(model.ClearNewValues(QModelIndexList{}), 0);
      QCOMPARE(model.siblingAtColumn(r000Index0, StyleItemData::EDITABLE_COLUMN), r000Index3);
      QCOMPARE(model.siblingAtColumn(r001Index0, StyleItemData::EDITABLE_COLUMN), r001Index3);
      QCOMPARE(model.siblingAtColumn(r002Index0, StyleItemData::EDITABLE_COLUMN), r002Index3);
      QCOMPARE(model.siblingAtColumn(r010Index0, StyleItemData::EDITABLE_COLUMN), r010Index3);
      QCOMPARE(model.siblingAtColumn(r011Index0, StyleItemData::EDITABLE_COLUMN), r011Index3);
      QCOMPARE(model.ClearNewValues(QModelIndexList{r000Index0, r001Index0, r002Index0, r010Index0, r011Index0}), 5);
      QCOMPARE(model.mEditFailedCells.size(), 0);
      QCOMPARE(dataChangedSpy.count(), 5); // DisplayRole Changed 5 times
      QCOMPARE(reqApplyChangesSpy.count(), 0);

      QCOMPARE(r000Index3.data(), defUnspecifiedVar);
      QCOMPARE(r001Index3.data(), defUnspecifiedVar);
      QCOMPARE(r002Index3.data(), defUnspecifiedVar);
      QCOMPARE(r010Index3.data(), defUnspecifiedVar);
      QCOMPARE(r011Index3.data(), defUnspecifiedVar);
      dataChangedSpy.clear();
      reqApplyChangesSpy.clear();
    }

    // 立即触发应用修改信号
    model.onLivePreviewSwitchChanged(true);
    QCOMPARE(model.m_bLivePreviewSwitch, true);

    QCOMPARE(model.mEditFailedCells.isEmpty(), true); // true
    // 成功修改, 且editCell失败字典为空, 只会有1次dataChanged
    {
      QVariant defUnspecifiedVar;
      QCOMPARE(r000Index3.data(), defUnspecifiedVar);
      QCOMPARE(model.setData(r000Index3, "62", Qt::EditRole), true); // 数值类型无候选下拉框, Delegate将直接返回数值字符串
      QCOMPARE(dataChangedSpy.count(), 1);
      QCOMPARE(reqApplyChangesSpy.count(), 1);
      QCOMPARE(reqApplyChangesSpy.takeLast(), (QVariantList{QVariant{"StyleSheetInTest/View/RowHeight"}, QVariant{"62"}}));
      dataChangedSpy.clear();
      reqApplyChangesSpy.clear();

      QCOMPARE(r001Index3.data(), defUnspecifiedVar);
      QCOMPARE(model.setData(r001Index3, "Times New Roman", Qt::EditRole), true);
      QCOMPARE(r001Index3.data(), "Times New Roman");
      QCOMPARE(dataChangedSpy.count(), 1);
      QCOMPARE(reqApplyChangesSpy.count(), 1);
      QCOMPARE(reqApplyChangesSpy.takeLast(), (QVariantList{QVariant{"StyleSheetInTest/View/FontFamily"}, QVariant{"Times New Roman"}}));
      dataChangedSpy.clear();
      reqApplyChangesSpy.clear();

      QCOMPARE(r002Index3.data(), defUnspecifiedVar);
      QCOMPARE(model.setData(r002Index3, "#123457", Qt::EditRole), true);
      QCOMPARE(r002Index3.data(), "#123457");
      QCOMPARE(dataChangedSpy.count(), 1);
      QCOMPARE(reqApplyChangesSpy.count(), 1);
      QCOMPARE(reqApplyChangesSpy.takeLast(), (QVariantList{QVariant{"StyleSheetInTest/View/AlternateRowColor"}, QVariant{"#123457"}}));
      dataChangedSpy.clear();
      reqApplyChangesSpy.clear();

      QCOMPARE(r010Index3.data(), defUnspecifiedVar);
      QCOMPARE(model.setData(r010Index3, "#123457", Qt::EditRole), true);
      QCOMPARE(r010Index3.data(), "#123457");
      QCOMPARE(dataChangedSpy.count(), 1);
      QCOMPARE(reqApplyChangesSpy.count(), 1);
      QCOMPARE(reqApplyChangesSpy.takeLast(), (QVariantList{QVariant{"StyleSheetInTest/Font/Foreground"}, QVariant{"#123457"}}));
      dataChangedSpy.clear();
      reqApplyChangesSpy.clear();

      QCOMPARE(r011Index3.data(), defUnspecifiedVar);
      QCOMPARE(model.setData(r011Index3, QFont::Weight::ExtraBold, Qt::EditRole), true); // 枚举类型由候选下拉框, Delegate返回枚举数值传入setData
      QCOMPARE(r011Index3.data(), QFont::Weight::ExtraBold);
      QCOMPARE(dataChangedSpy.count(), 1);
      QCOMPARE(reqApplyChangesSpy.count(), 1);
      QCOMPARE(reqApplyChangesSpy.takeLast(), (QVariantList{QVariant{"StyleSheetInTest/Font/Weight"}, QVariant{(int) QFont::Weight::ExtraBold}}));
      dataChangedSpy.clear();
      reqApplyChangesSpy.clear();

      QCOMPARE(model.ClearNewValues(QModelIndexList{r000Index3, r001Index3, r002Index3, r010Index3, r011Index3}), 5);
      QCOMPARE(dataChangedSpy.count(), 5); // DisplayRole Changed 5 times
      QCOMPARE(reqApplyChangesSpy.count(), 0);
      dataChangedSpy.clear();
      reqApplyChangesSpy.clear();
      QCOMPARE(r000Index3.data(), defUnspecifiedVar);
      QCOMPARE(r001Index3.data(), defUnspecifiedVar);
      QCOMPARE(r002Index3.data(), defUnspecifiedVar);
      QCOMPARE(r010Index3.data(), defUnspecifiedVar);
      QCOMPARE(r011Index3.data(), defUnspecifiedVar);
      QCOMPARE(model.mEditFailedCells.size(), 0);
    }

    model.onLivePreviewSwitchChanged(false);
    QCOMPARE(model.m_bLivePreviewSwitch, false);
    QCOMPARE(model.mEditFailedCells.isEmpty(), true);

    {
      QVariant defUnspecifiedVar;

      // 数值设置非法值, DisplayRole not changed, 但decorationRole Changed, 仅1次
      bool doubleStrVarConvert{true};
      QCOMPARE(QVariant{"62.1"}.toInt(&doubleStrVarConvert), 0);
      QCOMPARE(doubleStrVarConvert, false);

      QCOMPARE(r000Index3.data(), defUnspecifiedVar);
      QCOMPARE(model.data(r000Index3, Qt::DecorationRole).isValid(), false); //
      QCOMPARE(model.setData(r000Index3, "62.1", Qt::EditRole), false);      // 数值类型无候选下拉框, Delegate将直接返回数值字符串
      QCOMPARE(model.data(r000Index3, Qt::DecorationRole).isValid(), true);  // failed Icon
      QCOMPARE(model.mEditFailedCells.size(), 1);
      QCOMPARE(dataChangedSpy.count(), 1);
      QCOMPARE(reqApplyChangesSpy.count(), 0);
      dataChangedSpy.clear();
      reqApplyChangesSpy.clear();
      QCOMPARE(r000Index3.data(), defUnspecifiedVar);

      // 修改成功 DisplayRole Changed, 清除失败decoration role 二者各自1次
      QCOMPARE(model.setData(r000Index3, "62", Qt::EditRole), true); // 数值类型无候选下拉框, Delegate将直接返回数值字符串
      QCOMPARE(model.mEditFailedCells.size(), 0);
      QCOMPARE(dataChangedSpy.count(), 2);
      QCOMPARE(reqApplyChangesSpy.count(), 0);
      dataChangedSpy.clear();
      reqApplyChangesSpy.clear();
      QCOMPARE(r000Index3.data(), "62");
      model.mEditFailedCells.clear();
      QCOMPARE(model.setData(r000Index3, "62", Qt::EditRole), false); // 无变更
      QCOMPARE(model.mEditFailedCells.size(), 0);
      QCOMPARE(dataChangedSpy.count(), 0);
      QCOMPARE(reqApplyChangesSpy.count(), 0);
      dataChangedSpy.clear();
      reqApplyChangesSpy.clear();
      QCOMPARE(r000Index3.data(), "62");

      // 颜色字符串设置为空, 置颜色为初始无效值, 接受:true, 有变更:true; 失败词典为空
      QCOMPARE(r010Index3.data(), defUnspecifiedVar);
      QCOMPARE(model.setData(r010Index3, "", Qt::EditRole), true);
      QCOMPARE(model.mEditFailedCells.size(), 0);
      QCOMPARE(dataChangedSpy.count(), 1);
      QCOMPARE(reqApplyChangesSpy.count(), 0);
      dataChangedSpy.clear();
      reqApplyChangesSpy.clear();
      QCOMPARE(r010Index3.data(), defUnspecifiedVar);
      model.mEditFailedCells.clear();

      // 先设置为颜色字符串非颜色, 接受: false, 有变更:false; 失败词典1个元素; decorationChanged: 1次
      // 再修改成功合法颜色, 接受:true, 有变更:true; DisplayRole Changed, 清除失败decoration role 二者各自1次
      // 最后保持颜色, 接受:true, 有变更:false; 无信号
      QCOMPARE(model.setData(r010Index3, "#NotColorString", Qt::EditRole), false);
      QCOMPARE(model.mEditFailedCells.size(), 1);
      QCOMPARE(dataChangedSpy.count(), 1);
      QCOMPARE(reqApplyChangesSpy.count(), 0);
      dataChangedSpy.clear();
      reqApplyChangesSpy.clear();
      QCOMPARE(r010Index3.data(), defUnspecifiedVar);
      QCOMPARE(model.setData(r010Index3, "#FF00FF", Qt::EditRole), true);
      QCOMPARE(model.mEditFailedCells.size(), 0);
      QCOMPARE(dataChangedSpy.count(), 2);
      QCOMPARE(reqApplyChangesSpy.count(), 0);
      dataChangedSpy.clear();
      reqApplyChangesSpy.clear();
      QCOMPARE(r010Index3.data(), "#FF00FF");
      model.mEditFailedCells.clear();
      QCOMPARE(model.setData(r010Index3, "#FF00FF", Qt::EditRole), false);
      QCOMPARE(model.mEditFailedCells.size(), 0);
      QCOMPARE(dataChangedSpy.count(), 0);
      QCOMPARE(reqApplyChangesSpy.count(), 0);
      dataChangedSpy.clear();
      reqApplyChangesSpy.clear();

      const QModelIndexList selected5Node2Group{r000Index3, r001Index3, r002Index3, r010Index3, r011Index3, r00Index0, r01Index0, QModelIndex{}};
      // 上面只修改了r000Index3和r010Index3
      QVariantHash expectCfs;
      QCOMPARE(model.ClearNewValues(selected5Node2Group), 2); // 最后 两个group, 1个非法
      QCOMPARE(dataChangedSpy.count(), 2);                    // DisplayRole Changed 5 times
      QCOMPARE(reqApplyChangesSpy.count(), 0);
      dataChangedSpy.clear();
      reqApplyChangesSpy.clear();
      QCOMPARE(r000Index3.data(), defUnspecifiedVar);
      QCOMPARE(r001Index3.data(), defUnspecifiedVar);
      QCOMPARE(r002Index3.data(), defUnspecifiedVar);
      QCOMPARE(r010Index3.data(), defUnspecifiedVar);
      QCOMPARE(r011Index3.data(), defUnspecifiedVar);
      QCOMPARE(model.mEditFailedCells.size(), 0);

      // 已经没有任何需要应用的变更
      const QVariantHash needApplyChangesCfgsDict = model.CollectItemsNeedSeeChange(selected5Node2Group);
      expectCfs = QVariantHash{};
      QCOMPARE(needApplyChangesCfgsDict, expectCfs);

      // 全都恢复为默认值
      QCOMPARE(model.RecoverNewValuesToDefault(selected5Node2Group), 5); // 7 - (两个group, 1个非法)
      QCOMPARE(dataChangedSpy.count(), 5);                               // DisplayRole Changed 5 times
      QCOMPARE(reqApplyChangesSpy.count(), 0);
      dataChangedSpy.clear();
      reqApplyChangesSpy.clear();
      QCOMPARE(r000Index3.data(), 30);
      QCOMPARE(r001Index3.data(), "Microsoft YaHei");
      QCOMPARE(r002Index3.data(), "#4F4F4F");
      QCOMPARE(r010Index3.data(), "#000000");
      QCOMPARE(r011Index3.data(), QFont::Weight::Normal);
      const QVariantHash needApplyChangesCfgsNoSelectedDict = model.CollectItemsNeedSeeChange({});
      QCOMPARE(needApplyChangesCfgsNoSelectedDict, expectCfs); // 没有选中时, 没有任何需要应用的变更

      const QVariantHash cfgsDefaultDict = model.CollectItemsNeedSeeChange(selected5Node2Group);
      expectCfs = QVariantHash{
          {"StyleSheetInTest/View/RowHeight", 30},
          {"StyleSheetInTest/View/FontFamily", "Microsoft YaHei"},
          {"StyleSheetInTest/View/AlternateRowColor", "#4F4F4F"},
          {"StyleSheetInTest/Font/Foreground", "#000000"},
          {"StyleSheetInTest/Font/Weight", QFont::Weight::Normal},
      };
      QCOMPARE(cfgsDefaultDict, expectCfs);

      // 全都恢复为备份值 View/FontFamily无变化
      QCOMPARE(model.RecoverNewValuesToBackup(selected5Node2Group), 4); // 7 - (两个group, 1个非法) - View/Family无变化
      QCOMPARE(r000Index3.data(), 60);
      QCOMPARE(r001Index3.data(), "Microsoft YaHei");
      QCOMPARE(r002Index3.data(), "#5E5E5E");
      QCOMPARE(r010Index3.data(), "#FFFFFF");
      QCOMPARE(r011Index3.data(), QFont::Weight::Bold);
      QCOMPARE(dataChangedSpy.count(), 4); // DisplayRole Changed 4 times
      QCOMPARE(reqApplyChangesSpy.count(), 0);
      dataChangedSpy.clear();
      reqApplyChangesSpy.clear();

      const QVariantHash cfgsBackupDict = model.CollectItemsNeedSeeChange(selected5Node2Group);
      expectCfs = QVariantHash{
          {"StyleSheetInTest/View/RowHeight", 60},
          {"StyleSheetInTest/View/FontFamily", "Microsoft YaHei"},
          {"StyleSheetInTest/View/AlternateRowColor", "#5E5E5E"},
          {"StyleSheetInTest/Font/Foreground", "#FFFFFF"},
          {"StyleSheetInTest/Font/Weight", QFont::Weight::Bold},
      };
      QCOMPARE(cfgsBackupDict, expectCfs);

      model.mEditFailedCells.clear();
      dataChangedSpy.clear();
      reqApplyChangesSpy.clear();
    }

    {
      // SetNewColors 入参由对话框 QColorDialog::getColor提供, 必定合法
      QCOMPARE(model.SetNewColors({}, "#765432"), 0);

      // 只有前两个是颜色
      QModelIndexList colorsIndexes{r002Index3, r010Index3, r01Index3, r011Index3}; // View/AlternateRowColor,  Font/Foreground, group, Font/Weight
      QCOMPARE(model.SetNewColors({colorsIndexes}, "#765432"), 2);
      QCOMPARE(model.mEditFailedCells.size(), 0);
      QCOMPARE(dataChangedSpy.size(), 2);
      QCOMPARE(reqApplyChangesSpy.size(), 0);
      dataChangedSpy.clear();
      reqApplyChangesSpy.clear();

      // 无需任何变更了已经
      QCOMPARE(model.SetNewColors({colorsIndexes}, "#765432"), 0);
      QCOMPARE(model.mEditFailedCells.size(), 0);
      QCOMPARE(dataChangedSpy.size(), 0);
      QCOMPARE(reqApplyChangesSpy.size(), 0);
      dataChangedSpy.clear();
      reqApplyChangesSpy.clear();
    }
  }

  void SetFontGeneral_ok() {
    StyleSheetTreeModel model;
    QVERIFY(model.m_pRoot);
    QVERIFY(model.mFontGeneralFamilyNode != nullptr);
    QVERIFY(model.mFontGeneralSizeNode != nullptr);
    QVERIFY(model.mFontGeneralWeightNode != nullptr);
    QVERIFY(model.mFontGeneralStyleNode != nullptr);

    // 默认已关闭实时预览变更
    QVERIFY(!model.m_bLivePreviewSwitch);

    QSignalSpy dataChangedSpy{&model, &StyleSheetTreeModel::dataChanged};
    QSignalSpy reqApplyChangesSpy{&model, &StyleSheetTreeModel::requestSeeChanges};

    QString newFamily{"NewFontFamily"};
    int newPointSize = 99;
    QFont::Weight newWeight = QFont::Weight::Bold;
    QFont::Style newStyle = QFont::Style::StyleItalic;
    QFont newFont{newFamily, newPointSize, newWeight, newStyle};

    QVERIFY(model.mFontGeneralFamilyNode->value().modifiedToValue != newFamily);
    QVERIFY(model.mFontGeneralSizeNode->value().modifiedToValue != newPointSize);
    QVERIFY(model.mFontGeneralWeightNode->value().modifiedToValue != newWeight);
    QVERIFY(model.mFontGeneralStyleNode->value().modifiedToValue != newStyle);

    // 4次调用setData, 全accept, 全changed
    QCOMPARE(model.SetFontGeneral(newFont), 4);
    QVERIFY(model.mEditFailedCells.isEmpty());
    QCOMPARE(dataChangedSpy.count(), 4);
    QCOMPARE(reqApplyChangesSpy.count(), 0);
    dataChangedSpy.clear();
    reqApplyChangesSpy.clear();

    QCOMPARE(model.mFontGeneralFamilyNode->value().modifiedToValue, newFamily);
    QCOMPARE(model.mFontGeneralSizeNode->value().modifiedToValue, newPointSize);
    QCOMPARE(model.mFontGeneralWeightNode->value().modifiedToValue, newWeight);
    QCOMPARE(model.mFontGeneralStyleNode->value().modifiedToValue, newStyle);

    // 没有任何变更, 4次调用setData, 全accept, 全not changed
    QCOMPARE(model.SetFontGeneral(newFont), 0);
    QVERIFY(model.mEditFailedCells.isEmpty());
    QCOMPARE(dataChangedSpy.count(), 0);
    QCOMPARE(reqApplyChangesSpy.count(), 0);
    dataChangedSpy.clear();
    reqApplyChangesSpy.clear();
  }
};

#include "StyleSheetTreeModelTest.moc"
REGISTER_TEST(StyleSheetTreeModelTest, false)
