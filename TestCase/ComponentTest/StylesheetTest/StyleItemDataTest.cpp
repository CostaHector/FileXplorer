#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "StyleItemData.h"
#include "EndToExposePrivateMember.h"

#include "PublicVariable.h"
#include "Configuration.h"
#include <QHash>

class StyleItemDataTest : public PlainTestSuite {
  Q_OBJECT
public:
  const QFileInfo curFile{__FILE__};
  const QString curFilePath{curFile.absoluteFilePath()};
  const QString curFolderPath{curFile.absolutePath()};
  const QString otherFilePath{Configuration().fileName()};
  const QString otherFolderPath{SystemPath::HOME_PATH()};

private slots:
  void initTestCase() { //
    QVERIFY(QFile::exists(curFilePath));
    QVERIFY(QFile::exists(curFolderPath));
    QVERIFY(QFile::exists(otherFilePath)); // precondition must exists
    QVERIFY(QFile::exists(otherFolderPath));

    QVERIFY(StyleItemData::COLUMN_COUNT > 0);
    constexpr int elementCntInHorArray{sizeof(StyleItemData::HOR_HEADER_TITLES) / sizeof(StyleItemData::HOR_HEADER_TITLES[0])};
    QCOMPARE(elementCntInHorArray, StyleItemData::COLUMN_COUNT);
    QCOMPARE(StyleTreeNode::horizontalHeaderTitles(), StyleItemData::HOR_HEADER_TITLES);
  }

  void match_ok() {
    StyleItemData def;
    QCOMPARE(def.isGroup, true);
    QCOMPARE(def.name, "");
    QCOMPARE(def.defValue.isValid(), false);
    QCOMPARE(def.curValue.isValid(), false);
    QCOMPARE(def.modifiedToValue.isValid(), false);
    QCOMPARE(def.isValid(), false);
    QCOMPARE(def.match("", Qt::CaseSensitive), true);
    QCOMPARE(def.match("", Qt::CaseInsensitive), true);
    QCOMPARE(def.match("randomText", Qt::CaseInsensitive), false);
    QCOMPARE(def.match("randomText", Qt::CaseSensitive), false);

    StyleItemData group{"groupName"};
    QCOMPARE(group.isGroup, true);
    QCOMPARE(group.name, "groupName");
    QCOMPARE(group.defValue.isValid(), false);
    QCOMPARE(group.curValue.isValid(), false);
    QCOMPARE(group.modifiedToValue.isValid(), false);
    QCOMPARE(group.isValid(), true);
    QCOMPARE(group.match("groupName", Qt::CaseSensitive), true);
    QCOMPARE(group.match("GROUPNAME", Qt::CaseSensitive), false);
    QCOMPARE(group.match("GROUPNAME", Qt::CaseInsensitive), true);
    QCOMPARE(group.match("randomText", Qt::CaseInsensitive), false);
    QCOMPARE(group.match("randomText", Qt::CaseSensitive), false);

    StyleItemData numberNode{"RowHeight", 30, 60, GeneralDataType::Type::PLAIN_INT};
    StyleItemData stringNode{"FontFamily", "Microsoft YaHei UI", "Noto Sans", GeneralDataType::Type::FONT_FAMILY};
    StyleItemData styleIntNode{"FontStyle", QFont::Style::StyleNormal, QFont::Style::StyleItalic, GeneralDataType::Type::FONT_STYLE};
    StyleItemData weightIntNode{"FontWeight", QFont::Weight::Normal, QFont::Weight::Bold, GeneralDataType::Type::FONT_WEIGHT};
    StyleItemData colorNode{"ForegroundColor", "#FFFFFF", "#000000", GeneralDataType::Type::COLOR};

    QCOMPARE(numberNode.match(30), true);
    QCOMPARE(numberNode.match(60), true);
    QCOMPARE(numberNode.match(0), false);

    QCOMPARE(stringNode.isGroup, false);
    QCOMPARE(stringNode.name, "FontFamily");
    QCOMPARE(stringNode.defValue, "Microsoft YaHei UI");
    QCOMPARE(stringNode.curValue, "Noto Sans");
    QCOMPARE(stringNode.modifiedToValue, (QVariant{}));
    QCOMPARE(stringNode.isValid(), true);
    QCOMPARE(stringNode.match("FontFamily", Qt::CaseSensitive), true);
    QCOMPARE(stringNode.match("FONTFAMILY", Qt::CaseSensitive), false);
    QCOMPARE(stringNode.match("FONTFAMILY", Qt::CaseInsensitive), true);
    QCOMPARE(stringNode.match("Microsoft YaHei UI", Qt::CaseSensitive), true);
    QCOMPARE(stringNode.match("Noto Sans", Qt::CaseSensitive), true);
    QCOMPARE(stringNode.match("randomText", Qt::CaseInsensitive), false);
    QCOMPARE(stringNode.match("randomText", Qt::CaseSensitive), false);

    QCOMPARE(colorNode.match("#", Qt::CaseSensitive), true);
    QCOMPARE(colorNode.match("#000000", Qt::CaseSensitive), true);
    QCOMPARE(colorNode.match("#FFFFFF", Qt::CaseSensitive), true);
    QCOMPARE(colorNode.match("#ffffff", Qt::CaseSensitive), false);
    QCOMPARE(colorNode.match("#ffffff", Qt::CaseInsensitive), true);

    QCOMPARE((colorNode == colorNode), true);
    QCOMPARE((colorNode != stringNode), true);

    QCOMPARE((styleIntNode == group), false);
    QCOMPARE((styleIntNode != group), true);

    QCOMPARE((stringNode == colorNode), false);
    QCOMPARE((stringNode != colorNode), true);
  }

  void modifyValueTo_isNeedApplyChange_ok() {
    // group 类型拒绝modifiedToValue修改
    StyleItemData def;
    StyleItemData group{"groupName"};
    QVERIFY(!def.isNeedApplyChange());
    QVERIFY(!group.isNeedApplyChange());
    QVERIFY(!def.recoverToBackup());
    QVERIFY(!group.recoverToBackup());
    QVERIFY(!def.invalidateNewValue());
    QVERIFY(!group.invalidateNewValue());
    bool bNewValueAccept{true};
    QVERIFY(!def.modifyValueTo("ABC", bNewValueAccept));
    QVERIFY(!bNewValueAccept);
    bNewValueAccept = true;
    QVERIFY(!group.modifyValueTo("ABC", bNewValueAccept));
    QVERIFY(!bNewValueAccept);
    bNewValueAccept = true;
    QVERIFY(!def.modifiedColorTo("ABC"));
    QVERIFY(!group.modifiedColorTo("ABC"));

    StyleItemData numberNode{"RowHeight", 30, 60, GeneralDataType::Type::PLAIN_INT};
    StyleItemData stringNode{"FontFamily", "Microsoft YaHei UI", "Noto Sans", GeneralDataType::Type::FONT_FAMILY};
    StyleItemData styleIntNode{"FontStyle", QFont::Style::StyleNormal, QFont::Style::StyleItalic, GeneralDataType::Type::FONT_STYLE};
    StyleItemData weightIntNode{"FontWeight", QFont::Weight::Normal, QFont::Weight::Bold, GeneralDataType::Type::FONT_WEIGHT};
    StyleItemData colorNode{"ForegroundColor", "#FFFFFF", "#000000", GeneralDataType::Type::COLOR};
    StyleItemData fileNode{"MustExistFile", curFilePath, curFilePath, GeneralDataType::Type::FILE_PATH};
    StyleItemData imgOptionalNode{"ImageOptional", ":/DefImg", ":/CurImg", GeneralDataType::Type::IMAGE_PATH_OPTIONAL};
    StyleItemData folderNode{"MustExistFolder", curFolderPath, curFolderPath, GeneralDataType::Type::FOLDER_PATH};
    StyleItemData boolNode{"boolNode", false, true, GeneralDataType::Type::PLAIN_BOOL};

    QVERIFY(!numberNode.isNeedApplyChange());
    QVERIFY(!stringNode.isNeedApplyChange());
    QVERIFY(!styleIntNode.isNeedApplyChange());
    QVERIFY(!weightIntNode.isNeedApplyChange());
    QVERIFY(!colorNode.isNeedApplyChange());
    QVERIFY(!fileNode.isNeedApplyChange());
    QVERIFY(!imgOptionalNode.isNeedApplyChange());
    QVERIFY(!folderNode.isNeedApplyChange());
    QVERIFY(!boolNode.isNeedApplyChange());

    {
      // 数值类型且无下拉框候选, 返回QVariant{"123"}, 预期正确处理 ok
      { // precondition
        bool bVarString2Int{true};
        QCOMPARE((QVariant{"123"}.toInt(&bVarString2Int)), 123);
        QVERIFY(bVarString2Int);

        bool bVarDouble2Int{true};
        QCOMPARE((QVariant{3.14}.toInt(&bVarDouble2Int)), 3);
        QVERIFY(bVarDouble2Int); // 这里居然可以

        bool bVarStringDouble2Int{true};
        QCOMPARE(QVariant{"3.14"}.toInt(&bVarStringDouble2Int), 0);
        QVERIFY(!bVarStringDouble2Int);
      }
      QCOMPARE(numberNode.modifiedToValue, (QVariant()));
      QVERIFY(!numberNode.isNeedApplyChange());

      bool newNumberAccepted{false};
      QVERIFY(numberNode.modifyValueTo("48", newNumberAccepted));
      QVERIFY(newNumberAccepted);
      QCOMPARE(numberNode.modifiedToValue, 48);
      QVERIFY(numberNode.isNeedApplyChange());

      newNumberAccepted = false;
      QVERIFY(!numberNode.modifyValueTo("48", newNumberAccepted)); // unchange
      QVERIFY(newNumberAccepted);
      QCOMPARE(numberNode.modifiedToValue, 48);
      QVERIFY(numberNode.isNeedApplyChange());

      newNumberAccepted = true;
      QVERIFY(!numberNode.modifyValueTo("58.9", newNumberAccepted)); // double cannot convert
      QVERIFY(!newNumberAccepted);
      QCOMPARE(numberNode.modifiedToValue, 48);
      QVERIFY(numberNode.isNeedApplyChange());

      newNumberAccepted = true;
      QVERIFY(!numberNode.modifyValueTo("CannotConvertToNumber", newNumberAccepted)); // cannot convert QVariant{"string"}.toInt()
      QVERIFY(!newNumberAccepted);
      QCOMPARE(numberNode.modifiedToValue, 48);
      QVERIFY(numberNode.isNeedApplyChange());

      // 数值类型且有下拉框候选, 返回QVariant{123}, 预期正确处理 ok
      newNumberAccepted = false;
      QVERIFY(numberNode.modifyValueTo(58, newNumberAccepted));
      QVERIFY(newNumberAccepted);
      QCOMPARE(numberNode.modifiedToValue, 58);
      QVERIFY(numberNode.isNeedApplyChange());
    }

    { // 枚举数值类型且有下拉框候选, 返回QVariant{123}, 预期正确处理 ok
      QCOMPARE(weightIntNode.modifiedToValue, (QVariant()));
      QVERIFY(!weightIntNode.isNeedApplyChange());

      bool newNumberAccepted = false;
      QVERIFY(weightIntNode.modifyValueTo(123, newNumberAccepted));
      QVERIFY(newNumberAccepted);
      QCOMPARE(weightIntNode.modifiedToValue, 123);
      QVERIFY(weightIntNode.isNeedApplyChange());

      newNumberAccepted = false;
      QVERIFY(!weightIntNode.modifyValueTo(123, newNumberAccepted)); // unchange
      QVERIFY(newNumberAccepted);
      QCOMPARE(weightIntNode.modifiedToValue, 123);
      QVERIFY(weightIntNode.isNeedApplyChange());
    }

    { // string ok, 且由下拉框返回标准项
      QCOMPARE(stringNode.modifiedToValue, (QVariant()));
      QVERIFY(!stringNode.isNeedApplyChange());

      bool newFontFamilyStrAccepted = false;
      QVERIFY(stringNode.modifyValueTo("Arial", newFontFamilyStrAccepted));
      QVERIFY(newFontFamilyStrAccepted);
      QCOMPARE(stringNode.modifiedToValue, "Arial");
      QVERIFY(stringNode.isNeedApplyChange());

      QVERIFY(!stringNode.modifyValueTo("Arial", newFontFamilyStrAccepted)); // unchange
      QVERIFY(newFontFamilyStrAccepted);
      QCOMPARE(stringNode.modifiedToValue, "Arial");
      QVERIFY(stringNode.isNeedApplyChange());

      // 空自符串, 将重置为非法值
      newFontFamilyStrAccepted = false;
      QVERIFY(stringNode.modifyValueTo("", newFontFamilyStrAccepted));
      QVERIFY(newFontFamilyStrAccepted);
      QCOMPARE(stringNode.modifiedToValue, (QVariant()));
      QVERIFY(!stringNode.isNeedApplyChange());
    }

    { // color ok
      QCOMPARE(colorNode.modifiedToValue, (QVariant()));
      QVERIFY(!colorNode.isNeedApplyChange());

      bool newColorStrAccepted = false;
      QVERIFY(colorNode.modifyValueTo("#FF0000", newColorStrAccepted));
      QVERIFY(newColorStrAccepted);
      QCOMPARE(colorNode.modifiedToValue, "#FF0000");
      QVERIFY(colorNode.isNeedApplyChange());

      newColorStrAccepted = false;
      QVERIFY(!colorNode.modifyValueTo("#FF0000", newColorStrAccepted)); // unchange
      QVERIFY(newColorStrAccepted);
      QCOMPARE(colorNode.modifiedToValue, "#FF0000");
      QVERIFY(colorNode.isNeedApplyChange());

      // 空自符串, 将重置为非法值
      newColorStrAccepted = false;
      QVERIFY(colorNode.modifyValueTo("", newColorStrAccepted));
      QVERIFY(newColorStrAccepted);
      QCOMPARE(colorNode.modifiedToValue, (QVariant()));
      QVERIFY(!colorNode.isNeedApplyChange());
    }

    { // file ok
      QCOMPARE(fileNode.modifiedToValue, (QVariant()));
      QVERIFY(!fileNode.isNeedApplyChange());
      // 存在且之前是初始值 -> (接受, 有修改)
      bool newFileStrAccepted = false;
      QVERIFY(fileNode.modifyValueTo(otherFilePath, newFileStrAccepted));
      QVERIFY(newFileStrAccepted);
      QCOMPARE(fileNode.modifiedToValue, otherFilePath);
      QVERIFY(fileNode.isNeedApplyChange());

      // 存在但和之前已知, -> (接受, 不修改)
      newFileStrAccepted = false;
      QVERIFY(!fileNode.modifyValueTo(otherFilePath, newFileStrAccepted)); // unchange
      QVERIFY(newFileStrAccepted);
      QCOMPARE(fileNode.modifiedToValue, otherFilePath);
      QVERIFY(fileNode.isNeedApplyChange());

      // 不存在的文件 -> (不接受, 不修改)
      newFileStrAccepted = true;
      QVERIFY(!fileNode.modifyValueTo("inexist/file/path", newFileStrAccepted));
      QVERIFY(!newFileStrAccepted);
      QCOMPARE(fileNode.modifiedToValue, otherFilePath);
      QVERIFY(fileNode.isNeedApplyChange());

      // 存在, 但是是文件夹 类型不一致 -> (不接受, 不修改)
      newFileStrAccepted = true;
      QVERIFY(!fileNode.modifyValueTo(otherFolderPath, newFileStrAccepted));
      QVERIFY(!newFileStrAccepted);
      QCOMPARE(fileNode.modifiedToValue, otherFilePath);
      QVERIFY(fileNode.isNeedApplyChange());
    }


    { // imgOptionalNode ok
      QCOMPARE(imgOptionalNode.modifiedToValue, (QVariant()));
      QVERIFY(!imgOptionalNode.isNeedApplyChange());
      // 存在且之前是初始值 -> (接受, 有修改)
      bool newImgOptStrAccepted = false;
      QVERIFY(imgOptionalNode.modifyValueTo(otherFilePath, newImgOptStrAccepted));
      QVERIFY(newImgOptStrAccepted);
      QCOMPARE(imgOptionalNode.modifiedToValue, otherFilePath);
      QVERIFY(imgOptionalNode.isNeedApplyChange());

      // 存在但和之前已知, -> (接受, 不修改)
      newImgOptStrAccepted = false;
      QVERIFY(!imgOptionalNode.modifyValueTo(otherFilePath, newImgOptStrAccepted)); // unchange
      QVERIFY(newImgOptStrAccepted);
      QCOMPARE(imgOptionalNode.modifiedToValue, otherFilePath);
      QVERIFY(imgOptionalNode.isNeedApplyChange());

      // 不存在的文件 -> (不接受, 不修改)
      newImgOptStrAccepted = true;
      QVERIFY(!imgOptionalNode.modifyValueTo("inexist/file/path", newImgOptStrAccepted));
      QVERIFY(!newImgOptStrAccepted);
      QCOMPARE(imgOptionalNode.modifiedToValue, otherFilePath);
      QVERIFY(imgOptionalNode.isNeedApplyChange());

      // 存在, 但是是文件夹 类型不一致 -> (不接受, 不修改)
      newImgOptStrAccepted = true;
      QVERIFY(!imgOptionalNode.modifyValueTo(otherFolderPath, newImgOptStrAccepted));
      QVERIFY(!newImgOptStrAccepted);
      QCOMPARE(imgOptionalNode.modifiedToValue, otherFilePath);
      QVERIFY(imgOptionalNode.isNeedApplyChange());

      // "", 接受空字符串且之前非空 (接受, 有修改)
      newImgOptStrAccepted = false;
      QVERIFY(imgOptionalNode.modifyValueTo("", newImgOptStrAccepted));
      QVERIFY(newImgOptStrAccepted);
      QCOMPARE(imgOptionalNode.modifiedToValue, (QVariant{}));
      QVERIFY(!imgOptionalNode.isNeedApplyChange());
    }

    { // folder ok
      QCOMPARE(folderNode.modifiedToValue, (QVariant()));
      QVERIFY(!folderNode.isNeedApplyChange());
      // 存在且之前是初始值 -> (接受, 有修改)
      bool newFolderStrAccepted = false;
      QVERIFY(folderNode.modifyValueTo(otherFolderPath, newFolderStrAccepted));
      QVERIFY(newFolderStrAccepted);
      QCOMPARE(folderNode.modifiedToValue, otherFolderPath);
      QVERIFY(folderNode.isNeedApplyChange());

      // 存在但和之前已知, -> (接受, 不修改)
      newFolderStrAccepted = false;
      QVERIFY(!folderNode.modifyValueTo(otherFolderPath, newFolderStrAccepted)); // unchange
      QVERIFY(newFolderStrAccepted);
      QCOMPARE(folderNode.modifiedToValue, otherFolderPath);
      QVERIFY(folderNode.isNeedApplyChange());

      // 不存在的文件夹 -> (不接受, 不修改)
      newFolderStrAccepted = true;
      QVERIFY(!folderNode.modifyValueTo("inexist/folder/path", newFolderStrAccepted));
      QVERIFY(!newFolderStrAccepted);
      QCOMPARE(folderNode.modifiedToValue, otherFolderPath);
      QVERIFY(folderNode.isNeedApplyChange());

      // 存在, 但是是文件 类型不一致 -> (不接受, 不修改)
      newFolderStrAccepted = true;
      QVERIFY(!folderNode.modifyValueTo(otherFilePath, newFolderStrAccepted));
      QVERIFY(!newFolderStrAccepted);
      QCOMPARE(folderNode.modifiedToValue, otherFolderPath);
      QVERIFY(folderNode.isNeedApplyChange());
    }

    {
      // boolNode ok
      QCOMPARE(boolNode.modifiedToValue, (QVariant()));
      QVERIFY(!boolNode.isNeedApplyChange());

      // 初始值->false (accept, changed)
      bool newBoolAccept = false;
      QVERIFY(boolNode.modifyValueTo(false, newBoolAccept));
      QVERIFY(newBoolAccept);

      // false->false (accept, not changed)
      newBoolAccept = false;
      QVERIFY(!boolNode.modifyValueTo(false, newBoolAccept));
      QVERIFY(newBoolAccept);

      // false->true (accept, changed)
      newBoolAccept = false;
      QVERIFY(boolNode.modifyValueTo(true, newBoolAccept));
      QVERIFY(newBoolAccept);
    }
  }

  void modifiedColorTo_ok() {
    StyleItemData def;
    StyleItemData group{"groupName"};
    QVERIFY(!def.modifiedColorTo("#FF01234")); // group no need modifiedColorTo
    QCOMPARE(def.modifiedToValue, (QVariant()));
    QVERIFY(!group.modifiedColorTo("#FF01234")); // group no need modifiedColorTo
    QCOMPARE(group.modifiedToValue, (QVariant()));

    StyleItemData colorNode{"ForegroundColor", "#FFFFFF", "#000000", GeneralDataType::Type::COLOR};
    QCOMPARE(colorNode.modifiedToValue, (QVariant()));

    QVERIFY(colorNode.modifiedColorTo("#FF0000"));
    QVERIFY(!colorNode.modifiedColorTo("#FF0000")); // unchange

    QVERIFY(colorNode.recoverToBackup());
    QCOMPARE(colorNode.modifiedToValue, "#000000");

    QVERIFY(!colorNode.recoverToBackup()); // no need recover again
    QCOMPARE(colorNode.modifiedToValue, "#000000");

    QVERIFY(colorNode.invalidateNewValue());
    QCOMPARE(colorNode.modifiedToValue, (QVariant()));
    QVERIFY(!colorNode.invalidateNewValue()); // no need invalidateNewValue again
    QCOMPARE(colorNode.modifiedToValue, (QVariant()));

    QVERIFY(colorNode.recoverToDefault());
    QCOMPARE(colorNode.modifiedToValue, "#FFFFFF");
    QVERIFY(!colorNode.recoverToDefault()); // no need recover again
    QCOMPARE(colorNode.modifiedToValue, "#FFFFFF");
  }

  void recoverToBackup_invalidateNewValue_ok() {
    StyleItemData def;
    StyleItemData group{"groupName"};
    QVERIFY(!def.recoverToBackup()); // group no need recover/invalidate
    QVERIFY(!def.invalidateNewValue());
    QVERIFY(!group.recoverToBackup()); // group no need recover/invalidate
    QVERIFY(!group.invalidateNewValue());

    { // 数值类型
      StyleItemData numberNode{"RowHeight", 30, 60, GeneralDataType::Type::PLAIN_INT};
      QCOMPARE(numberNode.modifiedToValue, (QVariant()));

      QVERIFY(numberNode.recoverToBackup());
      QCOMPARE(numberNode.modifiedToValue, 60);
      QVERIFY(!numberNode.recoverToBackup()); // no need recover again
      QCOMPARE(numberNode.modifiedToValue, 60);

      QVERIFY(numberNode.invalidateNewValue());
      QCOMPARE(numberNode.modifiedToValue, (QVariant()));
      QVERIFY(!numberNode.invalidateNewValue()); // no need invalidate again
      QCOMPARE(numberNode.modifiedToValue, (QVariant()));

      QVERIFY(numberNode.recoverToDefault());
      QCOMPARE(numberNode.modifiedToValue, 30);
      QVERIFY(!numberNode.recoverToDefault()); // no need recover again
      QCOMPARE(numberNode.modifiedToValue, 30);
    }

    { // 字符串类型
      StyleItemData stringNode{"FontFamily", "Microsoft YaHei UI", "Noto Sans", GeneralDataType::Type::FONT_FAMILY};
      QCOMPARE(stringNode.modifiedToValue, (QVariant()));

      QVERIFY(stringNode.recoverToBackup());
      QCOMPARE(stringNode.modifiedToValue, "Noto Sans");
      QVERIFY(!stringNode.recoverToBackup()); // no need recover again
      QCOMPARE(stringNode.modifiedToValue, "Noto Sans");

      QVERIFY(stringNode.invalidateNewValue());
      QCOMPARE(stringNode.modifiedToValue, (QVariant()));
      QVERIFY(!stringNode.invalidateNewValue()); // no need invalidate again
      QCOMPARE(stringNode.modifiedToValue, (QVariant()));

      QVERIFY(stringNode.recoverToDefault());
      QCOMPARE(stringNode.modifiedToValue, "Microsoft YaHei UI");
      QVERIFY(!stringNode.recoverToDefault()); // no need recover again
      QCOMPARE(stringNode.modifiedToValue, "Microsoft YaHei UI");
    }
  }

  void filterAccept_ok_serialize_deserialize_ignored_ok() {
    std::unique_ptr<StyleTreeNode> r0{StyleTreeNode::NewTreeNodeRoot("StyleSheet")};
    r0->setName("StyleSheetInTest");

    auto* pFont = r0->appendRow(StyleTreeNode::create(StyleItemData{"Font"}));
    auto* pFontFamily = pFont->appendRow(StyleTreeNode::create(StyleItemData{"Family"}));
    auto* pFontFamilyGeneral = pFontFamily->appendRow(StyleTreeNode::create(StyleItemData{"General", "Arial", "Times New Roman", GeneralDataType::Type::FONT_FAMILY}));
    auto* pFontFamilySpecial = pFontFamily->appendRow(StyleTreeNode::create(StyleItemData{"Special", "Microsoft YaHei UI", "Noto Sans", GeneralDataType::Type::FONT_FAMILY}));

    QCOMPARE(r0->childsCount(), 1);
    QCOMPARE(pFont->childsCount(), 1);
    QCOMPARE(pFontFamily->childsCount(), 2);
    constexpr int totalNodeCount = 1 + 1 + 1 + 2;
    QCOMPARE((*pFontFamilyGeneral == *pFontFamilyGeneral), true);
    QCOMPARE((*pFontFamilyGeneral != *pFontFamilyGeneral), false);
    // pFontFamilyGeneral 和 pFontFamilySpecial 无父子关系
    QVERIFY(!pFontFamilyGeneral->isAncestorOf(pFontFamilySpecial));
    QVERIFY(!pFontFamilySpecial->isAncestorOf(pFontFamilyGeneral));
    QVERIFY(!pFontFamilyGeneral->isDescendantOf(pFontFamilySpecial));
    QVERIFY(!pFontFamilySpecial->isDescendantOf(pFontFamilyGeneral));

    QVERIFY(pFont->isDescendantOf(r0.get()));
    QVERIFY(pFontFamily->isDescendantOf(r0.get()));
    QVERIFY(pFontFamilyGeneral->isDescendantOf(r0.get()));
    QVERIFY(pFontFamilySpecial->isDescendantOf(r0.get()));
    QVERIFY(pFontFamilyGeneral->isDescendantOf(pFontFamily));
    QVERIFY(pFontFamilySpecial->isDescendantOf(pFontFamily));

    QVERIFY(r0->isAncestorOf(pFont));
    QVERIFY(r0->isAncestorOf(pFontFamily));
    QVERIFY(r0->isAncestorOf(pFontFamilyGeneral));
    QVERIFY(r0->isAncestorOf(pFontFamilySpecial));
    QVERIFY(pFontFamily->isAncestorOf(pFontFamilyGeneral));
    QVERIFY(pFontFamily->isAncestorOf(pFontFamilySpecial));

    QCOMPARE(r0->GetConfigKey(), "StyleSheetInTest");
    QCOMPARE(pFont->GetConfigKey(), "StyleSheetInTest/Font");
    QCOMPARE(pFontFamily->GetConfigKey(), "StyleSheetInTest/Font/Family");
    QCOMPARE(pFontFamilyGeneral->GetConfigKey(), "StyleSheetInTest/Font/Family/General");
    QCOMPARE(pFontFamilySpecial->GetConfigKey(), "StyleSheetInTest/Font/Family/Special");

    { // filterAccept
      QString searchText{"NotExistName"};
      QHash<const void*, bool> passCache;
      QCOMPARE(r0->filterAccept(searchText, passCache), false);
      QCOMPARE(passCache.size(), totalNodeCount); // 遍历入口根, 无任何匹配, 所有节点过滤结果都已经缓存

      QCOMPARE(pFontFamily->filterAccept(searchText, passCache), false);
      QCOMPARE(pFontFamilyGeneral->filterAccept(searchText, passCache), false);
      QCOMPARE(pFontFamilySpecial->filterAccept(searchText, passCache), false);
      QCOMPARE(passCache.size(), totalNodeCount); // 多次查询, 缓存词典不会增大

      {
        // 目标就在itself
        searchText = "Noto Sans";
        passCache.clear();
        QCOMPARE(pFontFamilySpecial->isCurrentNodeMatch(searchText, passCache), true);
        QCOMPARE(passCache.size(), 1);

        passCache.clear();
        QCOMPARE(pFontFamilySpecial->filterAccept(searchText, passCache), true);
        QCOMPARE(passCache.size(), 1); // 先1个自己节点
      }

      {
        // 目标在2个child中的后者
        searchText = "Noto Sans";
        passCache.clear();
        QCOMPARE(pFontFamily->isChildNodeMatch(searchText, passCache), true);
        QCOMPARE(passCache.size(), 2);

        passCache.clear();
        QCOMPARE(pFontFamily->filterAccept(searchText, passCache), true);
        QCOMPARE(passCache.size(), 1 + 2); // 先1个自己节点, 再2个子节点
      }

      {
        // 目标就在parent
        searchText = "Font";
        passCache.clear();
        QCOMPARE(pFontFamily->isParentNodeMatch(searchText, passCache), true);
        QCOMPARE(passCache.size(), 1);

        passCache.clear();
        QCOMPARE(pFontFamilyGeneral->filterAccept(searchText, passCache), true);
        QCOMPARE(passCache.size(), 1 + 0 + 2); // 先1个自己节点, 再0个子节点, 再2次父节点
      }
    }

    QByteArray ba;
    QDataStream writeDs(&ba, QIODevice::WriteOnly);
    writeDs << *r0;
    QCOMPARE(writeDs.status(), QDataStream::Status::Ok);
    QVERIFY(ba.isEmpty());

    std::unique_ptr<StyleTreeNode> recoverr0{StyleTreeNode::NewTreeNodeRoot("StyleSheetNodeOnly")};
    QDataStream readDs(ba);
    readDs >> *recoverr0;
    QCOMPARE(readDs.status(), QDataStream::Status::Ok);

    QCOMPARE(recoverr0->childsCount(), 0);
    QCOMPARE((*recoverr0 == *recoverr0), true);
    QCOMPARE((*recoverr0 != *recoverr0), false);

    QCOMPARE((*recoverr0 == *pFontFamilyGeneral), false);
    QCOMPARE((*recoverr0 != *pFontFamilyGeneral), true);

    QCOMPARE(recoverr0->GetConfigKey(), "StyleSheetNodeOnly");
  }

  void fromPairList_ok() {
    const QString rootName{"StyleSheet"};
    const QList<std::pair<QString, StyleItemData>> pairList{
        {"Font/Family", StyleItemData{"General", "Noto Sans", "Noto Sans", GeneralDataType::Type::FONT_FAMILY}},
        {"Font/Family", StyleItemData{"Code", "Noto Sans", "Noto Sans", GeneralDataType::Type::FONT_FAMILY}},
        {"Font/Size", StyleItemData{"General", 14, 14, GeneralDataType::Type::PLAIN_INT}},
        {"Font/Size", StyleItemData{"QTabBar", 15, 15, GeneralDataType::Type::PLAIN_INT}},
        {"LightColor/Background", StyleItemData{"General", "#FFFFFF", "#FFFFFF", GeneralDataType::Type::COLOR}},
        {"DarkColor/Background", StyleItemData{"General", "#000000", "#000000", GeneralDataType::Type::COLOR}},
    };

    std::unique_ptr<StyleTreeNode> pRoot{StyleTreeNode::NewTreeNodeRoot(rootName)};
    {
      auto* pFont = pRoot->appendRow(StyleTreeNode::create(StyleItemData{"Font"}));
      {
        auto* pFontFamily = pFont->appendRow(StyleTreeNode::create(StyleItemData{"Family"}));
        {
          pFontFamily->appendRow(StyleTreeNode::create(StyleItemData{"General", "Noto Sans", "Noto Sans", GeneralDataType::Type::FONT_FAMILY}));
          pFontFamily->appendRow(StyleTreeNode::create(StyleItemData{"Code", "Noto Sans", "Noto Sans", GeneralDataType::Type::FONT_FAMILY}));
        }
        auto* pFontSize = pFont->appendRow(StyleTreeNode::create(StyleItemData{"Size"}));
        {
          pFontSize->appendRow(StyleTreeNode::create(StyleItemData{"General", 14, 14, GeneralDataType::Type::PLAIN_INT}));
          pFontSize->appendRow(StyleTreeNode::create(StyleItemData{"QTabBar", 15, 15, GeneralDataType::Type::PLAIN_INT}));
        }
      }

      auto* pColorLight = pRoot->appendRow(StyleTreeNode::create(StyleItemData{"LightColor"}));
      {
        auto* pColorLightBg = pColorLight->appendRow(StyleTreeNode::create(StyleItemData{"Background"}));
        {
          pColorLightBg->appendRow(StyleTreeNode::create(StyleItemData{"General", "#FFFFFF", "#FFFFFF", GeneralDataType::Type::COLOR}));
        }
      }

      auto* pColorDark = pRoot->appendRow(StyleTreeNode::create(StyleItemData{"DarkColor"}));
      {
        auto* pColorDarkBg = pColorDark->appendRow(StyleTreeNode::create(StyleItemData{"Background"}));
        {
          pColorDarkBg->appendRow(StyleTreeNode::create(StyleItemData{"General", "#000000", "#000000", GeneralDataType::Type::COLOR}));
        }
      }
    }

    std::unique_ptr<StyleTreeNode> pRecoverRoot = StyleTreeNode::fromPairList(pairList, rootName);
    QVERIFY(pRecoverRoot);
    QCOMPARE(*pRecoverRoot, *pRoot);
  }

  void FindNode_ok() {
    std::unique_ptr<StyleTreeNode> pRoot{StyleTreeNode::NewTreeNodeRoot("StyleSheet")};
    auto* pFont = pRoot->appendRow(StyleTreeNode::create(StyleItemData{"Font"}));

    QVERIFY(pRoot->isAncestorOf(pFont));

    auto* p0 = pRoot->FindNode("Not/Exist/Path/Font");
    auto* p1 = pRoot->FindNode("StyleSheet/Not/Exist/Path/Font");
    auto* p2 = pRoot->FindNode("");
    auto* p3 = pRoot->FindNode("StyleSheet/Font");
    QCOMPARE(p0, nullptr);
    QCOMPARE(p1, nullptr);
    QCOMPARE(p2, pRoot.get()); // empty return root
    QCOMPARE(p3, pFont);
  }
};

#include "StyleItemDataTest.moc"
REGISTER_TEST(StyleItemDataTest, false)
