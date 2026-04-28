#include "ComboBoxString.h"
#include "Logger.h"

ComboBoxString::ComboBoxString(GeneralDataType::Type gDataType, QWidget *parent)
  : ComboBoxGeneral{gDataType, parent} {
  mCandidates = GetCandidates(gDataType);
  addItems(*mCandidates);
}

QVariant ComboBoxString::getSetDataEditRoleValue() const {
  QString rawText = currentText();
  // 严格检查即将写入的配置
  return mCandidates->contains(rawText) ? rawText : "";
}

void ComboBoxString::updateCurrentTextFromEditRole(const QVariant &editRoleData) {
  const QString editModeDisplayText = getDisplayString(editRoleData, *mCandidates);
  ComboBoxGeneral::updateCurrentTextFromEditRole(editModeDisplayText);
}

QString ComboBoxString::displayTextFromDisplayRole(GeneralDataType::Type gDataType, const QVariant &displayRoleData) {
  const T_CANDIDATES_STR_LST *const candidatesDisplay{GetCandidates(gDataType)};
  const QString displayModeDisplayText = getDisplayString(displayRoleData, *candidatesDisplay);
  return displayModeDisplayText;
}

const ComboBoxString::T_CANDIDATES_STR_LST *ComboBoxString::GetCandidates(GeneralDataType::Type gDataType) {
  static const T_CANDIDATES_STR_LST CANDIDATES_DEFAULT;
  static const QMap<int, T_CANDIDATES_STR_LST> CANDIDATES_MAP{
      {GeneralDataType::Type::FONT_FAMILY,
       {
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
       }},
  };
  auto it = CANDIDATES_MAP.find(gDataType);
  if (it == CANDIDATES_MAP.cend()) {
    return &CANDIDATES_DEFAULT;
  } else {
    return &it.value();
  }
}

QString ComboBoxString::getDisplayString(const QVariant &variantData, const T_CANDIDATES_STR_LST &pCandidateDisp) {
  QString curStrFromModel = variantData.toString();
  if (!pCandidateDisp.contains(curStrFromModel)) {
    return QString{"not allowed: data[%1] from model"}.arg(curStrFromModel);
  }
  return curStrFromModel;
}
