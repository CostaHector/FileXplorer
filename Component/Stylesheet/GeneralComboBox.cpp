#include "GeneralComboBox.h"
#include "StyleEnum.h"
#include "Logger.h"
#include <QMap>

GeneralComboBox::GeneralComboBox(GeneralDataType::Type gDataType, QWidget *parent)
  : QComboBox{parent}
  , mDataType{gDataType} {
  setEditable(false);
}

GeneralComboBox *GeneralComboBox::create(int generalDataType, QWidget *parent) {
  GeneralDataType::Type typeE = static_cast<GeneralDataType::Type>(generalDataType);
  if (GeneralDataType::isComboBoxNeededInStringEditor(typeE)) {
    return new StringComboBox(typeE, parent);
  } else if (GeneralDataType::isComboBoxNeededInStringToEnumEditor(typeE)) {
    return new EnumComboBox(typeE, parent);
  } else {
    LOG_E("type[%d] cannot edit in QComboBox", typeE);
  }
  return nullptr;
}

EnumComboBox::PAIR_TYPE EnumComboBox::GetCandidates(GeneralDataType::Type gDataType) {
  static const T_CANDIDATES_STR_TO_ENUM CANDIDATES_DEFAULT;
  static const QMap<int, T_CANDIDATES_STR_TO_ENUM> CANDIDATES_MAP{
      {GeneralDataType::Type::FONT_WEIGHT, //
       {
           {"Thin", QFont::Weight::Thin},             //
           {"ExtraLight", QFont::Weight::ExtraLight}, //
           {"Light", QFont::Weight::Light},           //
           {"Normal", QFont::Weight::Normal},         //
           {"Medium", QFont::Weight::Medium},         //
           {"DemiBold", QFont::Weight::DemiBold},     //
           {"Bold", QFont::Weight::Bold},             //
           {"ExtraBold", QFont::Weight::ExtraBold},   //
           {"Black", QFont::Weight::Black},           //
       }},
      {GeneralDataType::Type::FONT_STYLE, //
       {
           {"StyleNormal", QFont::Style::StyleNormal},   //
           {"StyleItalic", QFont::Style::StyleItalic},   //
           {"StyleOblique", QFont::Style::StyleOblique}, //
       }},
      {GeneralDataType::Type::RANGE_INT_STYLE_PRESET, //
       {
           {"WindowsVista", (int) Style::StylePresetE::PRESET_WINDOWS_VISTA}, //
           {"Windows", (int) Style::StylePresetE::PRESET_WINDOWS},            //
           {"Fusion", (int) Style::StylePresetE::PRESET_FUSION},              //
           {"MacOS", (int) Style::StylePresetE::PRESET_MACOS},                //
       }},
      {GeneralDataType::Type::RANGE_INT_STYLE_THEME, //
       {
           {"Light", (int) Style::StyleThemeE::THEME_LIGHT},        //
           {"Dark", (int) Style::StyleThemeE::THEME_DARK_MOON_FOG}, //
           {"None", (int) Style::StyleThemeE::THEME_NONE},          //
       }},
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

EnumComboBox::EnumComboBox(GeneralDataType::Type gDataType, QWidget *parent)
  : GeneralComboBox{gDataType, parent} {
  std::tie(mCandidates, mCandidatesDisplay) = GetCandidates(gDataType);
  addItems(mCandidates->keys());
}

QVariant EnumComboBox::getSetDataVariant() const {
  QString rawText = currentText();
  auto it = mCandidates->find(rawText);
  if (it == mCandidates->cend()) {
    LOG_W("No value correspond to text[%s], type[%d]", qPrintable(rawText), type());
    return {};
  }
  return it.value();
}

void EnumComboBox::updateCurrentDisplayString(const QVariant &editRoleData) {
  bool bInt{false};
  int curEnumInt = editRoleData.toInt(&bInt);
  if (!bInt) {
    setCurrentText(QString("not number"));
    return;
  }
  auto it = mCandidatesDisplay->find(curEnumInt);
  if (it == mCandidatesDisplay->cend()) {
    setCurrentText(QString("no string correspond to Enum[%1]").arg(curEnumInt));
    return;
  }
  setCurrentText(it.value());
}

const QStringList StringComboBox::CANDIDATES_FONT_FAMILY{
#ifdef _WIN32
    // Windows平台字体
    "Microsoft YaHei UI", // 微软雅黑
    "SimSun",             // 宋体
    "NSimSun",            // 新宋体
    "Microsoft JhengHei", // 微软正黑体
    "Arial",              // 英文无衬线字体
    "Times New Roman",    // 英文衬线字体
    "Tahoma",             // Windows系统UI字体
    "Segoe UI",           // Windows现代UI字体
    "Calibri",            // Office默认字体
    "Consolas",           // 等宽字体
    "Courier New",        // 等宽字体
    "Verdana",            // 屏幕显示优化字体
    "Georgia",            // 适合屏幕阅读的衬线字体
    "Trebuchet MS",       // Web安全字体
    "Comic Sans MS"       // 手写风格字体
#else
    // Linux平台字体
    "Noto Sans",        // Google跨平台字体
    "Noto Sans CJK SC", // Noto Sans中文字体
    "DejaVu Sans",      // 开源无衬线字体
    "DejaVu Serif",     // 开源衬线字体
    "Liberation Sans",  // 替换Arial的开源字体
    "Liberation Serif", // 替换Times New Roman的开源字体
    "Ubuntu",           // Ubuntu系统默认字体
    "FreeSans",         // 开源无衬线字体
    "Droid Sans",       // Android系统字体
    "Arial",            // 英文无衬线字体
    "Times New Roman",  // 英文衬线字体
    "Tahoma",           // 屏幕显示字体
    "Verdana",          // 屏幕显示优化字体
    "Courier New",      // 等宽字体
    "Monospace"         // 通用等宽字体
#endif
};

StringComboBox::StringComboBox(GeneralDataType::Type gDataType, QWidget *parent)
  : GeneralComboBox{gDataType, parent} {
  if (gDataType == GeneralDataType::Type::FONT_FAMILY) {
    mCandidates = &CANDIDATES_FONT_FAMILY;
  }
  addItems(*mCandidates);
}

QVariant StringComboBox::getSetDataVariant() const {
  QString rawText = currentText();
  return mCandidates->contains(rawText) ? rawText : "";
}
