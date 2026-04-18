#include "StyleSheetGetter.h"
#include "PublicMacro.h"
#include "MemoryKey.h"
#include "PreferenceActions.h"
#include <unordered_set>

namespace FontCfg {
QString ReadFontString() {
  return QString{R"(font-family: "%1"; font-size: %2px; font-weight: %3; font-style: %4;)"}
      .arg(GetFontFamily())
      .arg(GetFontSize())
      .arg(GetFontWeightString(), GetFontStyleString());
}

QString GetFontFamily() {
  return Configuration().value("StyleSheet/Font/Family/General", mFontFamilyDef).toString();
}

int GetFontSize() {
  return Configuration().value("StyleSheet/Font/Size/General", mFontSizeDef).toInt();
}

int GetFontSizeTab() {
  return Configuration().value("StyleSheet/Font/Size/QTabBar", mFontSizeTabDef).toInt();
}

QFont::Weight GetFontWeight() {
  static const std::unordered_set<int> weightsSet{//
                                                  QFont::Weight::Thin,
                                                  QFont::Weight::ExtraLight,
                                                  QFont::Weight::Light,
                                                  QFont::Weight::Normal,
                                                  QFont::Weight::Medium,
                                                  QFont::Weight::DemiBold,
                                                  QFont::Weight::Bold,
                                                  QFont::Weight::ExtraBold,
                                                  QFont::Weight::Black};
  int weight = Configuration().value("StyleSheet/Font/Weight/General", mFontWeightDef).toInt();
  if (weightsSet.find(weight) == weightsSet.cend()) {
    return QFont::Weight::Normal;
  }
  return static_cast<QFont::Weight>(weight);
}

QFont::Style GetFontStyle() {
  static const std::unordered_set<int> styleSet{QFont::Style::StyleNormal, QFont::Style::StyleItalic, QFont::Style::StyleOblique};
  int style = Configuration().value("StyleSheet/Font/Style/General", mFontStyleDef).toInt();
  if (styleSet.find(style) == styleSet.cend()) {
    return QFont::Style::StyleNormal;
  }
  return static_cast<QFont::Style>(style);
}

QString GetFontWeightString() {
  QFont::Weight weightE = GetFontWeight();
  switch (weightE) {
    case QFont::Weight::DemiBold:
    case QFont::Weight::Bold:
    case QFont::Weight::ExtraBold:
      return "bold";
    default:
      return "normal";
  }
}

QString GetFontStyleString() {
  QFont::Style styleE = GetFontStyle();
  switch (styleE) {
    case QFont::Style::StyleItalic:
    case QFont::Style::StyleOblique:
      return "italic";
    default:
      return "normal";
  }
}

QFont ReadFont() {
  return {GetFontFamily(), GetFontSize(), GetFontWeight(), GetFontStyle() == QFont::Style::StyleItalic};
}

void updateFont(const QFont& newFont) {
  Configuration().setValue("StyleSheet/Font/Family/General", newFont.family());
  Configuration().setValue("StyleSheet/Font/Size/General", newFont.pointSize());
  Configuration().setValue("StyleSheet/Font/Weight/General", newFont.weight());
  Configuration().setValue("StyleSheet/Font/Style/General", newFont.style());

  g_PreferenceActions().initStyleSheet(false);
}
} // namespace FontCfg

namespace ColorCfg {
QString GetColorBackgroundGeneral(Style::StyleSheetE styleE) {
  if (styleE == Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG) {
    return Configuration().value("StyleSheet/DarkColor/Background/General", ColorBackgroundGeneralDef(styleE)).toString();
  }
  return Configuration().value("StyleSheet/LightColor/Background/General", ColorBackgroundGeneralDef(styleE)).toString();
}
QString GetColorBackgroundAlternateRow(Style::StyleSheetE styleE) {
  if (styleE == Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG) {
    return Configuration().value("StyleSheet/DarkColor/Background/AlternateRow", ColorBackgroundAlternateRowDef(styleE)).toString();
  }
  return Configuration().value("StyleSheet/LightColor/Background/AlternateRow", ColorBackgroundAlternateRowDef(styleE)).toString();
}
QString GetColorBackgroundHover(Style::StyleSheetE styleE) {
  if (styleE == Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG) {
    return Configuration().value("StyleSheet/DarkColor/Background/Hover", ColorBackgroundHoverDef(styleE)).toString();
  }
  return Configuration().value("StyleSheet/LightColor/Background/Hover", ColorBackgroundHoverDef(styleE)).toString();
}
QString GetColorBackgroundSelectedInactive(Style::StyleSheetE styleE) {
  if (styleE == Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG) {
    return Configuration().value("StyleSheet/DarkColor/Background/SelectedInactive", ColorBackgroundSelectedInactiveDef(styleE)).toString();
  }
  return Configuration().value("StyleSheet/LightColor/Background/SelectedInactive", ColorBackgroundSelectedInactiveDef(styleE)).toString();
}
QString GetColorBackgroundSelectedActive(Style::StyleSheetE styleE) {
  if (styleE == Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG) {
    return Configuration().value("StyleSheet/DarkColor/Background/SelectedActive", ColorBackgroundSelectedActiveDef(styleE)).toString();
  }
  return Configuration().value("StyleSheet/LightColor/Background/SelectedActive", ColorBackgroundSelectedActiveDef(styleE)).toString();
}
QString GetColorBackgroundMenuChecked(Style::StyleSheetE styleE) {
  if (styleE == Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG) {
    return Configuration().value("StyleSheet/DarkColor/Background/MenuChecked", ColorBackgroundMenuCheckedDef(styleE)).toString();
  }
  return Configuration().value("StyleSheet/LightColor/Background/MenuChecked", ColorBackgroundMenuCheckedDef(styleE)).toString();
}
QString GetColorBackgroundMenuSelected(Style::StyleSheetE styleE) {
  if (styleE == Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG) {
    return Configuration().value("StyleSheet/DarkColor/Background/MenuSelected", ColorBackgroundMenuSelectedDef(styleE)).toString();
  }
  return Configuration().value("StyleSheet/LightColor/Background/MenuSelected", ColorBackgroundMenuSelectedDef(styleE)).toString();
}
QString GetColorGridLine(Style::StyleSheetE styleE) {
  if (styleE == Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG) {
    return Configuration().value("StyleSheet/DarkColor/GridLine", ColorGridLineDef(styleE)).toString();
  }
  return Configuration().value("StyleSheet/LightColor/GridLine", ColorGridLineDef(styleE)).toString();
}
QString GetColorBorderGeneral(Style::StyleSheetE styleE) {
  if (styleE == Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG) {
    return Configuration().value("StyleSheet/DarkColor/Border/General", ColorBorderGeneralDef(styleE)).toString();
  }
  return Configuration().value("StyleSheet/LightColor/Border/General", ColorBorderGeneralDef(styleE)).toString();
}
QString GetColorBorderMenuRight(Style::StyleSheetE styleE) {
  if (styleE == Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG) {
    return Configuration().value("StyleSheet/DarkColor/Border/MenuRight", ColorBorderMenuRightDef(styleE)).toString();
  }
  return Configuration().value("StyleSheet/LightColor/Border/MenuRight", ColorBorderMenuRightDef(styleE)).toString();
}
QString GetColorForegroundGeneral(Style::StyleSheetE styleE) {
  if (styleE == Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG) {
    return Configuration().value("StyleSheet/DarkColor/Foreground/General", ColorForegroundGeneralDef(styleE)).toString();
  }
  return Configuration().value("StyleSheet/LightColor/Foreground/General", ColorForegroundGeneralDef(styleE)).toString();
}

QString ColorBackgroundGeneralDef(Style::StyleSheetE styleE) {
  if (styleE == Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG) {
    return "#3F3B39";
  }
  return "#FFFFFF";
}
QString ColorBackgroundAlternateRowDef(Style::StyleSheetE styleE) {
  if (styleE == Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG) {
    return "#5C5C5C";
  }
  return "#F5F5F5"; // (245,245,245)
}

QString ColorBackgroundHoverDef(Style::StyleSheetE styleE) {
  if (styleE == Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG) {
    return "#3C3C3C";
  }
  return "#CCEBFF";
}
QString ColorBackgroundSelectedInactiveDef(Style::StyleSheetE styleE) {
  return "#999999";
}
QString ColorBackgroundSelectedActiveDef(Style::StyleSheetE styleE) {
  return "#99D1FF";
}
QString ColorBackgroundMenuCheckedDef(Style::StyleSheetE styleE) {
  return "#99D1FF";
}
QString ColorBackgroundMenuSelectedDef(Style::StyleSheetE styleE) {
  return "#CCEBFF";
}
QString ColorGridLineDef(Style::StyleSheetE styleE) {
  if (styleE == Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG) {
    return "#3C3C3C";
  }
  return "#D8D8D8";
}
QString ColorBorderGeneralDef(Style::StyleSheetE styleE) {
  return "#CCEBFF";
}
QString ColorBorderMenuRightDef(Style::StyleSheetE styleE) {
  if (styleE == Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG) {
    return "#CCEBFF";
  }
  return "#3C3C3C";
}
QString ColorForegroundGeneralDef(Style::StyleSheetE styleE) {
  if (styleE == Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG) {
    return "#FFFFFF";
  }
  return "#000000";
}

} // namespace ColorCfg

const StyleSheetGetter& StyleSheetGetter::GetInst() {
  static StyleSheetGetter inst;
  return inst;
}

QString StyleSheetGetter::operator()(Style::StyleSheetE styleE) const {
  QString styleSheets;
  styleSheets.reserve(10000);
  LOG_D("Components count: %d", GetRegistry().size());
  for (const auto& pComp : GetRegistry()) {
    if (!pComp) {
      LOG_E("pComp cannot be nullptr");
      continue;
    }
    styleSheets += pComp->GetStyleSheet(styleE);
  }
  return styleSheets;
}

bool StyleSheetGetter::Register(DerivedPtr creator) {
  CHECK_NULLPTR_RETURN_FALSE(creator);
  GetRegistry().push_back(std::move(creator));
  return true;
}

std::vector<StyleSheetGetter::DerivedPtr>& StyleSheetGetter::GetRegistry() {
  static std::vector<DerivedPtr> vec;
  return vec;
}
