#include "StyleSheetGetter.h"
#include "PublicMacro.h"
#include "MemoryKey.h"
#include <unordered_set>

namespace FontCfg {
bool isCoarseEqual(const QFont& lhs, const QFont& rhs) {
  const bool bEq{lhs.family() == rhs.family() && lhs.pointSize() == rhs.pointSize() && lhs.weight() == rhs.weight() && lhs.style() == rhs.style()};
  if (!bEq) {
    LOG_D("family: %s, %s", qPrintable(lhs.family()), qPrintable(rhs.family()));
    LOG_D("pointSize: %d, %d", lhs.pointSize(), rhs.pointSize());
    LOG_D("weight: %d, %d", lhs.weight(), rhs.weight());
    LOG_D("style: %d, %d", lhs.style(), rhs.style());
  }
  return bEq;
}

QString Font2String(const QFont& font) {
  return QString{R"(font-family: "%1"; font-size: %2px; font-weight: %3; font-style: %4;)"} //
      .arg(font.family())
      .arg(font.pointSize())
      .arg(GetFontWeightString(font.weight()), GetFontStyleString(font.style()));
}

QFont ReadGeneralFont() {
  return {GetFontFamily(), GetFontSize(), GetFontWeight(), GetFontStyle() == QFont::Style::StyleItalic};
}

QString ReadFontGeneralString() {
  QFont generalFont = ReadGeneralFont();
  return Font2String(generalFont);
}

QString GetFontFamily() {
  return StyleSheetGetter::GetInst().curValue("StyleSheet/Font/Family/General").toString();
}
QString GetFontFamilyCode() {
  return StyleSheetGetter::GetInst().curValue("StyleSheet/Font/Family/Code").toString();
}
int GetFontSize() {
  return StyleSheetGetter::GetInst().curValue("StyleSheet/Font/Size/General").toInt();
}

int GetFontSizeTab() {
  return StyleSheetGetter::GetInst().curValue("StyleSheet/Font/Size/QTabBar").toInt();
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
  static const auto& inst = StyleSheetGetter::GetInst();
  bool bKeyExist{false};
  int weight = inst.curValue("StyleSheet/Font/Weight/General", &bKeyExist).toInt();
  if (!bKeyExist || weightsSet.find(weight) == weightsSet.cend()) {
    return QFont::Weight::Normal;
  }
  return static_cast<QFont::Weight>(weight);
}

QFont::Style GetFontStyle() {
  static const std::unordered_set<int> styleSet{QFont::Style::StyleNormal, QFont::Style::StyleItalic, QFont::Style::StyleOblique};
  static const auto& inst = StyleSheetGetter::GetInst();
  bool bKeyExist{false};
  int style = inst.curValue("StyleSheet/Font/Style/General", &bKeyExist).toInt();
  if (!bKeyExist || styleSet.find(style) == styleSet.cend()) {
    return QFont::Style::StyleNormal;
  }
  return static_cast<QFont::Style>(style);
}

QString GetFontWeightString(int weightE) {
  switch (weightE) {
    case QFont::Weight::DemiBold:
    case QFont::Weight::Bold:
    case QFont::Weight::ExtraBold:
      return "bold";
    default:
      return "normal";
  }
}

QString GetFontGeneralWeightString() {
  QFont::Weight weightE = GetFontWeight();
  return GetFontWeightString(weightE);
}

QString GetFontStyleString(int styleE) {
  switch (styleE) {
    case QFont::Style::StyleItalic:
    case QFont::Style::StyleOblique:
      return "italic";
    default:
      return "normal";
  }
}

QString GetFontGeneralStyleString() {
  QFont::Style styleE = GetFontStyle();
  return GetFontStyleString(styleE);
}

} // namespace FontCfg

const StyleSheetGetter& StyleSheetGetter::GetInst() {
  static StyleSheetGetter inst;
  return inst;
}

StyleSheetGetter::StyleSheetGetter() {
  init();
}

void StyleSheetGetter::init() const {
  InitColorValue("Background/General", "#FFFFFF", "#000000");
  InitColorValue("Background/Disabled", "#F0F0F0", "#2D2D2D");
  InitColorValue("Background/Label", "#F5F6F7", "#202020");
  InitColorValue("Background/LineEdit", "#FFFFFF", "#252525");
  InitColorValue("Background/ComboBox", "#E1E1E1", "#252525");
  InitColorValue("Background/ToolBar", "#F5F6F7", "#323232");
  InitColorValue("Background/StatusBar", "#FFFFFF", "#333333");
  InitColorValue("Background/TabBar/tabNotSelected", "#FFFFFF", "#000000");
  InitColorValue("Background/TabBar/tabSelected", "#CCE8FF", "#75673D");
  InitColorValue("Background/TabBar/tabHover", "#FDFDFF", "#323232");
  InitColorValue("Background/Menu/Item", "#EEEEEE", "#2B2B2B");
  InitColorValue("Background/Menu/ItemChecked", "#C9E8F7", "#5E5E5E");
  InitColorValue("Background/Menu/ItemHovered", "#E8EFF7", "#474747");
  InitColorValue("Background/Menu/Separator", "#919191", "#7F7F7F");
  InitColorValue("Background/Menu/ItemRightBorder", "#3C3C3C", "#CCEBFF");
  InitColorValue("Background/Menu/Border", "#A0A0A0", "#A0A0A0");
  InitColorValue("Background/View/Panel", "#FCFCFC", "#202020");
  InitColorValue("Background/View/AlternateRow", "#F5F5F5", "#5C5C5C");
  InitColorValue("Background/View/SelectedInactive", "#D9D9D9", "#333333");
  InitColorValue("Background/View/SelectedActive", "#CCE8FF", "#777777");
  InitColorValue("Background/View/Hover", "#E5F3FF", "#4D4D4D");
  InitColorValue("Background/View/GridLine", "#D8D8D8", "#3C3C3C");
  InitColorValue("Background/View/TableCornerButton/Section", "#F5F6F7", "#323232");
  InitColorValue("Background/HeaderView/Pressed", "#BCDCF4", "#838383");
  InitColorValue("Background/HeaderView/Hover", "#D9EBF9", "#434343");
  InitColorValue("Background/HeaderView/NotHover", "#FFFFFF", "#202020");
  InitColorValue("Background/ScrollBar/EmptyZone", "#F0F0F0", "#171717");
  InitColorValue("Background/ScrollBar/Handle", "#CDCDCD", "#4D4D4D");
  InitColorValue("Background/ScrollBar/Checked", "#A6A6A6", "#7A7A7A");
  InitColorValue("Background/AbstractButton/ToolButton/General", "#F5F6F7", "#323232");
  InitColorValue("Background/AbstractButton/ToolButton/Hovered", "#E5F1FB", "#4A4A4A");
  InitColorValue("Background/AbstractButton/ToolButton/Pressed", "#CCE4F7", "#5A5A5A");
  InitColorValue("Background/AbstractButton/ToolButton/Checked", "#CCE8FF", "#75673D"); // dark also can be 666666
  InitColorValue("Background/AbstractButton/PushButton/General", "#E1E1E1", "#3F3F3F");
  InitColorValue("Background/AbstractButton/PushButton/Hovered", "#E5F1FB", "#4A4A4A");
  InitColorValue("Background/AbstractButton/PushButton/Pressed", "#CCE4F7", "#5A5A5A");
  InitColorValue("Background/AbstractButton/PushButton/Checked", "#CCE8FF", "#75673D"); // dark also can be 666666
  InitColorValue("Border/General", "#3C3C3C", "#CCEBFF");
  InitColorValue("Border/ComboBox", "#D9D9D9", "#666666");
  InitColorValue("Border/TabWidget", "#A0A0A0", "#A0A0A0");
  InitColorValue("Border/HeaderView", "#E5E5E5", "#636363");
  InitColorValue("Foreground/General", "#000000", "#FFFFFF");
  InitColorValue("Foreground/Disabled", "#A0A0A0", "#707070");
  InitColorValue("Foreground/Placeholder", "#6D6D6D", "#6D6D6D");
  InitColorValue("Foreground/MenuFont", "#000000", "#FFFFFF");

  using namespace FontCfg;
  InitColorUnrelatedValue("Font/Family/General", mFontFamilyDef);
  InitColorUnrelatedValue("Font/Family/Code", mFontFamilyCodeDef);
  InitColorUnrelatedValue("Font/Size/General", mFontSizeDef);
  InitColorUnrelatedValue("Font/Size/QTabBar", mFontSizeTabDef);
  InitColorUnrelatedValue("Font/Weight/General", mFontWeightDef);
  InitColorUnrelatedValue("Font/Style/General", mFontStyleDef);
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

const QVariant& StyleSheetGetter::defValue(const QString& key, bool* bKeyExist) const {
  return defCurValue(key, bKeyExist).def;
}

const QVariant& StyleSheetGetter::curValue(const QString& key, bool* bKeyExist) const {
  return defCurValue(key, bKeyExist).cur;
}

const Style::CfgDefCur& StyleSheetGetter::defCurValue(const QString& key, bool* bKeyExist) const {
  auto it = mStyleCfg.find(key);
  const bool bNotFindKey{it == mStyleCfg.end()};
  if (bKeyExist != nullptr) {
    *bKeyExist = !bNotFindKey;
  }
  if (bNotFindKey) {
    LOG_W("Cannot find key: %s, fallback to default", qPrintable(key));
    static const Style::CfgDefCur whenNotFind;
    return whenNotFind;
  }
  return it.value();
}

int StyleSheetGetter::UpdateCurValue(const QVariantHash& cfg) const {
  int settingItemsUpdatedCnt{0};
  for (auto newIt = cfg.cbegin(); newIt != cfg.cend(); ++newIt) {
    auto itFind = mStyleCfg.find(newIt.key());
    if (itFind == mStyleCfg.end()) {
      LOG_W("key[%s] not in StyleCfg cache", qPrintable(newIt.key()));
      continue;
    }
    if (newIt.value() == itFind.value().cur) {
      continue;
    }
    itFind->cur = newIt.value();
    ++settingItemsUpdatedCnt;
  }
  return settingItemsUpdatedCnt;
}

void StyleSheetGetter::InitColorUnrelatedValue(const QString& keyCore, const QVariant& def) const {
  static const auto& cfg = Configuration();
  QString keyComplete{"StyleSheet/" + keyCore};
  mStyleCfg[keyComplete] = {def, cfg.value(keyComplete, def)};
}

void StyleSheetGetter::InitColorValue(const QString& keyCore, const QVariant& lightDef, const QVariant& darkDef) const {
  static const auto& cfg = Configuration();
  QString lightKeyComplete{"StyleSheet/LightColor/" + keyCore};
  QString darkKeyComplete{"StyleSheet/DarkColor/" + keyCore};
  mStyleCfg[lightKeyComplete] = {lightDef, cfg.value(lightKeyComplete, lightDef)};
  mStyleCfg[darkKeyComplete] = {darkDef, cfg.value(darkKeyComplete, darkDef)};
}

QString StyleSheetGetter::GetColorValue(const QString& keyCore, Style::StyleSheetE styleE) const {
  QString keyComplete{"StyleSheet/"};
  keyComplete.reserve(30);
  if (styleE == Style::StyleSheetE::STYLESHEET_DARK_THEME_MOON_FOG) {
    keyComplete += "DarkColor/";
  } else {
    keyComplete += "LightColor/";
  }
  keyComplete += keyCore;
  return curValue(keyComplete).toString();
}

void StyleSheetGetter::WriteIntoSettingsCore(const StyleSheetGetter& self) {
  QSettings& settings = Configuration();
  for (auto it = self.mStyleCfg.cbegin(); it != self.mStyleCfg.cend(); ++it) {
    settings.setValue(it.key(), it.value().cur);
  }
}

void StyleSheetGetter::WriteIntoSettings() const {
  WriteIntoSettingsCore(*this);
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
