#include "EnumComboBox.h"
#include "StyleEnum.h"
#include "Logger.h"
#include <QMap>

EnumComboBox::EnumComboBox(GeneralDataType::Type gDataType, QWidget *parent)
  : GeneralComboBox{gDataType, parent} {
  std::tie(mCandidates, mCandidatesDisplay) = GetCandidates(gDataType);
  addItems(mCandidates->keys());
}

QVariant EnumComboBox::getSetDataEditRoleValue() const {
  QString rawText = currentText();
  auto it = mCandidates->find(rawText);
  if (it == mCandidates->cend()) {
    // 严格检查即将写入的配置
    LOG_W("No value correspond to text[%s], type[%d]", qPrintable(rawText), type());
    return {};
  }
  return it.value();
}

void EnumComboBox::updateCurrentTextFromEditRole(const QVariant &editRoleData) {
  const QString editModeDisplayText = getDisplayString(editRoleData, *mCandidatesDisplay);
  GeneralComboBox::updateCurrentTextFromEditRole(editModeDisplayText);
}

QString EnumComboBox::displayTextFromDisplayRole(GeneralDataType::Type gDataType, const QVariant &displayRoleData) {
  const T_ENUM_TO_CANDIDATES_STR *const candidatesDisplay{GetCandidates(gDataType).second};
  const QString displayModeDisplayText = getDisplayString(displayRoleData, *candidatesDisplay);
  return displayModeDisplayText;
}

EnumComboBox::PAIR_TYPE EnumComboBox::GetCandidates(GeneralDataType::Type gDataType) {
  static const T_CANDIDATES_STR_TO_ENUM CANDIDATES_DEFAULT;
  static const QMap<int, T_CANDIDATES_STR_TO_ENUM> CANDIDATES_MAP{
#define makeEnumPair(keyStr, enumValue) \
  { \
    (keyStr + QString::asprintf(" (%d)", static_cast<int>(enumValue))), static_cast<int>(enumValue) \
  }
      {GeneralDataType::Type::FONT_WEIGHT, //
       {
           makeEnumPair("Thin", QFont::Weight::Thin),             //
           makeEnumPair("ExtraLight", QFont::Weight::ExtraLight), //
           makeEnumPair("Light", QFont::Weight::Light),           //
           makeEnumPair("Normal", QFont::Weight::Normal),         //
           makeEnumPair("Medium", QFont::Weight::Medium),         //
           makeEnumPair("DemiBold", QFont::Weight::DemiBold),     //
           makeEnumPair("Bold", QFont::Weight::Bold),             //
           makeEnumPair("ExtraBold", QFont::Weight::ExtraBold),   //
           makeEnumPair("Black", QFont::Weight::Black),           //
       }},
      {GeneralDataType::Type::FONT_STYLE, //
       {
           makeEnumPair("StyleNormal", QFont::Style::StyleNormal),   //
           makeEnumPair("StyleItalic", QFont::Style::StyleItalic),   //
           makeEnumPair("StyleOblique", QFont::Style::StyleOblique), //
       }},
      {GeneralDataType::Type::RANGE_INT_STYLE_PRESET, //
       {
           makeEnumPair("WindowsVista", Style::StylePresetE::PRESET_WINDOWS_VISTA), //
           makeEnumPair("Windows", Style::StylePresetE::PRESET_WINDOWS),            //
           makeEnumPair("Fusion", Style::StylePresetE::PRESET_FUSION),              //
           makeEnumPair("MacOS", Style::StylePresetE::PRESET_MACOS),                //
       }},
      {GeneralDataType::Type::RANGE_INT_STYLE_THEME, //
       {
           makeEnumPair("Light", Style::StyleThemeE::THEME_LIGHT),        //
           makeEnumPair("Dark", Style::StyleThemeE::THEME_DARK_MOON_FOG), //
           makeEnumPair("None", Style::StyleThemeE::THEME_NONE),          //
       }},
#undef makeEnumPair
  };

  static const T_ENUM_TO_CANDIDATES_STR CANDIDATES_DISPLAY_DEFAULT;
  static const QMap<int, T_ENUM_TO_CANDIDATES_STR> CANDIDATES_DISPLAY_MAP{[]() -> QMap<int, T_ENUM_TO_CANDIDATES_STR> { //
    QMap<int, T_ENUM_TO_CANDIDATES_STR> ans;
    for (auto it = CANDIDATES_MAP.cbegin(); it != CANDIDATES_MAP.cend(); ++it) {
      T_ENUM_TO_CANDIDATES_STR int2disp;
      const T_CANDIDATES_STR_TO_ENUM &candidates = it.value();
      for (auto str2IntIt = candidates.cbegin(); str2IntIt != candidates.cend(); ++str2IntIt) {
        int2disp[str2IntIt.value()] = str2IntIt.key();
      }
      ans[it.key()] = int2disp;
    }
    return ans;
  }()};

  const T_CANDIDATES_STR_TO_ENUM *str2EnumPtr{nullptr};
  {
    auto it = CANDIDATES_MAP.find(gDataType);
    if (it == CANDIDATES_MAP.cend()) {
      str2EnumPtr = &CANDIDATES_DEFAULT;
    } else {
      str2EnumPtr = &it.value();
    }
  }
  const T_ENUM_TO_CANDIDATES_STR *enum2StrPtr{nullptr};
  {
    auto it = CANDIDATES_DISPLAY_MAP.find(gDataType);
    if (it == CANDIDATES_DISPLAY_MAP.cend()) {
      enum2StrPtr = &CANDIDATES_DISPLAY_DEFAULT;
    } else {
      enum2StrPtr = &it.value();
    }
  }

  return std::make_pair(str2EnumPtr, enum2StrPtr);
}

QString EnumComboBox::getDisplayString(const QVariant &variantData, const T_ENUM_TO_CANDIDATES_STR &pCandidateDisp) {
  // variantData can from editRole, displyRole
  bool bInt{false};
  int curEnumInt = variantData.toInt(&bInt);
  if (!bInt) {
    return "not allowed: data[NaN] from model";
  }
  auto it = pCandidateDisp.find(curEnumInt);
  if (it == pCandidateDisp.cend()) {
    return QString{"not allowed: Enum[%1]"}.arg(curEnumInt);
  }
  return it.value();
}
