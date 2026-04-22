#include "StyleSheetGetter.h"
#include "PublicMacro.h"
#include "MemoryKey.h"
#include <unordered_set>

namespace FontCfg {
bool isCoarseEqual(const QFont& lhs, const QFont& rhs) {
  const bool bEq{lhs.family() == rhs.family() && lhs.pointSize() == rhs.pointSize() && lhs.weight() == rhs.weight() && lhs.style() == rhs.style()};
  if (!bEq) {
    LOG_D("family: %s, %s; pointSize: %d, %d; weight: %d, %d; style: %d, %d;",  //
          qPrintable(lhs.family()),
          qPrintable(rhs.family()),  //
          lhs.pointSize(),
          rhs.pointSize(),  //
          lhs.weight(),
          rhs.weight(),  //
          lhs.style(),
          rhs.style());  //
  }
  return bEq;
}

QString Font2String(const QFont& font) {
  return QString{R"(font-family: "%1"; font-size: %2px; font-weight: %3; font-style: %4;)"}  //
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
  static const std::unordered_set<int> weightsSet{
      //
      QFont::Weight::Thin, QFont::Weight::ExtraLight, QFont::Weight::Light, QFont::Weight::Normal, QFont::Weight::Medium, QFont::Weight::DemiBold, QFont::Weight::Bold, QFont::Weight::ExtraBold, QFont::Weight::Black};
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

}  // namespace FontCfg

constexpr const char StyleSheetGetter::ROOT_NODE_NAME[];
constexpr int StyleSheetGetter::ROOT_NODE_NAME_ARR_LEN;

const StyleSheetGetter& StyleSheetGetter::GetInst() {
  static StyleSheetGetter inst;
  return inst;
}

StyleSheetGetter::StyleSheetGetter() {
  init();
}

void StyleSheetGetter::init() const {
  auto dict = GetPairDict();
  mStyleCfg.swap(dict);
}

QHash<QString, StyleItemData> StyleSheetGetter::GetPairDict() {
  QHash<QString, StyleItemData> dict;

  auto InsertPath2DataPair = [&dict](const QString& prePath, const QVariant& defVal, StyleItemData::DataTypeE dataType) -> void {
    const int nameStart = prePath.lastIndexOf('/');
    const QString name{nameStart == -1 ? prePath : prePath.mid(nameStart + 1)};

    QString fullPath{StyleSheetGetter::ROOT_NODE_NAME};
    fullPath += '/';
    fullPath += prePath;

    static const QSettings& cfg = Configuration();
    dict[fullPath] = StyleItemData{name, defVal, cfg.value(fullPath, defVal), dataType};
  };

  auto InsertPath2ColorPair = [InsertPath2DataPair](const QString& prePath, const QString& lightDef, const QString darkDef) -> void {
    InsertPath2DataPair("LightColor/" + prePath, lightDef, StyleItemData::DataTypeE::COLOR);
    InsertPath2DataPair("DarkColor/" + prePath, darkDef, StyleItemData::DataTypeE::COLOR);
  };

  InsertPath2ColorPair("Background/General", "#FFFFFF", "#000000");
  InsertPath2ColorPair("Background/Disabled", "#F0F0F0", "#2D2D2D");
  InsertPath2ColorPair("Background/Label", "#F5F6F7", "#202020");
  InsertPath2ColorPair("Background/LineEdit", "#FFFFFF", "#252525");
  InsertPath2ColorPair("Background/ComboBox", "#E1E1E1", "#252525");
  InsertPath2ColorPair("Background/ToolBar", "#F5F6F7", "#323232");
  InsertPath2ColorPair("Background/StatusBar", "#FFFFFF", "#333333");
  InsertPath2ColorPair("Background/TabBar/tabNotSelected", "#FFFFFF", "#000000");
  InsertPath2ColorPair("Background/TabBar/tabSelected", "#CCE8FF", "#75673D");
  InsertPath2ColorPair("Background/TabBar/tabHover", "#FDFDFF", "#323232");
  InsertPath2ColorPair("Background/Menu/Item", "#EEEEEE", "#2B2B2B");
  InsertPath2ColorPair("Background/Menu/ItemChecked", "#C9E8F7", "#5E5E5E");
  InsertPath2ColorPair("Background/Menu/ItemHovered", "#E8EFF7", "#474747");
  InsertPath2ColorPair("Background/Menu/Separator", "#919191", "#7F7F7F");
  InsertPath2ColorPair("Background/Menu/ItemRightBorder", "#3C3C3C", "#CCEBFF");
  InsertPath2ColorPair("Background/Menu/Border", "#A0A0A0", "#A0A0A0");
  InsertPath2ColorPair("Background/View/Panel", "#FCFCFC", "#202020");
  InsertPath2ColorPair("Background/View/AlternateRow", "#F5F5F5", "#5C5C5C");
  InsertPath2ColorPair("Background/View/SelectedInactive", "#D9D9D9", "#333333");
  InsertPath2ColorPair("Background/View/SelectedActive", "#CCE8FF", "#777777");
  InsertPath2ColorPair("Background/View/Hover", "#E5F3FF", "#4D4D4D");
  InsertPath2ColorPair("Background/View/GridLine", "#D8D8D8", "#3C3C3C");
  InsertPath2ColorPair("Background/View/TableCornerButton/Section", "#F5F6F7", "#323232");
  InsertPath2ColorPair("Background/HeaderView/Pressed", "#BCDCF4", "#838383");
  InsertPath2ColorPair("Background/HeaderView/Hover", "#D9EBF9", "#434343");
  InsertPath2ColorPair("Background/HeaderView/NotHover", "#FFFFFF", "#202020");
  InsertPath2ColorPair("Background/ScrollBar/EmptyZone", "#F0F0F0", "#171717");
  InsertPath2ColorPair("Background/ScrollBar/Handle", "#CDCDCD", "#4D4D4D");
  InsertPath2ColorPair("Background/ScrollBar/Checked", "#A6A6A6", "#7A7A7A");
  InsertPath2ColorPair("Background/AbstractButton/ToolButton/General", "#F5F6F7", "#323232");
  InsertPath2ColorPair("Background/AbstractButton/ToolButton/Hovered", "#E5F1FB", "#4A4A4A");
  InsertPath2ColorPair("Background/AbstractButton/ToolButton/Pressed", "#CCE4F7", "#5A5A5A");
  InsertPath2ColorPair("Background/AbstractButton/ToolButton/Checked", "#CCE8FF", "#75673D");  // dark also can be 666666
  InsertPath2ColorPair("Background/AbstractButton/PushButton/General", "#E1E1E1", "#3F3F3F");
  InsertPath2ColorPair("Background/AbstractButton/PushButton/Hovered", "#E5F1FB", "#4A4A4A");
  InsertPath2ColorPair("Background/AbstractButton/PushButton/Pressed", "#CCE4F7", "#5A5A5A");
  InsertPath2ColorPair("Background/AbstractButton/PushButton/Checked", "#CCE8FF", "#75673D");  // dark also can be 666666
  InsertPath2ColorPair("Border/General", "#3C3C3C", "#CCEBFF");
  InsertPath2ColorPair("Border/ComboBox", "#D9D9D9", "#666666");
  InsertPath2ColorPair("Border/TabWidget", "#A0A0A0", "#A0A0A0");
  InsertPath2ColorPair("Border/HeaderView", "#E5E5E5", "#636363");
  InsertPath2ColorPair("Foreground/General", "#000000", "#FFFFFF");
  InsertPath2ColorPair("Foreground/Disabled", "#A0A0A0", "#707070");
  InsertPath2ColorPair("Foreground/Placeholder", "#6D6D6D", "#6D6D6D");
  InsertPath2ColorPair("Foreground/MenuFont", "#000000", "#FFFFFF");

  using namespace FontCfg;
  InsertPath2DataPair("Font/Family/General", mFontFamilyDef, StyleItemData::DataTypeE::FONT_FAMILY);
  InsertPath2DataPair("Font/Family/Code", mFontFamilyCodeDef, StyleItemData::DataTypeE::FONT_FAMILY);
  InsertPath2DataPair("Font/Size/General", mFontSizeDef, StyleItemData::DataTypeE::NUMBER);
  InsertPath2DataPair("Font/Size/QTabBar", mFontSizeTabDef, StyleItemData::DataTypeE::NUMBER);
  InsertPath2DataPair("Font/Weight/General", mFontWeightDef, StyleItemData::DataTypeE::FONT_WEIGHT);
  InsertPath2DataPair("Font/Style/General", mFontStyleDef, StyleItemData::DataTypeE::FONT_STYLE);
  InsertPath2DataPair("BackgroundImage/TableView", ":/styles/BACKGROUND_IMAGE_TABLEVIEW", StyleItemData::DataTypeE::FILE_PATH);

  return dict;
}

std::unique_ptr<StyleTreeNode> StyleSheetGetter::GetModelData() const {
  // lstrip left ROOT_NODE_NAME
  // rstrip node name
  QList<std::pair<QString, StyleItemData>> lst;
  for (auto it = mStyleCfg.cbegin(); it != mStyleCfg.cend(); ++it) {
    QString fullPath = it.key();
    if (!fullPath.startsWith(ROOT_NODE_NAME)) {
      LOG_D("fullPath[%s] invalid", qPrintable(fullPath));
      continue;
    }

    static constexpr int corePathBegin = ROOT_NODE_NAME_ARR_LEN;
    int lastSlashInd = fullPath.lastIndexOf('/');
    if (lastSlashInd == -1 || lastSlashInd <= corePathBegin) {
      lastSlashInd = fullPath.size();
    }
    QString corePath = fullPath.mid(corePathBegin, lastSlashInd - corePathBegin);
    lst.push_back({corePath, it.value()});
  }
  return StyleTreeNode::fromPairList(lst, ROOT_NODE_NAME);
}

QString StyleSheetGetter::operator()(Style::StyleSheetE styleE) const {
  QString styleSheets;
  styleSheets.reserve(2000);
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
  return defCurValue(key, bKeyExist).defValue;
}

const QVariant& StyleSheetGetter::curValue(const QString& key, bool* bKeyExist) const {
  return defCurValue(key, bKeyExist).curValue;
}

const StyleItemData& StyleSheetGetter::defCurValue(const QString& key, bool* bKeyExist) const {
  auto it = mStyleCfg.find(key);
  const bool bNotFindKey{it == mStyleCfg.end()};
  if (bKeyExist != nullptr) {
    *bKeyExist = !bNotFindKey;
  }
  if (bNotFindKey) {
    LOG_W("Cannot find key: %s, fallback to default", qPrintable(key));
    static const StyleItemData whenNotFind;
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
    if (newIt.value() == itFind.value().curValue) {
      continue;
    }
    itFind->curValue = newIt.value();
    ++settingItemsUpdatedCnt;
  }
  return settingItemsUpdatedCnt;
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

QString StyleSheetGetter::GetNoColorValue(const QString& keyCore) const {
  QString keyComplete{"StyleSheet/"};
  keyComplete.reserve(30);
  keyComplete += keyCore;
  return curValue(keyComplete).toString();
}

void StyleSheetGetter::WriteIntoSettingsCore(const StyleSheetGetter& self) {
  QSettings& settings = Configuration();
  for (auto it = self.mStyleCfg.cbegin(); it != self.mStyleCfg.cend(); ++it) {
    settings.setValue(it.key(), it.value().curValue);
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
