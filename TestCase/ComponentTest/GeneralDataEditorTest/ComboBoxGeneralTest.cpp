#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "ComboBoxGeneral.h"
#include "ComboBoxString.h"
#include "ComboBoxEnum.h"
#include "EndToExposePrivateMember.h"

#include "StyleEnum.h"

class ComboBoxGeneralTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void ComboBoxString_ok() {
    QWidget wid;
    auto* fontFamilyComboBox = ComboBoxGeneral::create(GeneralDataType::Type::FONT_FAMILY, &wid);
    QVERIFY(fontFamilyComboBox != nullptr);
    QCOMPARE(fontFamilyComboBox->type(), GeneralDataType::Type::FONT_FAMILY);

    auto* fontFamilyComboBoxDerived = dynamic_cast<const ComboBoxString*>(fontFamilyComboBox);
    QVERIFY(fontFamilyComboBoxDerived != nullptr);

    const ComboBoxString::T_CANDIDATES_STR_LST* fontFamilyComboBoxDerivedCandidates = fontFamilyComboBoxDerived->mCandidates;
    QVERIFY(fontFamilyComboBoxDerivedCandidates != nullptr);

    // getDisplayString, displayTextFromDisplayRole ok
    QVariant editRoleValue{"NotExistFontFamily"};
    QVariant displayRoleValue{"NotExistFontFamily"};
    QCOMPARE("not allowed: data[NotExistFontFamily] from model", ComboBoxString::getDisplayString(editRoleValue, *fontFamilyComboBoxDerivedCandidates));
    QCOMPARE("not allowed: data[NotExistFontFamily] from model", ComboBoxString::getDisplayString(displayRoleValue, *fontFamilyComboBoxDerivedCandidates));
    QCOMPARE("not allowed: data[NotExistFontFamily] from model", ComboBoxString::displayTextFromDisplayRole(GeneralDataType::Type::FONT_FAMILY, displayRoleValue));

    editRoleValue = QVariant{Style::DEFAULT_FONT_FAMILY};    // enum ok
    displayRoleValue = QVariant{Style::DEFAULT_FONT_FAMILY}; //

    QCOMPARE(Style::DEFAULT_FONT_FAMILY, ComboBoxString::getDisplayString(editRoleValue, *fontFamilyComboBoxDerivedCandidates));
    QCOMPARE(Style::DEFAULT_FONT_FAMILY, ComboBoxString::getDisplayString(displayRoleValue, *fontFamilyComboBoxDerivedCandidates));
    QCOMPARE(Style::DEFAULT_FONT_FAMILY, ComboBoxString::displayTextFromDisplayRole(GeneralDataType::Type::FONT_FAMILY, displayRoleValue));
    QCOMPARE(Style::DEFAULT_FONT_FAMILY, ComboBoxGeneral::displayText(GeneralDataType::Type::FONT_FAMILY, displayRoleValue));

    // getSetDataEditRoleValue ok
    fontFamilyComboBox->setCurrentText(Style::DEFAULT_FONT_FAMILY);
    QCOMPARE(fontFamilyComboBox->getSetDataEditRoleValue(), QVariant{Style::DEFAULT_FONT_FAMILY});
    fontFamilyComboBox->setCurrentText("NotExistFontFamily"); // no edit, no write
    QCOMPARE(fontFamilyComboBox->getSetDataEditRoleValue(), QVariant{Style::DEFAULT_FONT_FAMILY});

    // updateCurrentTextFromEditRole
    fontFamilyComboBox->updateCurrentTextFromEditRole("NotExistFontFamily");
    QCOMPARE(fontFamilyComboBox->currentText(), Style::DEFAULT_FONT_FAMILY);
    fontFamilyComboBox->updateCurrentTextFromEditRole(Style::DEFAULT_FONT_FAMILY);
    QCOMPARE(fontFamilyComboBox->currentText(), Style::DEFAULT_FONT_FAMILY);
  }

  void ComboBoxEnum_ok() {
    QWidget wid;

    auto* fontWeightComboBox = ComboBoxGeneral::create(GeneralDataType::Type::FONT_WEIGHT, &wid);
    auto* fontStyleComboBox = ComboBoxGeneral::create(GeneralDataType::Type::FONT_STYLE, &wid);
    auto* presetComboBox = ComboBoxGeneral::create(GeneralDataType::Type::RANGE_INT_STYLE_PRESET, &wid);
    auto* themeComboBox = ComboBoxGeneral::create(GeneralDataType::Type::RANGE_INT_STYLE_THEME, &wid);
    auto* errorTypeComboBox = ComboBoxGeneral::create(GeneralDataType::Type::ERROR_TYPE, &wid);

    QVERIFY(fontWeightComboBox != nullptr);
    QVERIFY(fontStyleComboBox != nullptr);
    QVERIFY(presetComboBox != nullptr);
    QVERIFY(themeComboBox != nullptr);
    QVERIFY(errorTypeComboBox == nullptr);

    QCOMPARE(fontWeightComboBox->type(), GeneralDataType::Type::FONT_WEIGHT);

    auto* fontWeightComboBoxDerived = dynamic_cast<const ComboBoxEnum*>(fontWeightComboBox);
    auto* fontStyleComboBoxDerived = dynamic_cast<const ComboBoxEnum*>(fontStyleComboBox);
    auto* presetComboBoxDerived = dynamic_cast<const ComboBoxEnum*>(presetComboBox);
    auto* themeComboBoxDerived = dynamic_cast<const ComboBoxEnum*>(themeComboBox);
    QVERIFY(fontWeightComboBoxDerived != nullptr);
    QVERIFY(fontStyleComboBoxDerived != nullptr);
    QVERIFY(presetComboBoxDerived != nullptr);
    QVERIFY(themeComboBoxDerived != nullptr);

    const ComboBoxEnum::T_CANDIDATES_STR_TO_ENUM* fontWeightComboBoxDerivedCandidates = fontWeightComboBoxDerived->mCandidates;
    const ComboBoxEnum::T_CANDIDATES_STR_TO_ENUM* fontStyleComboBoxDerivedCandidates = fontStyleComboBoxDerived->mCandidates;
    const ComboBoxEnum::T_CANDIDATES_STR_TO_ENUM* presetComboBoxDerivedCandidates = presetComboBoxDerived->mCandidates;
    const ComboBoxEnum::T_CANDIDATES_STR_TO_ENUM* themeComboBoxDerivedCandidates = themeComboBoxDerived->mCandidates;

    const ComboBoxEnum::T_ENUM_TO_CANDIDATES_STR* fontWeightComboBoxDerivedCandidatesDisplay = fontWeightComboBoxDerived->mCandidatesDisplay;
    const ComboBoxEnum::T_ENUM_TO_CANDIDATES_STR* fontStyleComboBoxDerivedCandidatesDisplay = fontStyleComboBoxDerived->mCandidatesDisplay;
    const ComboBoxEnum::T_ENUM_TO_CANDIDATES_STR* presetComboBoxDerivedCandidatesDisplay = presetComboBoxDerived->mCandidatesDisplay;
    const ComboBoxEnum::T_ENUM_TO_CANDIDATES_STR* themeComboBoxDerivedCandidatesDisplay = themeComboBoxDerived->mCandidatesDisplay;

    QVERIFY(fontWeightComboBoxDerivedCandidates != nullptr);
    QVERIFY(fontStyleComboBoxDerivedCandidates != nullptr);
    QVERIFY(presetComboBoxDerivedCandidates != nullptr);
    QVERIFY(themeComboBoxDerivedCandidates != nullptr);

    QVERIFY(fontWeightComboBoxDerivedCandidatesDisplay != nullptr);
    QVERIFY(fontStyleComboBoxDerivedCandidatesDisplay != nullptr);
    QVERIFY(presetComboBoxDerivedCandidatesDisplay != nullptr);
    QVERIFY(themeComboBoxDerivedCandidatesDisplay != nullptr);

    // getDisplayString, displayTextFromDisplayRole ok
    QVariant editRoleValue{"NotEnumIntFontWeight"};
    QVariant displayRoleValue{"NotEnumIntFontWeight"};
    QCOMPARE("not allowed: data[NaN] from model", ComboBoxEnum::getDisplayString(editRoleValue, *fontWeightComboBoxDerivedCandidatesDisplay));
    QCOMPARE("not allowed: data[NaN] from model", ComboBoxEnum::getDisplayString(displayRoleValue, *fontWeightComboBoxDerivedCandidatesDisplay));
    QCOMPARE("not allowed: data[NaN] from model", ComboBoxEnum::displayTextFromDisplayRole(GeneralDataType::Type::FONT_WEIGHT, displayRoleValue));
    QCOMPARE("not allowed: data[NaN] from model", ComboBoxGeneral::displayText(GeneralDataType::Type::FONT_WEIGHT, displayRoleValue));

    editRoleValue = QVariant{9999};    // no enum correspond to large number
    displayRoleValue = QVariant{9998}; //
    QCOMPARE("not allowed: Enum[9999]", ComboBoxEnum::getDisplayString(editRoleValue, *fontWeightComboBoxDerivedCandidatesDisplay));
    QCOMPARE("not allowed: Enum[9998]", ComboBoxEnum::getDisplayString(displayRoleValue, *fontWeightComboBoxDerivedCandidatesDisplay));
    QCOMPARE("not allowed: Enum[9998]", ComboBoxEnum::displayTextFromDisplayRole(GeneralDataType::Type::FONT_WEIGHT, displayRoleValue));

    editRoleValue = QVariant{(int) QFont::Weight::Normal};  // enum ok
    displayRoleValue = QVariant{(int) QFont::Weight::Bold}; //

    QCOMPARE("Normal (50)", ComboBoxEnum::getDisplayString(editRoleValue, *fontWeightComboBoxDerivedCandidatesDisplay));
    QCOMPARE("Bold (75)", ComboBoxEnum::getDisplayString(displayRoleValue, *fontWeightComboBoxDerivedCandidatesDisplay));
    QCOMPARE("Bold (75)", ComboBoxEnum::displayTextFromDisplayRole(GeneralDataType::Type::FONT_WEIGHT, displayRoleValue));

    // getSetDataEditRoleValue
    fontWeightComboBox->setCurrentText("Normal (50)");
    QCOMPARE(fontWeightComboBox->getSetDataEditRoleValue(), QVariant{(int) QFont::Weight::Normal});
    fontWeightComboBox->setCurrentText("NotEnumIntFontWeight");
    QCOMPARE(fontWeightComboBox->getSetDataEditRoleValue(), QVariant{(int) QFont::Weight::Normal});

    // updateCurrentTextFromEditRole
    fontWeightComboBox->updateCurrentTextFromEditRole("NotEnumIntFontWeight");
    QCOMPARE(fontWeightComboBox->currentText(), "Normal (50)");

    fontWeightComboBox->updateCurrentTextFromEditRole(9999);
    QCOMPARE(fontWeightComboBox->currentText(), "Normal (50)");

    fontWeightComboBox->updateCurrentTextFromEditRole("Normal (50)");
    QCOMPARE(fontWeightComboBox->currentText(), "Normal (50)");
  }
};

#include "ComboBoxGeneralTest.moc"
REGISTER_TEST(ComboBoxGeneralTest, false)
